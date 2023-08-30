// @formatter:max_line_length 400
// @formatter:toplevel_function_definition_return_type_style on_single_line
// @formatter:simple_block_style on_single_line
// @formatter:member_initializer_list_style line_break
// @formatter:line_break_after_colon_in_member_initializer_lists on_single_line
// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuildingResizable.h"
#include "CoreMinimal.h"
#include "RTS_Builder/BuildingSystem/Public/BuildingSpline.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "RTS_Builder/PointActor.h"
#include "RTS_Builder/RTSController.h"
#include "StaticMeshAttributes.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Algo/RemoveIf.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Evaluation/Blending/MovieSceneBlendType.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MeshConversion/Public/MeshDescriptionBuilder.h"
#include "CompGeom/PolygonTriangulation.h"


#define ECC_SplineComponent ECC_GameTraceChannel1

USplineComponent* Border;

ABuildingResizable::ABuildingResizable(const FObjectInitializer& OI)
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ROOTAF"));
	SetRootComponent(RootMesh);
	InSceneBuildings = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SeveralMesh"));
	InSceneBuildings->SetupAttachment(GetRootComponent());
	InSceneBuildings->SetStaticMesh(StaticMesh);
	Border = CreateDefaultSubobject<USplineComponent>(TEXT("Border"));
	Border->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}

void ABuildingResizable::BeginPlay()
{
	Super::BeginPlay();
	RootMesh->SetMaterial(0, MaterialVirtual);
	RootMesh->RuntimeVirtualTextures.Add(RuntimeVirtualTextureMaterial);
	RootMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RootMesh->SetStaticMesh(MakeMesh(Vectors));
	DrawPointActors(Vectors);
	int32 index = 0;
	for (auto Vector : Vectors)
	{
		Border->AddSplinePointAtIndex(Vector, index, ESplineCoordinateSpace::Local, false);
		++index;
	}
	Border->UpdateSpline();
}

void ABuildingResizable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DrawDebugSphere(GetWorld(), GetActorLocation(), 30, 10, FColor::Green, false, 1);
}

void ABuildingResizable::OnConstruction(const FTransform& Transform)
{
	SizeX = 1;
	SizeY = 1;
}

void ABuildingResizable::OnStartBuilding(ARTSController* Ctrl)
{
	Controller = Ctrl;
	DrawGrid(1, 1);
}

void ABuildingResizable::Build()
{
	FActorSpawnParameters Params;
	ABuildingResizable* building = GetWorld()->SpawnActor<ABuildingResizable>(
		GetClass(), GetActorLocation(), GetActorRotation(), Params);
}

ANavigationData* GetNavData()
{
	UWorld* World = GEngine->GetWorld();
	if (World)
	{
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
		if (NavSys)
		{
			ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
			return NavData;
		}
	}
	return nullptr;
}


