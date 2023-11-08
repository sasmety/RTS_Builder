// Fill out your copyright notice in the Description page of Project Settings.


#include "RoadIntersection.h"

#include "MeshDescriptionBuilder.h"
#include "AI/NavigationSystemHelpers.h"
#include "CompGeom/PolygonTriangulation.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Public/Road.h"
#include "VT/RuntimeVirtualTexture.h"


// Sets default values
ARoadIntersection::ARoadIntersection()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	SetRootComponent(RootMesh);
	
	FSoftObjectPath PillarPath(TEXT("/Game/Assets/Buildings/bestroad.bestroad"));
	Mesh = Cast<UStaticMesh>(PillarPath.TryLoad());
	
	FSoftObjectPath GhostMat(TEXT("/Game/Assets/Buildings/denememat.denememat"));
	GhostMaterial = Cast<UMaterialInterface>(GhostMat.TryLoad());
	FSoftObjectPath RVMat(TEXT("/Game/Assets/Materials/NewRuntimeVirtualTexture.NewRuntimeVirtualTexture"));
	RVTexture = Cast<URuntimeVirtualTexture>(RVMat.TryLoad());
	FSoftObjectPath NonLine(TEXT("/Game/Megascans/Surfaces/Fine_Asphalt_vlzobiady/denemeaf.denemeaf"));
	NonLineMaterial = Cast<UMaterialInterface>(NonLine.TryLoad());

	FBoxSphereBounds Box = Mesh->GetBounds();
	MeshLength = Box.BoxExtent.X * 2;
	for (int i = 0; i < 20; ++i)
	{
		USplineComponent* Spline = CreateDefaultSubobject<USplineComponent>(*FString::FromInt(i));
		Spline->SetupAttachment(GetRootComponent());
		Spline->ClearSplinePoints();
		Splines.Add(Spline);
	}
}

void ARoadIntersection::Init(TArray<ARoad*> road, TArray<FIntersectionPoint> point, FVector& AttachLoc, float TanLength)
{
	this->TangentLength = TanLength;
	this->AttachPoint = AttachLoc;
	int32 index = 0;
	for (ARoad* Road : road)
	{
		if (!Roads.Contains(Road))
		{
 			Roads.AddUnique(Road);
			Connections.Add(Road, point[index]);
			++index;
		}
	}

	int32 i = 0;
	FMeshDescription* desc = Mesh->GetMeshDescription(0); 
	// int firstEdgeId = desc->Edges().GetFirstValidID();
	// UE_LOG(LogTemp, Warning, TEXT("First Edge ElementID: %d, Num Of Edges: %d | Num Of Triangles :%d"),  firstEdgeId, desc->Edges().Num() , desc->Triangles().Num());
	// for (int ElementID : desc->Edges().GetElementIDs())
	// {
		// UE_LOG(LogTemp, Warning, TEXT("i:%d, Edge ElementID: %d"), i, ElementID);
		// ++i;
	// }
	for (int id : desc->Triangles().GetElementIDs())
	{
		int32 id1 = id;
		int32 id2 = desc->GetTriangleVertexInstance(id1, 1);
		int32 id3 = desc->GetVertexInstanceVertex(id2);
		// FVector loc = (FVector)desc->GetVertexPositions()[id] + FVector::UpVector * 100;
		UE_LOG(LogTemp, Warning, TEXT("id1:%d, id2:%d, id3:%d"), id1, id2, id3);
		// DrawDebugString(GetWorld(), loc + FVector::UpVector * 50, FString::Printf(TEXT("Id : %d"), id1), 0, FColor::White, 30);
		// DrawDebugSphere(GetWorld(), loc, 5, 5, FColor::Green, false, 30);
	}


	for (int id : desc->Vertices().GetElementIDs())
	{
		int32 id1 = id;
		int32 id2 = desc->GetVertexInstanceVertex(id1);
		FVector loc = (FVector)desc->GetVertexPosition(id1);
		UE_LOG(LogTemp, Warning, TEXT("id1:%d, id2:%d"), id1, id2);
		DrawDebugString(GetWorld(), loc + FVector::UpVector * 50, FString::Printf(TEXT("Id : %d"), id1), 0, FColor::White, 30);
		DrawDebugSphere(GetWorld(), loc, 5, 5, FColor::Green, false, 30);

	} 


	
	CalculateSplines();
}

