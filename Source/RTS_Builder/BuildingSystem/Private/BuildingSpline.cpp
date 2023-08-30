// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuildingSpline.h"

#include "FrameTypes.h"
#include "RTS_Builder/RTSController.h"
#include "Components/SplineComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Evaluation/Blending/MovieSceneBlendType.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GraphAStar.h"
#include "Landscape.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NavModifierVolume.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetGuidLibrary.h"
#include "Kismet/KismetInputLibrary.h"
#include "Navigation/NavLocalGridManager.h"
#include "NavMesh/RecastNavMesh.h"

ARTSController* Controller;
float delta;

ABuildingSpline::ABuildingSpline(const FObjectInitializer& OI)
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	InSceneBuildings = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SeveralMesh"));
	InSceneBuildings->SetupAttachment(GetRootComponent());
	InSceneBuildings->SetStaticMesh(StaticMesh);
	SplineComponent = CreateDefaultSubobject<USplineComponent>("spliner");
	SplineComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}

void ABuildingSpline::BeginPlay()
{
	Super::BeginPlay();
	NavAgent.AgentHeight = AgentHeight;
	NavAgent.AgentRadius = AgentHalfRadius;
	NavAgent.AgentStepHeight = 45.f;
	//NavAgent.NavWalkingSearchHeightScale = 35.f;
	SizeX = 1;
	SizeY = 1;
}

void ABuildingSpline::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	delta = DeltaSeconds;
	FString message;

	if (StartPoint.IsSet())
	{
		message.Append(TEXT(" StartPoint: "));
		message.Append(StartPoint.GetValue().ToString());
		message.Append(TEXT(" || "));
	}
	if (StartTan.IsSet())
	{
		message.Append(TEXT(" StartTan: "));
		message.Append(StartTan.GetValue().ToString());
		message.Append(TEXT(" || "));
	}
	if (EndPoint.IsSet())
	{
		message.Append(TEXT(" EndPoint: "));
		message.Append(EndPoint.GetValue().ToString());
		message.Append(TEXT(" || "));
	}
	if (EndTan.IsSet())
	{
		message.Append(TEXT(" EndTan: "));
		message.Append(EndTan.GetValue().ToString());
		message.Append(TEXT(" || "));
	}


	GEngine->AddOnScreenDebugMessage(36, 10, FColor::Cyan, FString::Printf(TEXT("Endpoint: %s"), *message));
	GEngine->AddOnScreenDebugMessage(37, 10, FColor::Green, FString::Printf(TEXT("Startloc: %s || Endloc: %s"), *StartLocation.ToString(), *EndLocation.ToString()));
}

FVector ABuildingSpline::GetNavAgentLocation() const
{
	return GetActorLocation();
}

const FNavAgentProperties& ABuildingSpline::GetNavAgentPropertiesRef() const
{
	static FNavAgentProperties NavAgentProperties;
	NavAgentProperties.AgentRadius = 200;
	return NavAgent;
}


void ABuildingSpline::OnConstruction(const FTransform& Transform)
{
}