UStaticMesh* ABuildingResizable::MakeMesh(TArray<FVector> locations)
{
	UStaticMesh* staticMesh = NewObject<UStaticMesh>(this);
	staticMesh->GetStaticMaterials().Add(FStaticMaterial());

	TArray<FVector2f> zz;
	for (FVector Location : locations) { zz.Add(FVector2f(Location.X, Location.Y)); }
	TArray<UE::Geometry::FIndex3i> triangles;

	PolygonTriangulation::TriangulateSimplePolygon(zz, triangles, true);
	FMeshDescription meshDesc;

	FStaticMeshAttributes Attributes(meshDesc);
	Attributes.Register();

	FMeshDescriptionBuilder meshDescriptionBuilder;
	meshDescriptionBuilder.SetMeshDescription(&meshDesc);
	meshDescriptionBuilder.EnablePolyGroups();
	meshDescriptionBuilder.SetNumUVLayers(1);

	TArray<FVertexID> VertexIDs;
	Border->ClearSplinePoints();

	int index = 0;
	for (FVector Vector : locations)
	{
		FSplinePoint point(index, Vector);
		Border->AddPoint(point, true);
		Border->SetSplinePointType(index, ESplinePointType::Linear);
		VertexIDs.Add(meshDescriptionBuilder.AppendVertex(Vector));

		index++;
	}
	Border->SetClosedLoop(true, true);

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
	int32 ind = 0;
	for (UE::Geometry::FIndex3i& Triangle : triangles)
	{
		// GEngine->AddOnScreenDebugMessage(INDEX_NONE, 60, FColor::Orange, FString::Printf(TEXT("i:%d A: %d | B: %d | C: %d "), ind, Triangle.A, Triangle.B, Triangle.C));
		AppendTriangle({
			VertexInfo(Triangle.A, FVector(0, 0, 1), FVector2D(0, 1)),
			VertexInfo(Triangle.B, FVector(0, 0, 1), FVector2D(1, 0)),
			VertexInfo(Triangle.C, FVector(0, 0, 1), FVector2D(0, 0)),
		});

		FColor Color = FColor::MakeRandomColor();
		FVector A = UKismetMathLibrary::TransformLocation(GetTransform(), locations[Triangle.A]);
		FVector B = UKismetMathLibrary::TransformLocation(GetTransform(), locations[Triangle.B]);
		FVector C = UKismetMathLibrary::TransformLocation(GetTransform(), locations[Triangle.C]);
		DrawDebugPoint(GetWorld(), A, 10, Color, false, 30);
		DrawDebugPoint(GetWorld(), B, 10, Color, false, 30);
		DrawDebugPoint(GetWorld(), C, 10, Color, false, 30);
		DrawDebugLine(GetWorld(), A, B, Color, false, 30);
		DrawDebugLine(GetWorld(), B, C, Color, false, 30);
		DrawDebugLine(GetWorld(), C, A, Color, false, 30);

		++ind;
	}

	// for (int i = 0; i < locations.Num() - 1; ++i)
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
	// 	VertexInfo(locations.Num(), FVector(0, 0, 1), FVector2D(1, 0)),
	// 	VertexInfo(1, FVector(0, 0, 1), FVector2D(0, 0)),
	// });

	UStaticMesh::FBuildMeshDescriptionsParams mdParams;
	mdParams.bBuildSimpleCollision = true;
	mdParams.bFastBuild = true;

	TArray<const FMeshDescription*> meshDescPtrs;
	meshDescPtrs.Emplace(&meshDesc);

	staticMesh->BuildFromMeshDescriptions(meshDescPtrs, mdParams);

	return staticMesh;
}

FVector ABuildingResizable::GetCurrentSplinePoint(USplineComponent* SplineComp, FVector Loc)
{
	FVector Out;
	SplineComp->GetClosestPointOnCollision(Loc, Out);
	return Out;
}

void ABuildingResizable::OnClick(ARTSController* Ctrl)
{
	if (StartLocation == FVector::ZeroVector && BuildingMode == EBuildingMode::BM_Follow)
	{
		BuildingMode = EBuildingMode::BM_StaticLocation;
		bIsBuilding = true;
		StartLocation.X = (int)(Controller->CursorHitLocation.X / 100);
		StartLocation.Y = (int)(Controller->CursorHitLocation.Y / 100);
		StartLocation.Z = (int)(Controller->CursorHitLocation.Z);
	}
	else if (BuildingMode == EBuildingMode::BM_StaticLocation)
	{
		if (HitUnderCursor.bBlockingHit)
		{
			if (HitUnderCursor.GetActor() != nullptr && HitUnderCursor.Component != nullptr)
			{
				if (component == nullptr)
				{
					if (HitUnderCursor.GetActor())
					{
						if (boxes.Contains(HitUnderCursor.GetActor()))
						{
							int32 index;
							if (Cast<APointActor>(HitUnderCursor.GetActor()))
							{
								boxes.Find(Cast<APointActor>(HitUnderCursor.GetActor()), index);
								component = boxes[index];
								// GEngine->AddOnScreenDebugMessage(58, 100, FColor::White,
								//                                  FString::Printf(TEXT("Select Box: %d"), index));
							}
						}
						else
						{
							TArray<FVector> path;
							if (HitUnderCursor.bBlockingHit)
								FindMeshPoints(path);
						}
					}
				}
			}
			else { component = nullptr; }
		}
		else { component = nullptr; }
	}
	else { component = nullptr; }
}