void ARoadIntersection::UpdateRoadData(ARoad* Road, FIntersectionPoint Data)
{
	if (Connections.Contains(Road))
	{
		Connections.Emplace(Road, Data);
	}
}

void ARoadIntersection::SortRoadsByAnglesAccordingToCenter()
{
	TArray<float> angles;

	for (ARoad* Road : Roads)
	{
		FIntersectionPoint PointData = Connections.FindRef(Road);
		FVector CenterVector(AttachPoint - PointData.Loc);
		float angle = FMath::RadiansToDegrees(atan2(CenterVector.Y, CenterVector.X));
		if (angle < 0)
		{
			angle += 360;
		}
		angles.Add(angle);
	}

	for (int f = 0; f < angles.Num() - 1; ++f)
	{
		for (int h = f + 1; h < angles.Num(); ++h)
		{
			if (angles[h] < angles[f])
			{
				float temp = angles[f];
				angles[f] = angles[h];
				angles[h] = temp;

				ARoad* TempRoad = Roads[f];
				Roads[f] = Roads[h];
				Roads[h] = TempRoad;
			}
		}
	}
}

TArray<FVector> ARoadIntersection::GetLocs(USplineMeshComponent* SplineMeshComponent)
{
	int l_NumVertices;
	TArray<FVector> l_Positions;

	UStaticMesh* l_StaticMesh = SplineMeshComponent->GetStaticMesh();
	const FStaticMeshLODResources& l_RenderData = l_StaticMesh->GetRenderData()->LODResources[0];
	l_NumVertices = l_RenderData.GetNumVertices();

	for (int32 Index = 0; Index < l_NumVertices; Index++)
	{
		FVector location(SplineMeshComponent->GetComponentLocation() + FVector(l_RenderData.VertexBuffers.PositionVertexBuffer.VertexPosition(Index).X,
			l_RenderData.VertexBuffers.PositionVertexBuffer.VertexPosition(Index).Y, l_RenderData.VertexBuffers.PositionVertexBuffer.VertexPosition(Index).Z));
		l_Positions.Add(UKismetMathLibrary::TransformLocation(GetTransform(), location));
	}

	return l_Positions;
}

TArray<FVector> ARoadIntersection::	DrawBorderDebugLocationsOfSpline(USplineComponent* SplineComponent, int32 Direction, USplineMeshComponent* InSplineMeshComponent, float Start, float End)
{
	TArray<FVector> Points;
	if (SplineComponent->GetNumberOfSplinePoints() < 2)
	{
		return TArray<FVector>();
	}

	if (SplineComponent->GetSplineLength() < 30)
	{
		return TArray<FVector>();
	}

	float segmentlen = End - Start;

	
	int32 CheckPointsNum = FMath::CeilToInt32(segmentlen / 50);
	float Distance = segmentlen / CheckPointsNum;

	FVector LastTangent(0,0,0);
	FVector LastPlaced(0,0,0);
	// for (int i = 0; i < CheckPointsNum + 1; ++i)
	// {
		float StartValue = Start ;
		FVector StartLoc = SplineComponent->GetLocationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::World);
		FVector StartLocLocal = SplineComponent->GetLocationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local);
		FVector DebugLocation = SplineComponent->GetLocationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local)
		+ SplineComponent->GetRightVectorAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local) * Mesh->GetBounds().BoxExtent.Y * Direction;
		
		const double& AxisValue = USplineMeshComponent::GetAxisValueRef(StartLocLocal, InSplineMeshComponent->ForwardAxis);
		FTransform SliceTransform = InSplineMeshComponent->CalcSliceTransform(AxisValue);
		FVector a = UKismetMathLibrary::TransformLocation(GetTransform(), (FVector)SliceTransform.GetLocation() + StartLocLocal);

		DebugEdge(a, a);


	return Points;
}