void ABuildingSpline::OnClick(ARTSController* s)
{
	Controller = s;
	bIsBuilding = true;

	if (StartLocation == FVector(0, 0, 0))
	{
		FActorSpawnParameters Params;
		//ABuildingSpline* building = GetWorld()->SpawnActor<ABuildingSpline>(GetClass(), GetActorLocation(), GetActorRotation(), Params);
		TArray<FHitResult> Hits = MakeHit(
			Controller->CursorHitLocation,
			Controller->CursorHitLocation + 1,
			FVector(100, 100, 100),
			FRotator::ZeroRotator,
			TArray<AActor*>(),
			EDrawDebugTrace::ForDuration
		);

		for (FHitResult& FHit : Hits)
		{
			if (FHit.bBlockingHit && FHit.GetActor()->IsA(ABuildingSpline::StaticClass()))
			{
				USplineComponent* spline = Cast<ABuildingSpline>(FHit.GetActor())->SplineComponent;
				StartLocation = spline->FindLocationClosestToWorldLocation(FHit.Location, ESplineCoordinateSpace::World);
				int32 OutIndex;
				FVector LocalPos;
				LocalPos = FindNearestSplinePointToWorldLocation(spline, FHit.Location, OutIndex);
				int32 NumOfPoints = spline->GetNumberOfSplinePoints();

				FVector Loc = spline->FindLocationClosestToWorldLocation(FHit.Location, ESplineCoordinateSpace::World);
				FVector Tang = spline->FindTangentClosestToWorldLocation(FHit.Location, ESplineCoordinateSpace::World);
				float Key = spline->FindInputKeyClosestToWorldLocation(FHit.Location);
				float Dist = SplineComponent->GetDistanceAlongSplineAtSplineInputKey(Key);

				StartPoint = Loc;
				if (OutIndex == 0 || OutIndex == NumOfPoints -1)
				{
					StartPoint = spline->GetWorldLocationAtSplinePoint(OutIndex);
				}
				
				if (Dist <= 100 || Dist > spline->GetSplineLength() - 100)
					StartTan = Tang;
				else
					StartTan.Reset();
				return;
			}
		}
		StartTan.Reset();
		StartPoint.Reset();
		StartLocation = Controller->CursorHitLocation;
	}
	else
	{
		bIsBuilding = false;
		FActorSpawnParameters Params;
		ABuildingSpline* building = GetWorld()->SpawnActor<ABuildingSpline>(
			GetClass(),
			GetActorLocation(),
			GetActorRotation(),
			Params
		);
		building->StartTan = StartTan;
		building->EndTan = EndTan;
		if (EndPoint.IsSet())
		{
			building->MakeRoad(StartLocation, EndPoint.GetValue());
		}
		else
		{
			building->EndPoint = EndPoint;
			building->MakeRoad(StartLocation, EndLocation);

			//building->MakeRoad(StartLocation, EndPoint);
		}

		
		TArray<UActorComponent*> a = building->GetComponentsByClass(USplineMeshComponent::StaticClass());
		for (UActorComponent* A : a)
		{
			USplineMeshComponent* NewSplineMeshComponent = Cast<USplineMeshComponent>(A);
			if (NewSplineMeshComponent)
			{
				NewSplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				NewSplineMeshComponent->SetCollisionProfileName(FName("splines"), true);
			}
		}
		StartLocation = FVector(0, 0, 0);
	}
}

void ABuildingSpline::DrawGrid(int32 x, int32 y)
{
	FTransform tr = GetTransform();
	FVector s = StaticMesh->GetBounds().BoxExtent;
	tr.SetLocation(FVector(0, 0, 0) - FVector(s.X, s.Y, 0));
	InSceneBuildings->AddInstance(tr);
}

void ABuildingSpline::UpdateGrid()
{
}


void ABuildingSpline::OnRender()
{
	Super::OnRender();
}

void ABuildingSpline::OnStartBuilding(ARTSController* Ctrl)
{
	Controller = Ctrl;
	UE_LOG(LogTemp, Warning, TEXT("controller assigned"))
}


