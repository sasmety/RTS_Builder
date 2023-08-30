// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/SplineBuilding.h"

#include <mfidl.h>

#include "EngineUtils.h"
#include "RTS_Builder/RTSController.h"
#include "AssetTypeActions/AssetTypeActions_BlueprintGeneratedClass.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<AActor*> ignorants;

ASplineBuilding::ASplineBuilding()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	SplineComponent = CreateDefaultSubobject<USplineComponent>("RoadSpline");
	SplineComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	FSoftObjectPath MeshPath(TEXT("/Game/Assets/Buildings/Wall1.Wall1"));	
	Mesh = Cast<UStaticMesh>(MeshPath.TryLoad());

	FSoftObjectPath PillarPath(TEXT("/Game/Assets/Buildings/Pillar1.Pillar1"));	
	PillarMesh = Cast<UStaticMesh>(PillarPath.TryLoad());
	
	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>("InstancedMesh");
	InstancedStaticMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	InstancedStaticMeshComponent->SetStaticMesh(PillarMesh);
}

void ASplineBuilding::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ASplineBuilding::BeginPlay()
{
	Super::BeginPlay();
	ClearSpline();
}

void ASplineBuilding::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (SplineComponent->GetNumberOfSplinePoints() > 1)
    	{
    		GEngine->AddOnScreenDebugMessage(34, 10, FColor::Turquoise, FString::Printf(TEXT("FirstLoc: %s | LastLoc: %s"),
    			*SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World).ToString(),
    			*SplineComponent->GetLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World).ToString()));
    
    		GEngine->AddOnScreenDebugMessage(35, 10, FColor::Purple, FString::Printf(TEXT("FirstTan: %s | LastTan: %s"),
    			*SplineComponent->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::World).ToString(),
    			*SplineComponent->GetTangentAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World).ToString()));
    	}
}

void ASplineBuilding::Update(float DeltaSeconds, FHitResult& Hit)
{
	// GEngine->AddOnScreenDebugMessage(31, 10, FColor::Green, FString::Printf(TEXT("Spline Points: %d"), SplineComponent->GetNumberOfSplinePoints()));
	if (SplineComponent->GetNumberOfSplinePoints() > 0)
	{
		if (SplineComponent->GetNumberOfSplinePoints() > 1)
			SplineComponent->RemoveSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1);
		TArray<AActor*> AttachmentActors;
		TOptional<FVector> AttachPoint = FindAttachmentPoint(Hit.Location, AttachmentActors);
		if (AttachPoint.IsSet())
			Hit.Location = AttachPoint.GetValue();
		SplineComponent->AddSplineWorldPoint(Hit.Location);
		ClearSplineMeshes();
		GenerateSplineMesh(0, SplineComponent->GetNumberOfSplinePoints() - 1, Mesh);
	}
}

void  ASplineBuilding::OnClick(FVector& Point)
{
	AddSplinePoint(Point);	
}

int32 ASplineBuilding::GetClosestPoint(FVector& WorldPoint, USplineComponent* TargetSpline)
{
	FVector SplineLocation = TargetSpline->FindLocationClosestToWorldLocation(WorldPoint, ESplineCoordinateSpace::World);
	float StartDist = FVector::Distance(TargetSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World), WorldPoint);
	float EndDist = FVector::Distance(TargetSpline->GetLocationAtSplinePoint(TargetSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World), WorldPoint);
	if (StartDist < EndDist)
		return 0;
	return TargetSpline->GetNumberOfSplinePoints() - 1;
}

TOptional<FVector> ASplineBuilding::FindAttachmentPoint(FVector& WorldPoint, TArray<AActor*>& AttachmentActors)
{
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	UKismetSystemLibrary::BoxTraceMultiByProfile(GetWorld(), WorldPoint, WorldPoint, FVector(100, 100, 100), FRotator::ZeroRotator, FName(TEXT("splinemeshes")), true, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResults, true, FColor::Green, FColor::Red, 10);

	TOptional<FVector> AttachPoint;
	for (FHitResult& HitResult : HitResults) {
		if (HitResult.bBlockingHit)
		{
			GEngine->AddOnScreenDebugMessage(11, 10, FColor::Magenta, HitResult.GetActor()->GetName());
			if (HitResult.GetActor() && HitResult.GetActor()->IsA(ASplineBuilding::StaticClass()))
			{
				int32 index = GetClosestPoint(HitResult.Location, Cast<ASplineBuilding>(HitResult.GetActor())->SplineComponent);
				if (index != -1) {
					if (!AttachPoint.IsSet())
						AttachPoint = Cast<ASplineBuilding>(HitResult.GetActor())->SplineComponent->GetLocationAtSplinePoint(index, ESplineCoordinateSpace::World);
					AttachmentActors.Add(HitResult.GetActor());
					ignorants.Add(HitResult.GetActor());
				}
			}
		}
	}
	return AttachPoint;
}