void ARoadIntersection::DebugEdge(const FVector& Start, const FVector& End)
{
	FColor Color = FColor::MakeRandomColor();
	// DrawDebugSphere(GetWorld(), Start, 5, 5, Color, false, 10, 4);
	// DrawDebugLine(GetWorld(), Start, End, Color, false, 10, 4);
	DrawDebugSphere(GetWorld(), End, 5, 5, Color, false, 10, 4);
}

void ARoadIntersection::CalculateSplines()
{
	if (Roads.Num() <= 1)
		Destroy();

	SortRoadsByAnglesAccordingToCenter();
	TArray<FVector> EdgePoints;
	int32 index = 0;
	FVector off(0, 0, 0);
	for (TTuple<ARoad*, FIntersectionPoint> Connection : Connections)
	{
		Connection.Value.Loc.Z = 100;
	}
		

	int32 l = 0;
	for (int i = 0; i < Roads.Num() - 1; ++i)
	{
		TObjectPtr<ARoad> FirstRoad = Roads[i];
		FIntersectionPoint FirstPoint = Connections.FindRef(FirstRoad);
		for (int j = i + 1; j < Roads.Num(); ++j)
		{
			TObjectPtr<ARoad> EndRoad = Roads[j];
			FIntersectionPoint EndPoint = Connections.FindRef(EndRoad);
			TObjectPtr<USplineComponent> Spline = Splines[index];
			++index;
			Spline->AddSplineWorldPoint(FirstPoint.Loc + (off * index));
			Spline->SetSplinePointType(0, ESplinePointType::CurveCustomTangent, false);
			FVector Dir1, Dir2;
			Dir1 = (AttachPoint - FirstPoint.Loc).GetSafeNormal() * FVector::Dist(FirstPoint.Loc, AttachPoint) * 2;
			Dir2 = (EndPoint.Loc - AttachPoint).GetSafeNormal() * FVector::Dist(AttachPoint, EndPoint.Loc) * 2;

			Spline->SetTangentAtSplinePoint(0, Dir1, ESplineCoordinateSpace::World, false);
			Spline->AddSplineWorldPoint(EndPoint.Loc + (off * index));
			Spline->SetSplinePointType(1, ESplinePointType::CurveCustomTangent, false);
			Spline->SetTangentAtSplinePoint(1, Dir2, ESplineCoordinateSpace::World, false);
			Spline->UpdateSpline();

			if (i + 1 == j)
			{
				UMaterialInstanceDynamic* GhostMaterialInstance = UMaterialInstanceDynamic::Create(GhostMaterial, this);
				GhostMaterialInstance->SetScalarParameterValue(FName(TEXT("TextureIndex")), 1);
				GhostMaterialInstance->SetScalarParameterValue(FName(TEXT("RotationAngle")), 0);
				EdgePoints.Append(GenerateSplineMesh(Spline, Mesh, 1, GhostMaterialInstance, - 1));
				UE_LOG(LogTemp, Warning, TEXT("%s solunda %s"), *FirstRoad.GetName(), *EndRoad.GetName());
				// EdgePoints.Append(DrawBorderDebugLocationsOfSpline(Spline, -1));
				// EdgePoints.Append(CalculatePoints(Spline, -1, 5));
			}
			else if (i == 0 && j == Roads.Num() - 1)
			{
				UMaterialInstanceDynamic* GhostMaterialInstance = UMaterialInstanceDynamic::Create(GhostMaterial, this);
				GhostMaterialInstance->SetScalarParameterValue(FName(TEXT("TextureIndex")), 1);
				GhostMaterialInstance->SetScalarParameterValue(FName(TEXT("RotationAngle")), 180);
				EdgePoints.Append(GenerateSplineMesh(Spline, Mesh, 1, GhostMaterialInstance, 1));
				UE_LOG(LogTemp, Warning, TEXT("%s solunda %s döngüde"), *EndRoad.GetName(), *FirstRoad.GetName());
				// EdgePoints.Append(CalculatePoints(Spline, 1, 5));
				// EdgePoints.Append(DrawBorderDebugLocationsOfSpline(Spline, 1));
			}
			// else
			// {
			// 	UMaterialInstanceDynamic* GhostMaterialInstance = UMaterialInstanceDynamic::Create(NonLineMaterial, this);
			// 	GenerateSplineMesh(Spline, Mesh, 0, GhostMaterialInstance);
			// 	continue;
			// }
			//
			// UMaterialInstanceDynamic* GhostMaterialInstance = UMaterialInstanceDynamic::Create(NonLineMaterial, this);
			// GenerateSplineMesh(Spline, Mesh, l * 0.02, GhostMaterialInstance);

			++l;

			// FirstRoad->EndIntersection = this;
			// EndRoad->StartIntersection = this;


			//Remove These Unnecessary Codes.
			// if (Roads.Num() > 2)
			// {
			// 	FVector x =  AttachPoint - FirstPoint.Loc;
			// 	FVector a = FirstPoint.Loc - EndPoint.Loc;
			// 	float k = atan2(x.Y, x.X);
			// 	float l = atan2(a.Y, a.X);
			//
			// 	// if (k < 0)
			// 	// {
			// 	// 	k += 360;
			// 	// }
			// 	//
			// 	// if (l < 0)
			// 	// {
			// 	// 	l += 360;
			// 	// }
			// 	
			// 	float value = l - k;
			// 	value = FMath::RadiansToDegrees(value);
			// 	if (value < 0)
			// 	{
			// 		value += 360;
			// 	}
			// 	rs.Add(Spline);
			// 	fs.Add(value);
			// 	xs.Add(EndRoad);
			// }
		}
		// float temp = 359;
		// float ct1 = 0;
		// for (int k = 0; k < fs.Num(); ++k)
		// {
		// 	if (fs[k] < temp && fs[k] > 0)
		// 	{
		// 		temp = fs[k];
		// 		ct1 = k;
		// 	}
		// }
		//
		// temp = 0;
		// float ct2 = 0;
		// for (int k = 0; k < fs.Num(); ++k)
		// {
		// 	if (fs[k] > temp && fs[k] > 0)
		// 	{
		// 		temp = fs[k];
		// 		ct2 = k;
		// 	}
		// }
		// USplineComponent* mostRight = rs[ct2];
		// USplineComponent* mostLeft = rs[ct1];
		//

		// UMaterialInstanceDynamic* GhostMaterialInstance2 = UMaterialInstanceDynamic::Create(GhostMaterial, this);
		// GhostMaterialInstance2->SetScalarParameterValue(FName(TEXT("TextureIndex")), 1);
		// GhostMaterialInstance2->SetScalarParameterValue(FName(TEXT("RotationAngle")), 0);
		// GenerateSplineMesh(mostRight, Mesh, 10, GhostMaterialInstance2);
		//
		// UE_LOG(LogTemp, Warning, TEXT("For %s, Most Left is :%s[%f], Most Right is :%s[%f]"), *Roads[i]->GetName(), *xs[ct1]->GetName(), fs[ct1], *xs[ct2]->GetName(), fs[ct2]);
		// for (int t = 0; t < xs.Num(); ++t)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("  - %s : %f"), *xs[t]->GetName(), fs[t]);
		// }
	}
	// RootMesh->SetStaticMesh(MakeMesh(EdgePoints));
	// RootMesh->SetMaterial(0, NonLineMaterial);
}