void ABuildingSpline::UpdateDuringBuilding(FVector Endloc, FHitResult& Hit)
{
	if (!bIsBuilding)
	{
		return;
	}
	EndLocation = Endloc;
	//EndPoint = Endloc;
	TArray<AActor*> Ignored;
	Ignored.Add(this);
	TArray<FHitResult> Hits = MakeHit(
		Controller->CursorHitLocation,
		Controller->CursorHitLocation + 1,
		FVector(100, 100, 100),
		FRotator::ZeroRotator,
		Ignored,
		EDrawDebugTrace::None
	);
	if (Hits.Num() > 0)
	{
		for (FHitResult& FHit : Hits)
		{
			if (FHit.bBlockingHit && FHit.GetActor()->IsA(ABuildingSpline::StaticClass()))
			{
				int32 OutIndex;
				USplineComponent* spline = Cast<ABuildingSpline>(FHit.GetActor())->SplineComponent;
				FVector LocalPos;
				LocalPos = FindNearestSplinePointToWorldLocation(spline, FHit.Location, OutIndex);
				//EndTan = spline->GetArriveTangentAtSplinePoint(OutIndex, ESplineCoordinateSpace::World);

				FVector Loc = spline->FindLocationClosestToWorldLocation(FHit.Location, ESplineCoordinateSpace::World);
				FVector Tang = spline->FindTangentClosestToWorldLocation(FHit.Location, ESplineCoordinateSpace::Local);
				float Key = spline->FindInputKeyClosestToWorldLocation(FHit.Location);
				float Dist = spline->GetDistanceAlongSplineAtSplineInputKey(Key);

				EndPoint = Loc;
				if (Dist <= 100 || Dist > spline->GetSplineLength() - 100)
					EndTan = Tang;
				else
					EndTan.Reset();

				GEngine->AddOnScreenDebugMessage(36, 10, FColor::Cyan, FString::Printf(TEXT("Index: %d ||| Loc:%s ||| Tang:%s ||| Key:%f ||| Dist:%f "), OutIndex, *Loc.ToString(), *Tang.ToString(), Key, spline->GetDistanceAlongSplineAtSplineInputKey(Key)));
				// DrawDebugString(
				// 	GetWorld(),
				// 	spline->GetLocationAtSplinePoint(OutIndex, ESplineCoordinateSpace::World),
				// 	FString::Printf(TEXT("ATTACHMENT")),
				// 	NULL,
				// 	FColor::Yellow,
				// 	10.0f,
				// 	false
				// );
				// DrawDebugSphere(
				// 	GetWorld(),
				// 	spline->GetLocationAtSplinePoint(OutIndex, ESplineCoordinateSpace::World),
				// 	30,
				// 	30,
				// 	FColor::Emerald,
				// 	false,
				// 	10
				// );
				break;
			}
			// EndPoint.Reset();
			// EndTan.Reset();
		}
	}
	else
	{
		EndPoint.Reset();
		EndTan.Reset();
	}

	//
	
	if (Controller && Controller->GridManager)
	{
		//InSceneBuildings->ClearInstances();
		SplineComponent->ClearSplinePoints();
		// TArray<UActorComponent*> a = GetComponentsByClass(USplineMeshComponent::StaticClass());
		// for (UActorComponent* A : a)
		// {
		// 	A->DestroyComponent();
		// }
		for (UActorComponent* Component : GetComponents())
		{
			if (Component->IsA(UStaticMeshComponent::StaticClass()) || Component->IsA(USplineMeshComponent::StaticClass()))
				Component->DestroyComponent();
		} 
		if (!EndPoint.IsSet())
		{
			MakeRoad(StartLocation, Endloc);
		}
		else
		{
			MakeRoad(StartLocation, EndPoint.GetValue());
		}
		//DrawDebugBox(GetWorld(), EndPoint, FVector(50, 50, 50), FColor::Silver, false, 1);
	}
	
}

void ABuildingSpline::Resize()
{
	FVector Distance = EndLocation - StartLocation;
	SizeX = Distance.X + Sign(Distance.X) * 1;
	SizeY = Distance.Y + Sign(Distance.Y) * 1;
}

bool ABuildingSpline::DidResized()
{
	int x = SizeX;
	int y = SizeY;
	Resize();
	if (x == SizeX && y == SizeY)
	{
		return false;
	}
	return true;
}

void ABuildingSpline::StickToGrid(FVector& Location)
{
	Super::StickToGrid(Location);
	FVector Origin, Box;
	GetActorBounds(false, Origin, Box, true);
	if (bIsBuilding && !hasmoved)
	{
		hasmoved = true;
		SetActorLocation(
			FVector(
				StartLocation.X * 100 + Box.X * Sign(SizeX),
				StartLocation.Y * 100 + Box.Y * Sign(SizeY),
				StartLocation.Z
			)
		);
	}
	else if (!bIsBuilding)
	{
		hasmoved = false;
		SetActorLocation(
			FVector(
				((int)EndLocation.X / 100) * 100 + ((int)Box.X % 100) * Sign(SizeX),
				((int)EndLocation.Y / 100) * 100 + ((int)Box.Y % 100) * Sign(SizeY),
				EndLocation.Z
			)
		);
	}
}

int32 ABuildingSpline::Sign(int32 value)
{
	if (value >= 0)
	{
		return 1;
	}
	return -1;
}