void ABuildingResizable::FindMeshPoints(TArray<FVector>& path)
{
	TArray<FHitResult> SweepResult = MakeHit(GetActorLocation(), HitUnderCursor.Location, FRotator(0, 0, 0), TArray<AActor*>(), 0);
	if (SweepResult.Num() < 1)
		return;
	FHitResult FResult = SweepResult[0];
	if (!FResult.bBlockingHit || !FResult.GetActor()->IsA<ABuildingSpline>()) return;
	ABuildingSpline* Building = Cast<ABuildingSpline>(FResult.GetActor());
	int32 pointIndex;

	DrawDebugSphere(GetWorld(), FindNearestSplinePointToWorldLocation(Building->SplineComponent, FResult.Location, pointIndex), 30, 30, FColor::Blue, false, 5);
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Blue, FString::Printf(TEXT("PointIndex = %d"), pointIndex));
	
	TArray<FHitResult> HitResults;
	USplineComponent* CurrentSpline = Building->SplineComponent;
	USplineComponent* PreviousSpline = nullptr;
	TArray<AActor*> IgnoredActors;
	TArray<AActor*> CollidingActors;

	int32 order = 0;
	int32 target;

	if (Sign(CurrentSpline->GetNumberOfSplinePoints() - pointIndex) != -1) { target = CurrentSpline->GetNumberOfSplinePoints(); }
	else target = 0;
	int z = 0;
	bool bHasJumped = false;
	for (int k = pointIndex; k != target; k += Sign(CurrentSpline->GetNumberOfSplinePoints() - k))
	{
		FVector CenterDir;
		if (target == 0)
			CenterDir = CurrentSpline->GetRightVectorAtSplinePoint(k, ESplineCoordinateSpace::Local) * -1;
		else
			CenterDir = CurrentSpline->GetRightVectorAtSplinePoint(k, ESplineCoordinateSpace::Local);
		DrawDebugString(GetWorld(), CurrentSpline->GetLocationAtSplinePoint(k, ESplineCoordinateSpace::World), FString::Printf(TEXT("i:%d"), order), NULL, FColor::White, 100);
		++order;
		IgnoredActors.Empty();
		IgnoredActors.Add(CurrentSpline->GetOwner());
		CollidingActors.AddUnique(CurrentSpline->GetOwner());
		if (PreviousSpline) { CollidingActors.AddUnique(PreviousSpline->GetOwner()); }

		//MultiBoxTrace For Detecting Splines
		HitResults = MakeHit(
			CurrentSpline->GetLocationAtSplinePoint(k, ESplineCoordinateSpace::World),
			CurrentSpline->GetLocationAtSplinePoint(k, ESplineCoordinateSpace::World),
			CurrentSpline->GetDirectionAtSplinePoint(k, ESplineCoordinateSpace::Local).Rotation(),
			IgnoredActors, 1
		);

		TSet<USplineComponent*> SplineComponents;
		TArray<AActor*> Actors;
		for (FHitResult& HitResult : HitResults)
		{
			if (HitResult.GetActor() && HitResult.GetActor()->IsA(ABuildingSpline::StaticClass()))
			{
				SplineComponents.Add(Cast<ABuildingSpline>(HitResult.GetActor())->SplineComponent);
				Actors.Add(HitResult.GetActor());
			}
		}

		TArray<AActor*> Temp;
		for (AActor* CollidingActor : CollidingActors)
		{
			if (Actors.Contains(CollidingActor))
				Temp.Add(CollidingActor);
		}
		CollidingActors = Temp;

		double MostCloseAngle = -1;
		USplineComponent* MostCloseSpline = nullptr;
		FVector CurrentPos, CurrentDir;
		int32 OutIndex = 0;
		CurrentPos = CurrentSpline->GetLocationAtSplinePoint(k, ESplineCoordinateSpace::World);
		CurrentDir = CenterDir;
		DrawDebugString(GetWorld(), (CurrentSpline->GetLocationAtSplinePoint(k, ESplineCoordinateSpace::World) + CenterDir * 220) + FVector(0, 0, 20), FString::Printf(TEXT("id:%d, coll:%d"), z, CollidingActors.Num()), NULL, FColor::White, 100);
		if (SplineComponents.Num() == 0) { path.Add(CurrentSpline->GetLocationAtSplinePoint(k, ESplineCoordinateSpace::World) + CenterDir * 220); }
		z++;

		int i = 0;
		for (USplineComponent* SplineComp : SplineComponents)
		{
			if (CollidingActors.Contains(SplineComp->GetOwner()))
				break;

			FindNearestSplinePointToWorldLocation(SplineComp, CurrentSpline->GetLocationAtSplinePoint(k, ESplineCoordinateSpace::World), OutIndex);
			FVector SplinePos = SplineComp->GetLocationAtSplinePoint(OutIndex, ESplineCoordinateSpace::World);
			FVector SplineDir = SplineComp->GetDirectionAtSplinePoint(OutIndex, ESplineCoordinateSpace::Local);

			double Product = 0.0f;
			FColor Color = FColor::MakeRandomColor();
			Product = FVector::DotProduct(CurrentDir, SplineComp->GetDirectionAtSplinePoint(OutIndex, ESplineCoordinateSpace::Local));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, Color, FString::Printf(TEXT("i:%d | Product: %f"), i, Product));
			DrawDebugLine(GetWorld(), SplinePos + FVector::UpVector * i * 10, (SplinePos + FVector::UpVector * i * 10) + SplineDir * 100, Color, true, -1, 21, 10);
			DrawDebugSphere(GetWorld(), (SplinePos + FVector::UpVector * i * 10) + SplineDir * 100, 20, 20, Color, true, -1, 21, 10);
			DrawDebugString(GetWorld(), (SplinePos + FVector::UpVector * i * 10) + SplineDir * 100, FString::Printf(TEXT("i:%d | k: %d | Product: %f | SplinePos: %s"), i, k, Product, *SplinePos.ToString()), NULL, FColor::White, 100);

			if (!SplinePos.Equals(CurrentPos))
			{
				continue;
			}

			if (Product > MostCloseAngle)
			{
				MostCloseSpline = SplineComp;
				MostCloseAngle = Product;
			}
			if (CurrentSpline->GetOwner() == FResult.GetActor() && k >= pointIndex && bHasJumped)
			{
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Magenta, FString::Printf(TEXT("point: %d, k: %d"), pointIndex, k));
				TArray<FVector> localPath;
				for (FVector& Path : path)
				{
					localPath.Add(UKismetMathLibrary::InverseTransformLocation(GetTransform(), Path));
					DrawDebugSphere(GetWorld(), Path, 30, 30, FColor::Emerald, false, 5);
				}
				RootMesh->SetStaticMesh(MakeMesh(localPath));
				return;
			}
			++i;
		}

		if (MostCloseSpline)
		{
			bHasJumped = true;
			PreviousSpline = CurrentSpline;
			CurrentSpline = MostCloseSpline;
			for (USplineComponent* SplineComp : SplineComponents)
				CollidingActors.AddUnique(SplineComp->GetOwner());
			k = OutIndex - 1;
			if (Sign(CurrentSpline->GetNumberOfSplinePoints() - k) != -1) { target = CurrentSpline->GetNumberOfSplinePoints(); }
			else target = 0;
		}
	}


	//Algo::Reverse(path);
	TArray<FVector> localPath;
	int a = 0;
	for (FVector& Path : path)
	{
		localPath.Add(UKismetMathLibrary::InverseTransformLocation(GetTransform(), Path));
		DrawDebugPoint(GetWorld(), Path, 30, FColor::Blue, false, 5);
		//DrawDebugString(GetWorld(), Path + FVector(0, 0, 20), FString::Printf(TEXT("id:%d"), a), NULL, FColor::White, 100);

		++a;
	}
	//MakeMesh(localPath);
}