UStaticMesh* ARoadIntersection::MakeMesh(TArray<FVector> loc)
{
	UStaticMesh* staticMesh = NewObject<UStaticMesh>(this);
	staticMesh->GetStaticMaterials().Add(FStaticMaterial());

	


	TArray<float> angles;

	for (FVector Vec : loc)
	{
		FVector CenterVector(UKismetMathLibrary::InverseTransformLocation(GetTransform(), AttachPoint) - Vec);
		float angle = FMath::RadiansToDegrees(atan2(CenterVector.Y, CenterVector.X));
		if (angle < 0)
		{
			angle += 360;
		}
		angles.Add(angle);
	}

	for (int f = 0; f < angles.Num() - 1; ++f)
	{
		for (int h = f + 1; h < angles.Num(); ++h)
		{
			if (angles[h] < angles[f])
			{
				float temp = angles[f];
				angles[f] = angles[h];
				angles[h] = temp;

				FVector TempRoad = loc[f];
				loc[f] = loc[h];
				loc[h] = TempRoad;
			}
		}
	}

	Algo::Reverse(loc);

	TArray<FVector2f> zz;
	for (FVector Location : loc) { zz.Add(FVector2f(Location.X, Location.Y)); }
	TArray<UE::Geometry::FIndex3i> triangles;
	
	PolygonTriangulation::TriangulateSimplePolygon(zz, triangles, false);
	FMeshDescription meshDesc;

	FStaticMeshAttributes Attributes(meshDesc);
	Attributes.Register();

	FMeshDescriptionBuilder meshDescriptionBuilder;
	meshDescriptionBuilder.SetMeshDescription(&meshDesc);
	meshDescriptionBuilder.EnablePolyGroups();
	meshDescriptionBuilder.SetNumUVLayers(1);

	TArray<FVertexID> VertexIDs;

	
	TArray<FVector> locations;
	// locations.Add(UKismetMathLibrary::InverseTransformLocation(GetTransform(), AttachPoint));
	for (FVector Loc : loc)
	{
		locations.Add(Loc);
	} 
	int index = 0;
	// locations.Insert(UKismetMathLibrary::InverseTransformLocation(GetTransform(), AttachPoint), 0);
	
	for (FVector Vector : locations)
	{
		FSplinePoint point(index, Vector);
		VertexIDs.Add(meshDescriptionBuilder.AppendVertex(Vector));

		index++;
	}

	FPolygonGroupID polygonGroup = meshDescriptionBuilder.AppendPolygonGroup();
	struct VertexInfo
	{
		int ID;
		FVector InstanceNormal;
		FVector2D InstanceUV;

		VertexInfo(int InID, FVector InInstanceNormal, FVector2D InInstanceUV)
			: ID(InID), InstanceNormal(InInstanceNormal), InstanceUV(InInstanceUV)
		{
		}
	};

	auto AppendTriangle = [&meshDescriptionBuilder, &VertexIDs, polygonGroup](TArray<VertexInfo> vertex)
	{
		TArray<FVertexInstanceID> vertexInsts;
		for (int i = 0; i < 3; ++i)
		{
			FVertexInstanceID instance = meshDescriptionBuilder.AppendInstance(VertexIDs[vertex[i].ID]);
			meshDescriptionBuilder.SetInstanceNormal(instance, vertex[i].InstanceNormal);
			meshDescriptionBuilder.SetInstanceUV(instance, vertex[i].InstanceUV, 0);
			meshDescriptionBuilder.SetInstanceColor(instance, FVector4f(1.f, 1.f, 1.f, 1.f));
			vertexInsts.Add(instance);
		}
		meshDescriptionBuilder.AppendTriangle(vertexInsts[0], vertexInsts[1], vertexInsts[2], polygonGroup);
	};
	for (UE::Geometry::FIndex3i& Triangle : triangles)
	{
		// GEngine->AddOnScreenDebugMessage(INDEX_NONE, 60, FColor::Orange, FString::Printf(TEXT("i:%d A: %d | B: %d | C: %d "), ind, Triangle.A, Triangle.B, Triangle.C));
		AppendTriangle(
			{
				VertexInfo(Triangle.A, FVector(0, 0, 1), FVector2D(0, 1)),
				VertexInfo(Triangle.B, FVector(0, 0, 1), FVector2D(1, 0)),
				VertexInfo(Triangle.C, FVector(0, 0, 1), FVector2D(0, 0)),
			}
		);
	}

		// for (int i = 0; i < locations.Num() - 2; ++i)
		// {
		// 	AppendTriangle({
		// 		VertexInfo(0, FVector(0, 0, 1), FVector2D(0, 1)),
		// 		VertexInfo(i + 1, FVector(0, 0, 1), FVector2D(1, 0)),
		// 		VertexInfo(i + 2, FVector(0, 0, 1), FVector2D(0, 0)),
		// 	});
		// }
		//
		// AppendTriangle({
		// 	VertexInfo(0, FVector(0, 0, 1), FVector2D(0, 1)),
		// 	VertexInfo(locations.Num() - 1, FVector(0, 0, 1), FVector2D(1, 0)),
		// 	VertexInfo(1, FVector(0, 0, 1), FVector2D(0, 0)),
		// });
		//
		// FColor Color = FColor::MakeRandomColor();
		// FVector A = UKismetMathLibrary::TransformLocation(GetTransform(), locations[Triangle.A]);
		// FVector B = UKismetMathLibrary::TransformLocation(GetTransform(), locations[Triangle.B]);
		// FVector C = UKismetMathLibrary::TransformLocation(GetTransform(), locations[Triangle.C]);
		// DrawDebugPoint(GetWorld(), A, 10, Color, false, 30);
		// DrawDebugPoint(GetWorld(), B, 10, Color, false, 30);
		// DrawDebugPoint(GetWorld(), C, 10, Color, false, 30);
		// DrawDebugLine(GetWorld(), A, B, Color, false, 30);
		// DrawDebugLine(GetWorld(), B, C, Color, false, 30);
		// DrawDebugLine(GetWorld(), C, A, Color, false, 30);

	

	UStaticMesh::FBuildMeshDescriptionsParams mdParams;
	mdParams.bBuildSimpleCollision = true;
	mdParams.bFastBuild = true;

	TArray<const FMeshDescription*> meshDescPtrs;
	meshDescPtrs.Emplace(&meshDesc);

	staticMesh->BuildFromMeshDescriptions(meshDescPtrs, mdParams);

	return staticMesh;
}