void ABuildingSpline::MakeRoad(FVector Start, FVector End)
{
	SplineComponent->ClearSplinePoints();
	for (UActorComponent* Component : GetComponents())
	{
		if (Component->IsA(USplineMeshComponent::StaticClass()))
		{
			Component->DestroyComponent();
		}
	}
	FindPathBetweenPoints(Start, End);
	CreateRoadSegment();
}

void ABuildingSpline::CreateRoadSegment()
{
	float SplineLength = SplineComponent->GetSplineLength() - 1;
	int32 NumberOfSegments = FMath::CeilToInt(SplineLength / MeshLength);
	if (NumberOfSegments == 0)
		NumberOfSegments = 1;
	//int32 AdditionalLength = ((int)SplineLength % (int)MeshLength) / NumberOfSegments;
	TOptional<FVector> PreviousEndTangent;

	if (NumberOfSegments >= 0)
	{
		float StretchedMeshLength = SplineLength / NumberOfSegments;
		if (StretchedMeshLength > MeshLength * 1.5f)
		{
			StretchedMeshLength /= 2;
			NumberOfSegments *= 2;
		}
		for (int i = 0; i < NumberOfSegments; ++i)
		{
			float StartValue = StretchedMeshLength * i;
			float EndValue = StretchedMeshLength * (i + 1);

			FVector Start = SplineComponent->GetLocationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::World);
			FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local);
			StartTang = StartTang.GetSafeNormal() * StretchedMeshLength;
			StartTang = StartTang.GetClampedToMaxSize(StretchedMeshLength);
			
			DrawDebugSphere(GetWorld(), Start,  30, 10, FColor::Green, false, 0);
			
			
			FVector End = SplineComponent->GetLocationAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::World);
			FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::Local);
			EndTang = StretchedMeshLength * EndTang.GetSafeNormal();
			EndTang = EndTang.GetClampedToMaxSize(StretchedMeshLength);

			DrawDebugSphere(GetWorld(), End, 30, 10, FColor::Red, false, 0);

			USplineMeshComponent* NewSplineMeshComponent = NewObject<USplineMeshComponent>(this);
			NewSplineMeshComponent->RegisterComponent();
			NewSplineMeshComponent->SetMobility(EComponentMobility::Movable);
			NewSplineMeshComponent->SetStaticMesh(StaticMesh);
			NewSplineMeshComponent->SetMaterial(0, MaterialVirtual);
			if (MeshLength < 300)
			{
				NewSplineMeshComponent->RuntimeVirtualTextures.Add(RuntimeVirtualTextureMaterial);
				NewSplineMeshComponent->CastShadow = false;
			}
			NewSplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			NewSplineMeshComponent->SetCollisionProfileName(FName(TEXT("splinemeshes")));
			NewSplineMeshComponent->AttachToComponent(
				SplineComponent,
				FAttachmentTransformRules::KeepWorldTransform
			);

			;
			float RollAngleStart = SplineComponent->GetRotationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local).Roll / 55;
			float RollAngleEnd = SplineComponent->GetRotationAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::Local).Roll / 55;
			DrawDebugString(GetWorld(), Start + FVector(0,0, 400), FString::Printf(TEXT("RollStart:%f || RollEnd:%f"), RollAngleStart, RollAngleEnd), 0, FColor::Turquoise, 0);
			
			NewSplineMeshComponent->SetStartRoll(RollAngleStart);
			NewSplineMeshComponent->SetEndRoll(RollAngleEnd);
			
			if (i == 0)
			{
				if (StartTan.IsSet())
					StartTang = StartTan.GetValue().GetSafeNormal() * StretchedMeshLength;
			}
			if (i == NumberOfSegments - 1)
			{
				if (EndTan.IsSet())
					EndTang = EndTan.GetValue().GetSafeNormal() * StretchedMeshLength;
				// else
				// 	EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local).GetSafeNormal() * StretchedMeshLength;
				FColor color;
				color = (EndValue == SplineLength) ? FColor::Green : FColor::Red;
				// DrawDebugString(GetWorld(), End + FVector(0,0, 400), FString::Printf(TEXT("EndValue:%f, SplineLength:%f"), EndValue, SplineLength), 0, color, 0);
			}

			// EndTang = TangentStart;
			// StartTang = TangentEnd;
			// DrawDebugString(GetWorld(), End, FString::Printf(TEXT("EndTan: %s"), *EndTang.ToString()), 0, FColor::White, 0);

			NewSplineMeshComponent->SetStartAndEnd(Start, StartTang, End, EndTang);
		}
	}
}