void ABuildingResizable::AddPointToSpline(USplineComponent* SplineComponent, int32 Index, FVector Location)
{
	Border->AddSplinePointAtIndex(Location, Index, ESplineCoordinateSpace::World, true);
	Border->SetSplinePointType(Index, ESplinePointType::Linear);

	TArray<FVector> SplinePoints;
	for (int i = 0; i < Border->GetNumberOfSplinePoints(); ++i) { SplinePoints.Add(Border->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local)); }
	DrawPointActors(SplinePoints);
	RootMesh->SetStaticMesh(MakeMesh(SplinePoints));
}


void ABuildingResizable::FindIndexBetweenPoints(USplineComponent* SplineComponent, FVector Location, int32& LowerIndex,
                                                int32& UpperIndex)
{
	FInterpCurveVector vecs = Border->GetSplinePointsPosition();
	for (int index = 0; index < Border->GetNumberOfSplinePoints(); ++index)
	{
		if (index == Border->GetNumberOfSplinePoints())
		{
			FVector locStart = Border->GetLocationAtSplinePoint(index, ESplineCoordinateSpace::World);
			FVector locEnd = Border->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

			if ((Location - locStart).Normalize() == (Location - locEnd).Normalize())
			{
				LowerIndex = index;
				UpperIndex = 0;
				return;
			}
			return;
		}
		FVector locStart = Border->GetLocationAtSplinePoint(index, ESplineCoordinateSpace::World);
		FVector locEnd = Border->GetLocationAtSplinePoint(index + 1, ESplineCoordinateSpace::World);

		FVector normal1 = (Location - locStart);
		FVector normal2 = (Location - locEnd);
		UKismetMathLibrary::Vector_Normalize(normal1, 0.001);
		UKismetMathLibrary::Vector_Normalize(normal2, 0.001);

		if (UKismetMathLibrary::Vector_GetAbs(normal1).Equals(UKismetMathLibrary::Vector_GetAbs(normal2)))
		{
			LowerIndex = index;
			UpperIndex = index + 1;
			return;
		}
	}
}