void ARoadIntersection::DrawArrow(FVector& Loc, FVector& Dir, float len, FColor color)
{
	DrawDebugDirectionalArrow(GetWorld(), Loc + FVector::UpVector * 50, Loc + FVector::UpVector * 50 + (Dir * len) + FVector::UpVector * 50, 10, color, true, 0, 1, 10);
}

TArray<FVector> ARoadIntersection::GenerateSplineMesh(USplineComponent* SplineComponent, UStaticMesh* StaticMesh, float HeightOffset, UMaterialInstanceDynamic* Material, float Direction)
{
	TArray<FVector> a;
	float SegmentLength = SplineComponent->GetSplineLength() - 1;
	int32 NumberOfSegments = FMath::CeilToInt(SegmentLength / MeshLength);
	if (NumberOfSegments == 0)
		NumberOfSegments = 1;
	float StretchedMeshLength = SegmentLength / NumberOfSegments;

	if (StretchedMeshLength > MeshLength * 1.5f)
	{
		StretchedMeshLength /= 2;
		NumberOfSegments *= 2;
	}
	for (int j = 0; j < NumberOfSegments; ++j)
	{
		float StartValue = StretchedMeshLength * j;
		float EndValue = StretchedMeshLength * (j + 1);

		FVector Start = SplineComponent->GetLocationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::World);
		FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local);
		StartTang = StartTang.GetSafeNormal() * StretchedMeshLength;

		FVector End = SplineComponent->GetLocationAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::World);
		FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::Local);
		EndTang = StretchedMeshLength * EndTang.GetSafeNormal();

		USplineMeshComponent* NewSplineMeshComponent = NewObject<USplineMeshComponent>(this);
		NewSplineMeshComponent->RegisterComponent();
		NewSplineMeshComponent->SetMobility(EComponentMobility::Movable);
		NewSplineMeshComponent->SetStaticMesh(StaticMesh);
		NewSplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		NewSplineMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Block);
		NewSplineMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		if (Material == nullptr)
		{
			NewSplineMeshComponent->RuntimeVirtualTextures.Add(RVTexture);
			NewSplineMeshComponent->SetMaterial(0, GhostMaterial);
		}
		else
		{
			NewSplineMeshComponent->SetMaterial(0, Material);
		}

		NewSplineMeshComponent->CastShadow = false;


		NewSplineMeshComponent->SetStartAndEnd(Start + (FVector::UpVector * HeightOffset), StartTang, End + (FVector::UpVector * HeightOffset), EndTang);
		
		a.Append(DrawBorderDebugLocationsOfSpline(SplineComponent, Direction, NewSplineMeshComponent, StartValue, EndValue));

		// FTransform z = NewSplineMeshComponent->CalcSliceTransform(20);
		// // DrawDebugSphere(GetWorld(), z.GetLocation(), 10, 10, FColor::Orange, false, 5);
		// DrawDebugSphere(GetWorld(), z.GetLocation(), 50, 10, FColor::Orange, false, 30);
		// FVector min, max;
		// NewSplineMeshComponent->GetLocalBounds(min,max);
		// UE_LOG(LogTemp, Warning, TEXT("min:%s max:%s"), *min.ToString(), *max.ToString());
		//
		//
		// TArray<FVector> x = GetLocs(NewSplineMeshComponent);
		//
		// for (FVector Loc : x)
		// {
		// 	// DrawDebugLine(GetWorld(), Loc, Loc + (FVector::UpVector * 200), FColor::Cyan, false, 10);
		// }
	}

	return a;
}