TArray<FVector> ABuildingSpline::FindPathBetweenPoints(const FVector& StartCoord, const FVector& EndCoord)
{
	float RemainingMovement = 30;
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FPathFindingQuery PathFindingQuery(this, *NavSystem->MainNavData, StartCoord, EndCoord);
	PathFindingQuery.QueryFilter = UNavigationQueryFilter::GetQueryFilter(*NavSystem->MainNavData, QueryFilter);
	FPathFindingResult Path = NavSystem->FindPathSync(NavAgent, PathFindingQuery);

	FNavLocation OutLocation;
	FHitResult LandHit;
	float GroundZ;
	GetWorld()->LineTraceSingleByChannel(LandHit, SplineComponent->GetWorldLocationAtSplinePoint(0) + FVector(0,0,1000), SplineComponent->GetWorldLocationAtSplinePoint(0) - FVector(0,0,2000), ECC_Visibility);
	if (LandHit.bBlockingHit)
		GroundZ = LandHit.Location.Z;
	
	FVector End(0.f, 0.f, 0.f);
	TArray<FVector> OldPoints;
	TArray<float> Distances;
	TArray<FVector> Locats;
	if (Path.IsSuccessful())
	{
		for (FNavPathPoint& PathPoint : Path.Path->GetPathPoints())
		{
			Locats.Add(PathPoint.Location);
		}

		SplineComponent->SetSplinePoints(Locats, ESplineCoordinateSpace::World);
		for (int i = 0; i < Locats.Num(); ++i)
		{
			//SplineComponent->SetTangentAtSplinePoint(i, FVector::ZeroVector, ESplineCoordinateSpace::World);
			float Dist = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i);
			Distances.Push(Dist);
			OldPoints.Push(Locats[i]);
			End = Locats[i];
			if (Dist > RemainingMovement * 100)
				break;
		}
		SplineComponent->SetSplinePoints(OldPoints, ESplineCoordinateSpace::World);
		for (int i = 0; i < OldPoints.Num(); ++i)
		{
			SplineComponent->SetTangentAtSplinePoint(i, FVector::ZeroVector, ESplineCoordinateSpace::World);
		}
	}

	float AdjustedZ = GroundZ;
	int32 LastIndex = -1;
	TArray<FVector> NewPoints;
	FVector LastPoint = FVector::ZeroVector;
	int PrecisionFactor = 25;
	float MaxDifference = 5;
	float TotalLength;
	if (SplineComponent->GetSplineLength() < RemainingMovement * 100)
		TotalLength = SplineComponent->GetSplineLength();
	else
		TotalLength = RemainingMovement * 100;

	FNavLocation FirstLocation;
	NavSystem->ProjectPointToNavigation(SplineComponent->GetWorldLocationAtSplinePoint(0), FirstLocation, FVector::One() * 100, &NavAgent, PathFindingQuery.QueryFilter);
	TOptional<float> FirstHeight = LandscapeProxy->GetHeightAtLocation(SplineComponent->GetWorldLocationAtSplinePoint(0), EHeightfieldSource::Complex);
	GetWorld()->LineTraceSingleByChannel(LandHit, SplineComponent->GetWorldLocationAtSplinePoint(0) + FVector(0,0,1000), SplineComponent->GetWorldLocationAtSplinePoint(0) - FVector(0,0,2000), ECC_Visibility);
	FVector FirstPoint;
	if (LandHit.bBlockingHit)
	{
		FirstPoint = LandHit.Location;
	}
	//FirstPoint.Z = FirstHeight.GetValue();
	NewPoints.Push(FirstPoint);
	
	FVector Nextor;
	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints() - 1; ++i)
	{
		float StartingDist = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i);
		float NextDist = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i + 1);
		float StartZ = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World).Z;
		float NextZ = SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World).Z;
		float Step = (NextDist - StartingDist) / 4;
		if (Step < PrecisionFactor || FMath::Abs(NextZ - StartZ) > PrecisionFactor)
			Step = PrecisionFactor;
		DrawDebugPoint(GetWorld(), SplineComponent->GetWorldLocationAtSplinePoint(i), 30.0f, FColor::Red, false, 0.f);
		Step = FMath::Clamp(Step, float(PrecisionFactor), 250.f);
		for (int j = StartingDist; (j + Step < NextDist) && (j < TotalLength); j += Step)
		{
			bool Push = false;
			FVector CurrentPoint = SplineComponent->GetWorldLocationAtDistanceAlongSpline(j);;
			NavSystem->ProjectPointToNavigation(CurrentPoint, OutLocation, FVector::One() * 100, &NavAgent, PathFindingQuery.QueryFilter);
			TOptional<float> CurrentHeight = LandscapeProxy->GetHeightAtLocation(CurrentPoint, EHeightfieldSource::Complex);
			GetWorld()->LineTraceSingleByChannel(LandHit, CurrentPoint + FVector(0,0,1000), CurrentPoint - FVector(0,0,2000), ECC_Visibility);
			FVector PointOnNav;
			if (LandHit.bBlockingHit)
				PointOnNav = LandHit.Location;

			FVector NextPoint = SplineComponent->GetWorldLocationAtDistanceAlongSpline(j + Step);
			GetWorld()->LineTraceSingleByChannel(LandHit, NextPoint + FVector(0,0,1000), NextPoint - FVector(0,0,2000), ECC_Visibility);
			FVector NextPointOnNav;
			if (LandHit.bBlockingHit)
				 NextPointOnNav = LandHit.Location;


			if (FMath::Abs(PointOnNav.Z - NextPointOnNav.Z) > MaxDifference)
			{
				Step = PrecisionFactor;
				Push = true;
			}

			if (FMath::Abs(AdjustedZ - PointOnNav.Z) > MaxDifference || Push)
			{
				if (j > StartingDist && j + Step <= NextDist )
				{
					 FVector Previous = SplineComponent->GetLocationAtDistanceAlongSpline(j - Step, ESplineCoordinateSpace::World);
					FVector Current = SplineComponent->GetLocationAtDistanceAlongSpline(j, ESplineCoordinateSpace::World);
					 FVector Next =SplineComponent->GetLocationAtDistanceAlongSpline(j + Step, ESplineCoordinateSpace::Local); 
					FHitResult first, second, last;
					GetWorld()->LineTraceSingleByChannel(first, Previous + FVector(0,0,1000), Previous - FVector(0,0,2000), ECC_Visibility);
					// bool bIsAligned = FVector::Dist((PointOnNav - first.Location).GetSafeNormal(), ((NextPointOnNav - PointOnNav).GetSafeNormal()));
					
					
					FVector Firstdiff = UKismetMathLibrary::GetDirectionUnitVector(first.Location, PointOnNav);
					FVector SecondDiff = UKismetMathLibrary::GetDirectionUnitVector(PointOnNav, NextPointOnNav);
					bool bIsAligned = FVector::Distance(Firstdiff, SecondDiff) <= 0.1f;
					FColor Color = bIsAligned ? FColor::Green : FColor::Red;
					
					if (!bIsAligned)
					{
						NewPoints.Push(PointOnNav);
					}
					else
					{
						DrawDebugLine(GetWorld(), PointOnNav, PointOnNav + SecondDiff * FVector::Dist(PointOnNav,NextPointOnNav), Color, false, 0, 3, 10);
					}


					DrawDebugString(GetWorld(), PointOnNav + FVector::UpVector * (i % 2) * -50, FString::Printf(TEXT("arriveDir: %s targetDir: %s"),
					*(Firstdiff.ToString()), *SecondDiff.ToString()), 0, Color, 0);
				}
				
				

			}
			AdjustedZ = PointOnNav.Z;
			End.Z = AdjustedZ;
			Nextor = NextPointOnNav;
		}
		if (SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) < TotalLength)
		{
			FVector Next = SplineComponent->GetWorldLocationAtSplinePoint(i + 1);
			
			NewPoints.Push(Nextor);
		}
	}
	
	
	
	SplineComponent->ClearSplinePoints();
	SplineComponent->SetSplinePoints(NewPoints, ESplineCoordinateSpace::World);
	int32 NumOfMeshes = FMath::CeilToInt(SplineComponent->GetSplineLength() / MeshLength);
	if (NumOfMeshes == 0)
		NumOfMeshes++;
	float StretchedMeshLength = SplineComponent->GetSplineLength() / NumOfMeshes;
	if (StretchedMeshLength > MeshLength * 1.5f)
	{
		StretchedMeshLength /= 2;
		NumOfMeshes *= 2;
	}
	// NewPoints.Push(Nextor.RightVector * StretchedMeshLength + Nextor);
	// if (NewPoints.Num() > 0)
	// {
	// 	FVector s = NewPoints.Last(0);
	// 	NewPoints.Push(s + s.ForwardVector * StretchedMeshLength);	
	// }
	
	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); ++i)
	{
		// SplineComponent->SetTangentAtSplinePoint(i, FVector::Zero(), ESplineCoordinateSpace::World, true);
		// SplineComponent->SetSplinePointType(i, ESplinePointType::CurveCustomTangent);
		FRotator rot = SplineComponent->GetRotationAtSplinePoint(i, ESplineCoordinateSpace::World);
		SplineComponent->SetRotationAtSplinePoint(i, FRotator(0, rot.Vector().Y, rot.Vector().Z), ESplineCoordinateSpace::World, true);
		if (i < SplineComponent->GetNumberOfSplinePoints() - 1)
		{
			float half;
			FVector loc,dir;
			half = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i + 1) - SplineComponent->GetDistanceAlongSplineAtSplinePoint(i);
			dir = SplineComponent->GetDirectionAtDistanceAlongSpline(SplineComponent->GetDistanceAlongSplineAtSplinePoint(i), ESplineCoordinateSpace::World);
			loc = SplineComponent->GetLocationAtDistanceAlongSpline(SplineComponent->GetDistanceAlongSplineAtSplinePoint(i), ESplineCoordinateSpace::World);
			FVector locNext = SplineComponent->GetLocationAtDistanceAlongSpline(SplineComponent->GetDistanceAlongSplineAtSplinePoint(i + 1), ESplineCoordinateSpace::World);
			SplineComponent->SetRotationAtSplinePoint(i, UKismetMathLibrary::FindLookAtRotation(loc, locNext), ESplineCoordinateSpace::World, true);
			FVector Nextdir = UKismetMathLibrary::GetDirectionUnitVector(loc, locNext);;
			
			if (i != 0)
			{
				float local =SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) - SplineComponent->GetDistanceAlongSplineAtSplinePoint(i - 1);
				dir = UKismetMathLibrary::GetDirectionUnitVector(SplineComponent->GetLocationAtSplinePoint(i - 1, ESplineCoordinateSpace::World), SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
				 if (local < half)
					half = local;
			}
		
			
			SplineComponent->SetTangentAtSplinePoint(i, Nextdir * half, ESplineCoordinateSpace::World); 
		}

		if (i == 0)
		{
		// if (StartTan.IsSet())
		// 	SplineComponent->SetTangentAtSplinePoint(i, StartTan.GetValue().GetSafeNormal() * StretchedMeshLength, ESplineCoordinateSpace::Local); 
		if (StartPoint.IsSet())
			SplineComponent->SetLocationAtSplinePoint(i, StartPoint.GetValue(), ESplineCoordinateSpace::World);
		}
			
		else if ( i == SplineComponent->GetNumberOfSplinePoints() - 1)
		{
			// if (EndTan.IsSet())
			// 	SplineComponent->SetTangentAtSplinePoint(i, EndTan.GetValue().GetSafeNormal() * StretchedMeshLength, ESplineCoordinateSpace::Local); 
			if (EndPoint.IsSet())
				SplineComponent->SetLocationAtSplinePoint(i, EndPoint.GetValue(), ESplineCoordinateSpace::World);
		}

		if (i != SplineComponent->GetNumberOfSplinePoints() - 1)
		{
			
			// float SegmentLength = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i + 1) - SplineComponent->GetDistanceAlongSplineAtSplinePoint(i);
			// //FVector Tan = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
			// FVector Tan = SplineComponent->GetTangentAtDistanceAlongSpline(SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) + 1, ESplineCoordinateSpace::Local) * SegmentLength;
			// SplineComponent->SetRotationAtSplinePoint(i, Tan.GetUnsafeNormal().Rotation(), ESplineCoordinateSpace::Local);	
		}
		
	}
	// SplineComponent->UpdateSpline();

	// if (SplineComponent->GetSplineLength() > RemainingMovement * 100)
	// {
	// 	float Diff = APMax * Speed - RemainingMovement;
	// 	float Length = (APMax * Speed - Diff) * 100;
	// 	SplineComponent->SetLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, SplineComponent->GetLocationAtDistanceAlongSpline(Length, ESplineCoordinateSpace::World), ESplineCoordinateSpace::World);
	// 	End = SplineComponent->GetLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
	// }

	return NewPoints;
}