bool ABuildingResizable::DidResized()
{
	int x = SizeX;
	int y = SizeY;
	FVector Distance = EndLocation - StartLocation;
	SizeX = Distance.X + Sign(Distance.X) * 1;
	SizeY = Distance.Y + Sign(Distance.Y) * 1;

	if (x == SizeX && y == SizeY) { return false; }
	return true;
}

void ABuildingResizable::OnRender() { Super::OnRender(); }

void ABuildingResizable::UpdateDuringBuilding(FVector Endloc, FHitResult& Hit)
{
	Super::UpdateDuringBuilding(Endloc, Hit);
	EndLocation = Endloc;
	EndLocation.X = (int)(Endloc.X / 100);
	EndLocation.Y = (int)(Endloc.Y / 100);
	EndLocation.Z = (int)Endloc.Z;
	if (DidResized() && bIsBuilding)
	{
		//UpdateGrid();
	}
}

void ABuildingResizable::DrawPointActors(TArray<FVector>& Locations)
{
	for (APointActor* Box : boxes) { Box->Destroy(); }

	boxes.Empty();
	for (FVector& Vertex : Locations)
	{
		//Locations.Add(Vertex);
		APointActor* actor = GetWorld()->SpawnActor<APointActor>(SpawnItem);
		actor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		actor->SetActorLocation(
			UKismetMathLibrary::TransformLocation(GetTransform(),
			                                      Vertex - (actor->GetRootComponent()->Bounds.BoxExtent / 2)));
		boxes.Add(actor);
	}
}


void ABuildingResizable::UpdateGrid()
{
}