// void ARoadIntersection::PropagateSplineDeformationToMesh(const USplineMeshComponent* InSplineMeshComponent, FVector& InOutVector)
// {
// 	// Apply spline deformation for each vertex's tangents
// 	
// 	int32 WedgeIndex = 0;
// 	for (const FTriangleID TriangleID : InOutMeshDescription.Triangles().GetElementIDs())
// 	{
// 		for (int32 Corner = 0; Corner < 3; ++Corner, ++WedgeIndex)
// 		{
// 			const FVertexInstanceID VertexInstanceID = InOutMeshDescription.GetTriangleVertexInstance(TriangleID, Corner);
// 			const FVertexID VertexID = InOutMeshDescription.GetVertexInstanceVertex(VertexInstanceID);
// 			const float& AxisValue = USplineMeshComponent::GetAxisValueRef(VertexPositions[VertexID], InSplineMeshComponent->ForwardAxis);
// 			FTransform SliceTransform = InSplineMeshComponent->CalcSliceTransform(AxisValue);
// 			FVector TangentY = FVector::CrossProduct((FVector)VertexInstanceNormals[VertexInstanceID], (FVector)VertexInstanceTangents[VertexInstanceID]).GetSafeNormal() * VertexInstanceBinormalSigns[VertexInstanceID];
// 			VertexInstanceTangents[VertexInstanceID] = (FVector3f)SliceTransform.TransformVector((FVector)VertexInstanceTangents[VertexInstanceID]);
// 			TangentY = SliceTransform.TransformVector(TangentY);
// 			VertexInstanceNormals[VertexInstanceID] = (FVector3f)SliceTransform.TransformVector((FVector)VertexInstanceNormals[VertexInstanceID]);
// 			VertexInstanceBinormalSigns[VertexInstanceID] = GetBasisDeterminantSign((FVector)VertexInstanceTangents[VertexInstanceID], TangentY, (FVector)VertexInstanceNormals[VertexInstanceID]);
// 		}
// 	}
//
// 	// Apply spline deformation for each vertex position
// 	for (const FVertexID VertexID : InOutMeshDescription.Vertices().GetElementIDs())
// 	{
// 		float& AxisValue = USplineMeshComponent::GetAxisValueRef(VertexPositions[VertexID], InSplineMeshComponent->ForwardAxis);
// 		FTransform SliceTransform = InSplineMeshComponent->CalcSliceTransform(AxisValue);
// 		AxisValue = 0.0f;
// 		VertexPositions[VertexID] = (FVector3f)SliceTransform.TransformPosition((FVector)VertexPositions[VertexID]);
// 	}
// }