bool ASplineBuilding::AddSplinePoint(FVector& WorldPoint)
{
	TArray<AActor*> Attachment;
	TOptional<FVector> Opt = FindAttachmentPoint(WorldPoint, Attachment);
	if (Opt.IsSet())
		WorldPoint = Opt.GetValue();
	SplineComponent->AddSplineWorldPoint(WorldPoint);
	SplineComponent->UpdateSpline();

	if (SplineComponent->GetNumberOfSplinePoints() > 1)
	{
		int32 lastIndex = SplineComponent->GetNumberOfSplinePoints() - 1;
		int32 secondLastIndex = SplineComponent->GetNumberOfSplinePoints() - 2;

		float StartDistance = SplineComponent->GetDistanceAlongSplineAtSplinePoint(secondLastIndex);
		float SegmentLength = SplineComponent->GetDistanceAlongSplineAtSplinePoint(lastIndex) - StartDistance;
		TArray<FVector> locs;
		//locs = FindPathBetweenPoints(SplineComponent->GetWorldLocationAtSplinePoint(0), SplineComponent->GetWorldLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1), ignorants);
		SplineComponent->RemoveSplinePoint(lastIndex, true);
		for (FVector& Loc : locs)
		{
			SplineComponent->AddSplineWorldPoint(Loc);
		}
		if (locs.Num() == 0)
			SplineComponent->AddSplineWorldPoint(WorldPoint);
	}
	
	if (SplineComponent->GetNumberOfSplinePoints() > 1)
	{
		SplineComponent->SetSplinePointType(0, ESplinePointType::CurveCustomTangent, true);
		SplineComponent->SetSplinePointType(SplineComponent->GetNumberOfSplinePoints() - 1, ESplinePointType::CurveCustomTangent, true);
		int32 last = SplineComponent->GetNumberOfSplinePoints() - 1;

		FVector StartTan, EndTan, StartLoc, EndLoc;
		StartLoc = SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		StartTan = SplineComponent->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local);
		EndLoc = SplineComponent->GetLocationAtSplinePoint(last, ESplineCoordinateSpace::World);
		EndTan = SplineComponent->GetTangentAtSplinePoint(last, ESplineCoordinateSpace::Local);

		float StartSegment = SplineComponent->GetDistanceAlongSplineAtSplinePoint(1) - SplineComponent->GetDistanceAlongSplineAtSplinePoint(0);
		float EndSegment = SplineComponent->GetDistanceAlongSplineAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1) - SplineComponent->GetDistanceAlongSplineAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 2);


		FVector a = (EndLoc - StartLoc).GetSafeNormal();
		FVector NormaledStart = a;
		NormaledStart *= StartSegment / 2;
		NormaledStart.Z = 0;

		FVector NormaledEnd = SplineComponent->GetDirectionAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
		NormaledEnd *= EndSegment / 2;
		NormaledEnd.Z = 0;

		DrawDebugBox(GetWorld(), NormaledStart, FVector(50,50,50), FColor::Blue, false, 10, 10, 3);
		SplineComponent->SetTangentAtSplinePoint(0, NormaledStart, ESplineCoordinateSpace::Local, true);

		DrawDebugBox(GetWorld(), NormaledEnd, FVector(50,50,50), FColor::Purple, false, 10, 10, 3);
		SplineComponent->SetTangentAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, NormaledEnd, ESplineCoordinateSpace::Local, true);
		SplineComponent->UpdateSpline();

		// GenerateSplineMesh(0, SplineComponent->GetNumberOfSplinePoints() - 1, Mesh);
		
	}
	return SplineComponent->GetNumberOfSplinePoints() > 1;
}

void ASplineBuilding::RotatePoint(int32 Index, float Value)
{
}