void ABuildingResizable::StickToGrid(FVector& Location)
{
	//Super::StickToGrid(Location);
	FVector Loc, Dir;
	FHitResult Hit;
	FCollisionQueryParams CollisionParameters;
	FCollisionResponseParams CollisionResponseParameters;
	for (APointActor* actor : boxes) { CollisionParameters.AddIgnoredActor(actor); }
	if (component) { CollisionParameters.AddIgnoredActor(component); }
	if (BuildingMode == EBuildingMode::BM_Follow) { CollisionParameters.AddIgnoredActor(this); }


	Controller->DeprojectMousePositionToWorld(Loc, Dir);
	GetWorld()->LineTraceSingleByChannel(Hit, Loc, Dir * 5000 + Loc, ECC_Visibility, CollisionParameters,
	                                     CollisionResponseParameters);
	if (!Hit.bBlockingHit)
		return;

	if (BuildingMode == EBuildingMode::BM_Follow) { SetActorLocation(Hit.Location); }
	else if (BuildingMode == EBuildingMode::BM_StaticLocation)
	{
		if (component != nullptr)
		{
			if (Hit.bBlockingHit)
			{
				component->SetActorLocation(Hit.Location);
				TArray<FVector> Locations;
				Border->ClearSplinePoints();
				for (int i = 0; i < boxes.Num(); ++i)
				{
					Border->SetLocationAtSplinePoint(i, Hit.Location, ESplineCoordinateSpace::World, false);
					Border->SetSplinePointType(i, ESplinePointType::Linear);
					Locations.Add(
						UKismetMathLibrary::InverseTransformLocation(GetTransform(), boxes[i]->GetActorLocation()));
				}
				Border->UpdateSpline();
				RootMesh->SetStaticMesh(MakeMesh(Locations));
			}
		}
	}
	return;
}

TArray<FHitResult> ABuildingResizable::MakeHit(FVector StartLoc, FVector EndLoc, FRotator Rot, TArray<AActor*> IgnoredActors, int32 type)
{
	FVector BoxExtent = FVector(1, 1, 50.0f);
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(BoxExtent);

	FQuat Rotation = UKismetMathLibrary::MakeRotFromX(Rot.Vector()).Quaternion();

	// Perform the box trace
	FCollisionQueryParams QueryParams(FName(TEXT("spline")), true);
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	for (APointActor* Box : boxes)
		ActorsToIgnore.Add(Box);
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Append(IgnoredActors);


	TArray<TEnumAsByte<EObjectTypeQuery>> types;
	//types.Add(EObjectTypeQuery::ObjectTypeQuery1);
	//	USplineComponent* currentSpline;
	// actorloc, hitunderloc
	if (type == 1)
	{
		UKismetSystemLibrary::BoxTraceMultiByProfile(GetWorld(), StartLoc + FVector(0, 0, 500), StartLoc + FVector(0, 0, -1000),
		                                             FVector(30, 30, 30), Rotation.Rotator(),
		                                             FName(TEXT("splinemeshes")),
		                                             true, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResults, true);
	}
	else
	{
		UKismetSystemLibrary::BoxTraceMultiByProfile(GetWorld(), StartLoc, EndLoc,
		                                             FVector(60, 200, 50), Rotation.Rotator(),
		                                             FName(TEXT("splinemeshes")),
		                                             true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}

	for (FHitResult& HitResult : HitResults)
	{
		if (HitResult.bBlockingHit)
		{
			FColor Color = FColor::MakeRandomColor();
			DrawDebugBox(GetWorld(), StartLoc, FVector(60, 200, 50), Color, false, 20);
			DrawDebugBox(GetWorld(), EndLoc, FVector(60, 200, 50), Color, false, 20);
			DrawDebugPoint(GetWorld(), HitResult.Location, 5, FColor::White, false, 20);
		}
	}
	types.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));


	//GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, Rotation, ECC_GameTraceChannel1, CollisionShape, QueryParams);
	return HitResults;
}

int32 ABuildingResizable::Sign(int32 value)
{
	if (value >= 0) { return 1; }
	return -1;
}