TArray<FVector> ARoadIntersection::CalculatePoints(USplineComponent* SplineComponent, int32 Direction, int32 NumOfDesiredPoints)
{
	int32 x = 0;
	FVector p0, p1, t0, t1;

	p0 = SplineComponent->GetWorldLocationAtSplinePoint(0) + (SplineComponent->GetRightVectorAtSplinePoint(0, ESplineCoordinateSpace::Local) * Mesh->GetBounds().BoxExtent.Y * Direction);
	p1 = SplineComponent->GetWorldLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1) + (SplineComponent->GetRightVectorAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local) * Mesh->GetBounds().BoxExtent.Y * Direction);


	t0 = SplineComponent->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::World) / 3;
	t1 = SplineComponent->GetTangentAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World) / 3;

	

	TArray<FVector> locs;
	float interval = SplineComponent->GetSplineLength() / NumOfDesiredPoints;

	FVector l =  FMath::CubicInterp<FVector, float>(p0, t0, p1, t1, 0);
	locs.Add(l);

	


	float min,max;
	SplineComponent->SplineCurves.ReparamTable.CalcBounds(min ,max);

	int z = 0;
	for (FVector Out : locs)
	{
		UE_LOG(LogTemp, Warning, TEXT("BEZIER %d:%s  |  min: %f max: %f"), z, *Out.ToString(), min, max);
		++z;
		DrawDebugSphere(GetWorld(), Out, 5, 5, FColor::Red, false, 10, 4);
	} 

	return locs;
}