FVector ABuildingSpline::FindNearestSplinePointToWorldLocation(
	USplineComponent* SplineComp,
	const FVector& ClickLocation,
	int& OutIndex
)
{
	FVector LineLocation = SplineComp->FindLocationClosestToWorldLocation(
		ClickLocation,
		ESplineCoordinateSpace::World
	);
	float NearestDistanceSq = FLT_MAX;
	FVector NearestPoint;

	// Tüm spline noktalarını dönerek en yakın noktayı bulma
	for (int32 i = 0; i < SplineComp->GetNumberOfSplinePoints(); i++)
	{
		FVector SplinePoint = SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

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

TArray<FHitResult> ABuildingSpline::MakeHit(
	FVector StartLoc,
	FVector EndLoc,
	FVector Size,
	FRotator Rot,
	TArray<AActor*> IgnoredActors,
	EDrawDebugTrace::Type DebugType
)
{
	FVector BoxExtent = FVector(50.0f, 50.0f, 50.0f);
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(BoxExtent);

	FQuat Rotation = UKismetMathLibrary::MakeRotFromX(Rot.Vector()).Quaternion();

	// Perform the box trace
	FCollisionQueryParams QueryParams(FName(TEXT("spline")), true);
	QueryParams.bTraceComplex = true;
	//QueryParams.bReturnPhysicalMaterial = false;
	FHitResult resse;
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);


	TArray<TEnumAsByte<EObjectTypeQuery>> types;
	//types.Add(EObjectTypeQuery::ObjectTypeQuery1);
	//USplineComponent* currentSpline;
	// actorloc, hitunderloc
	UKismetSystemLibrary::BoxTraceMultiByProfile(
		GetWorld(),
		StartLoc,
		EndLoc,
		Size,
		Rotation.Rotator(),
		FName(TEXT("splinemeshes")),
		true,
		ActorsToIgnore,
		DebugType,
		HitResults,
		true
	);

	types.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));


	//GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, Rotation, ECC_GameTraceChannel1, CollisionShape, QueryParams);
	return HitResults;
}

TArray<FVector> ABuildingSpline::DivideVector(const FVector& Start, const FVector& End, float DistanceInterval)
{
	TArray<FVector> DividedVectors;

	float Distance = FVector::Dist(Start, End);
	int32 NumSegments = FMath::Max(1, FMath::FloorToInt(Distance / DistanceInterval));
	float ActualDistanceInterval = Distance / static_cast<float>(NumSegments);

	FVector Direction = (End - Start).GetSafeNormal() * ActualDistanceInterval;

	for (int32 i = 0; i < NumSegments; ++i)
	{
		FVector Segment = Start + Direction * static_cast<float>(i);
		DividedVectors.Add(Segment);
	}

	// Son segmenti ekle (eğer aralık 960 birimden fazla ise kalan mesafeyi de ekler)
	FVector LastSegment = Start + Direction * static_cast<float>(NumSegments);
	DividedVectors.Add(LastSegment);

	return DividedVectors;
}