FVector ABuildingResizable::FindNearestSplinePointToWorldLocation(USplineComponent* SplineComponent,
                                                                  const FVector& ClickLocation,
                                                                  int& OutIndex)
{
	FVector LineLocation = SplineComponent->FindLocationClosestToWorldLocation(
		ClickLocation, ESplineCoordinateSpace::World);
	float NearestDistanceSq = FLT_MAX;
	FVector NearestPoint;

	// Tüm spline noktalarını dönerek en yakın noktayı bulma
	for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		FVector SplinePoint = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

		float DistanceSq = FVector::DistSquared(SplinePoint, LineLocation);
		if (DistanceSq < NearestDistanceSq)
		{
			NearestDistanceSq = DistanceSq;
			NearestPoint = SplinePoint;
			OutIndex = i;
		}
	}

	return NearestPoint;
}

// void ABuildingResizable::LoopTroughSplines(ABuildingSpline* SplineBuilding)
// {
// 	FVector OutLocation;
// 	int32 lower = 0;
// 	int32 upper = 0;
// 	SplineBuilding->SplineComponent->GetClosestPointOnCollision(
// 		FResult.Location, OutLocation);
// 	FindIndexBetweenPoints(SplineBuilding->SplineComponent, OutLocation, lower, upper);
// 	TArray<FVector> path;
// 	for (int i = lower; i < SplineBuilding->SplineComponent->GetNumberOfSplinePoints(); ++i)
// 	{
// 		TArray<FHitResult> HitResults;
// 		UKismetSystemLibrary::BoxTraceMultiByProfile(
// 			GetWorld(),
// 			SplineBuilding->SplineComponent->GetWorldLocationAtSplinePoint(i),
// 			SplineBuilding->SplineComponent->GetWorldLocationAtSplinePoint(i) + 1,
// 			FVector(1000, 1000, 300),
// 			FRotator::ZeroRotator,
// 			FName(TEXT("splinemeshes")),
// 			true,
// 			TArray<AActor*>(),
// 			EDrawDebugTrace::ForDuration,
// 			HitResults,
// 			false);
// 		//GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FRotator(0,0,0).Quaternion(), ECC_GameTraceChannel1, FCollisionShape::MakeBox(FVector(30,30,30)));
//
// 		TArray<FHitResult> finals;
// 		for (FHitResult& HitResult : HitResults)
// 		{
// 			if (HitResult.GetActor() && HitResult.GetActor()->IsA(ABuildingSpline::StaticClass()))
// 				finals.Add(HitResult);
// 		}
//
// 		if (finals.Num() == 1)
// 		{
// 			path.Add(SplineBuilding->SplineComponent->GetWorldLocationAtSplinePoint(i));
// 			continue;
// 		}
// 		else if (finals.Num() == 2)
// 		{
// 			Algo::RemoveIf(finals, [SplineBuilding](const FHitResult& Element)
// 			{
// 				return Element.GetActor() == SplineBuilding;
// 			});
// 			for (FHitResult& Final : finals)
// 			{
// 				int32 lower1 = 0;
// 				int32 upper1 = 0;
// 				FindIndexBetweenPoints(Cast<ABuildingSpline>(Final.GetActor())->SplineComponent, Final.Location, lower1, upper1); 
// 			} 
// 			path.Add(SplineBuilding->SplineComponent->GetWorldLocationAtSplinePoint(i));
// 			continue;
// 		}
// 		else if (finals.Num() > 2)
// 		{
// 			Algo::RemoveIf(finals, [SplineBuilding](const FHitResult& Element)
// 			{
// 				return Element.GetActor() == SplineBuilding;
// 			});
// 			
// 		}
// 	}
// }
// @formatter:line_break_after_colon_in_member_initializer_lists restore
// @formatter:member_initializer_list_style restore
// @formatter:simple_block_style restore
// @formatter:toplevel_function_definition_return_type_style restore
// @formatter:max_line_length restore