void ARoadIntersection::AddConnection(ARoad* Road, FIntersectionPoint& Point, float CutSize)
{
	if (Roads.Num() >= 6) return;
	if (Roads.Contains(Road)) return;
	float dist = Road->SplineComponent->GetDistanceAlongSplineAtSplineInputKey(Road->SplineComponent->FindInputKeyClosestToWorldLocation(Point.Loc));
	if (dist < Road->SplineComponent->GetSplineLength() / 2)
	{
		Point = Road->CutToFromStart(TangentLength);
		Road->StartIntersection = this;
	}
	else
	{
		Point = Road->CutFromToEnd(Road->SplineComponent->GetSplineLength() - (TangentLength));
		Road->EndIntersection = this;
	}
	Roads.AddUnique(Road);
	Connections.Add(Road, Point);
	ClearSplines();
	CalculateSplines();
}

void ARoadIntersection::DisconnectRoad(ARoad* Road)
{
	if (Roads.Contains(Road))
	{
		Roads.Remove(Road);
		Connections.Remove(Road);
		ClearSplines();
		CalculateSplines();
	}
}

void ARoadIntersection::ClearSplines()
{
	for (UActorComponent* Component : GetComponents())
		if (Component->IsA(USplineMeshComponent::StaticClass()))
			Component->DestroyComponent();

	for (USplineComponent* SplineComponent : Splines)
	{
		SplineComponent->ClearSplinePoints();
	}
}

void ARoadIntersection::DestroyConnections(ARoad* Road)
{
	if (!Road) return;
	Road->RemoveIntersection(this);
	Roads.Remove(Road);
	for (ARoad* ConnectionRoad : Roads)
	{
		ConnectionRoad->RemoveIntersection(this);
	}
	this->Destroy();
}

// Called when the game starts or when spawned
void ARoadIntersection::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARoadIntersection::Tick(float DeltaTime)
{
	DrawDebugString(GetWorld(), AttachPoint + FVector::UpVector * 100, FString::Printf(TEXT("Junction:%s"), *GetName()), 0, FColor::Yellow, 0);
	DrawDebugSphere(GetWorld(), AttachPoint, 25, 10, FColor::Yellow, false, 0);
}