void ASplineBuilding::GenerateSplineMesh(int32 TargetStartIndex, int32 TargetEndIndex, UStaticMesh* StaticMesh)
{
	if (TargetEndIndex < TargetStartIndex)
	{
		int32 temp = TargetEndIndex;
		TargetEndIndex = TargetStartIndex; 
		TargetStartIndex = temp;
	}
	if (SplineComponent->GetNumberOfSplinePoints() < TargetEndIndex + 1 || TargetStartIndex < 0 || TargetEndIndex < 0 || TargetStartIndex == TargetEndIndex)
		return;

	FBoxSphereBounds Box = StaticMesh->GetBounds();
	float MeshLength = Box.BoxExtent.X * 2;
	
	for (int i = TargetStartIndex; i < TargetEndIndex ; ++i)
	{
		float StartDistance = 0;
		float SegmentLength = SplineComponent->GetSplineLength() - 1;
		GEngine->AddOnScreenDebugMessage(89, 20, FColor::Green, FString::Printf(TEXT("Dist: %f"), SegmentLength));

		// SegmentLength = SplineComponent->GetSplineLength() - 1;
		int NumOfMeshes = FMath::CeilToInt(SegmentLength / MeshLength);
		if (NumOfMeshes == 0) ++NumOfMeshes;
		float StretchLength = SegmentLength / NumOfMeshes;
		
		for (int j = 0; j < NumOfMeshes; ++j)
		{
			FVector StartLoc, StartTan, EndLoc, EndTan;

			float StartValue = StartDistance + (j * StretchLength);
			StartLoc = SplineComponent->GetWorldLocationAtDistanceAlongSpline(StartValue);
			StartTan = SplineComponent->GetTangentAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local).GetSafeNormal() * StretchLength;
			
			float EndValue = StartDistance + ((j + 1) * StretchLength);
			EndLoc = SplineComponent->GetWorldLocationAtDistanceAlongSpline(EndValue);
			EndTan = SplineComponent->GetTangentAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::Local).GetSafeNormal() * StretchLength;


			#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
				DrawDebugBox(GetWorld(), StartLoc, FVector(50,50,50), FColor::Green, false, 10);
				DrawDebugBox(GetWorld(), EndLoc + FVector(0,0,125), FVector(50,50,50), FColor::Yellow, false, 10);
				DrawDebugString(GetWorld(), SplineComponent->GetLocationAtSplineInputKey(i, ESplineCoordinateSpace::World), FString::Printf(TEXT("input:%d"), i), 0, FColor::White, 10);
			#endif
		
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->RegisterComponent();
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->SetStaticMesh(StaticMesh);
			SplineMesh->SetMaterial(0, BuildingMaterial);
			SplineMesh->RuntimeVirtualTextures.Add(RVTexture);
			SplineMesh->CastShadow = true;
			SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SplineMesh->SetCollisionProfileName(FName(TEXT("splinemeshes")));
			SplineMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
			SplineMesh->AttachToComponent(SplineComponent,FAttachmentTransformRules::KeepWorldTransform);
		
			SplineMesh->SetStartAndEnd(StartLoc, StartTan, EndLoc, EndTan);
		}
	}
	InstancedStaticMeshComponent->AddInstance( FTransform(FRotator::ZeroRotator,SplineComponent->GetLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World), FVector(1,1,1)), true);
	InstancedStaticMeshComponent->AddInstance( FTransform(FRotator::ZeroRotator,SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World), FVector(1,1,1)), true);
}

void ASplineBuilding::ClearSpline()
{
	for (UActorComponent* Component : GetComponents())
		if (Component->IsA(USplineMeshComponent::StaticClass()))
			Component->DestroyComponent();
	SplineComponent->ClearSplinePoints();
}

void ASplineBuilding::ClearSplineMeshes()
{
	for (UActorComponent* Component : GetComponents())
		if (Component->IsA(USplineMeshComponent::StaticClass()))
			Component->DestroyComponent();
	InstancedStaticMeshComponent->ClearInstances();
}

bool ASplineBuilding::SnapSplineToLandscape(float Start, float End, float Interval, USplineComponent* SplineComp, TArray<FVector>& OutArray)
{
	int32 CheckPointCount = FMath::CeilToInt((SplineComp->GetSplineLength() - 1) / 50);
	float interval = (SplineComp->GetSplineLength() - 1) / CheckPointCount;
	TArray<FVector> Locations;
	
	for (int i = 1; i < CheckPointCount - 1; ++i)
	{
		FVector Location = SplineComp->GetWorldLocationAtDistanceAlongSpline(Start + i * interval);

		FHitResult Hit;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(SplineComp->GetOwner());
		CollisionQueryParams.AddIgnoredActors(ignorants);
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), Location + FVector(0, 0, 500), Location + FVector(0, 0, -1000), ETraceTypeQuery::TraceTypeQuery1, true, TArray<AActor*>(), EDrawDebugTrace::ForDuration, Hit, true, FLinearColor::Red, FLinearColor::Green, 20);
		//GetWorld()->LineTraceSingleByChannel(Hit, Location + FVector(0,0,500), Location + FVector(0,0,-1000), ECC_Visibility);
		if (Hit.bBlockingHit)
		{
			int32 Diff = FMath::Abs(Hit.Location.Z - Location.Z);
			if (Diff < 100)
			{
				if (Diff > 5)
				{
					DrawDebugDirectionalArrow(GetWorld(), Location, FVector(Location.X, Location.Y, Hit.Location.Z), 10, FColor::Cyan, false, 10);
					Locations.Add(Hit.Location);
				}
			} else
			{
				OutArray = Locations;
				return false;
			}	
		}
	}
	OutArray = Locations;
	return true;
}

TArray<FVector> ASplineBuilding::FindPathBetweenPoints(const FVector& StartCoord, const FVector& EndCoord, TArray<AActor*> AttachedActors)
{
	float RemainingMovement = 30;
	//UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	//FPathFindingQuery PathFindingQuery(this, *NavSystem->MainNavData, StartCoord, EndCoord);
	//PathFindingQuery.QueryFilter = UNavigationQueryFilter::GetQueryFilter(*NavSystem->MainNavData, QueryFilter);
	//FPathFindingResult Path = NavSystem->FindPathSync(NavAgent, PathFindingQuery);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActors(AttachedActors);
	FNavLocation OutLocation;
	FHitResult LandHit;
	float GroundZ;
	GetWorld()->LineTraceSingleByChannel(LandHit, SplineComponent->GetWorldLocationAtSplinePoint(0) + FVector(0, 0, 1000), SplineComponent->GetWorldLocationAtSplinePoint(0) - FVector(0, 0, 2000), ECC_Visibility, Params);
	if (LandHit.bBlockingHit)
		GroundZ = LandHit.Location.Z;

	FVector End(0.f, 0.f, 0.f);
	TArray<FVector> OldPoints;
	TArray<float> Distances;
	TArray<FVector> Locats;
	
	Locats.Add(StartCoord);
	Locats.Add(EndCoord);

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
	//NavSystem->ProjectPointToNavigation(SplineComponent->GetWorldLocationAtSplinePoint(0), FirstLocation, FVector::One() * 100);
	GetWorld()->LineTraceSingleByChannel(LandHit, SplineComponent->GetWorldLocationAtSplinePoint(0) + FVector(0, 0, 1000), SplineComponent->GetWorldLocationAtSplinePoint(0) - FVector(0, 0, 2000), ECC_Visibility, Params);
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
		if (Step > PrecisionFactor || FMath::Abs(NextZ - StartZ) > PrecisionFactor)
			Step = PrecisionFactor;
		DrawDebugPoint(GetWorld(), SplineComponent->GetWorldLocationAtSplinePoint(i), 30.0f, FColor::Red, false, 0.f);
		Step = FMath::Clamp(Step, float(PrecisionFactor), 250.f);
		for (int j = StartingDist; (j + Step < NextDist) && (j < TotalLength); j += Step)
		{
			bool Push = false;
			FVector CurrentPoint = SplineComponent->GetWorldLocationAtDistanceAlongSpline(j);;
			//NavSystem->ProjectPointToNavigation(CurrentPoint, OutLocation, FVector::One() * 100, &NavAgent, PathFindingQuery.QueryFilter);
			//TOptional<float> CurrentHeight = LandscapeProxy->GetHeightAtLocation(CurrentPoint, EHeightfieldSource::Complex);
			
			GetWorld()->LineTraceSingleByChannel(LandHit, CurrentPoint + FVector(0, 0, 1000), CurrentPoint - FVector(0, 0, 2000), ECC_Visibility, Params);
			FVector PointOnNav;
			if (LandHit.bBlockingHit)
				PointOnNav = LandHit.Location;

			FVector NextPoint = SplineComponent->GetWorldLocationAtDistanceAlongSpline(j + Step);
			GetWorld()->LineTraceSingleByChannel(LandHit, NextPoint + FVector(0, 0, 1000), NextPoint - FVector(0, 0, 2000), ECC_Visibility, Params);
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
				if (j > StartingDist && j + Step <= NextDist)
				{
					FVector Previous = SplineComponent->GetLocationAtDistanceAlongSpline(j - Step, ESplineCoordinateSpace::World);
					FVector Current = SplineComponent->GetLocationAtDistanceAlongSpline(j, ESplineCoordinateSpace::World);
					FVector Next = SplineComponent->GetLocationAtDistanceAlongSpline(j + Step, ESplineCoordinateSpace::Local);
					FHitResult first, second, last;
					GetWorld()->LineTraceSingleByChannel(first, Previous + FVector(0, 0, 1000), Previous - FVector(0, 0, 2000), ECC_Visibility);
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
						DrawDebugLine(GetWorld(), PointOnNav, PointOnNav + SecondDiff * FVector::Dist(PointOnNav, NextPointOnNav), Color, false, 0, 3, 10);
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
	return NewPoints;
}


