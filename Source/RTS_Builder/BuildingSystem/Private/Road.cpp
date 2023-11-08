// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/Road.h"

#include "EngineUtils.h"
#include "LandscapeSplinesComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/BodySetup.h"
#include "RTS_Builder/GameManager.h"
#include "RTS_Builder/RTSController.h"
#include "RTS_Builder/BuildingSystem/RoadIntersection.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderBase.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderRoad.h"
#include "VT/RuntimeVirtualTexture.h"

TObjectPtr<USplineComponent> CopyComponent;


ARoad::ARoad()
{
	FSoftObjectPath PillarPath(TEXT("/Game/Assets/Buildings/bestroad.bestroad"));
	Mesh = Cast<UStaticMesh>(PillarPath.TryLoad());
	// FSoftObjectPath GhostMat(TEXT("/Game/Megascans/Surfaces/Gravel_Ground_xdusfjy/MM_Road_Inst.MM_Road_Inst"));
	FSoftObjectPath GhostMat(TEXT("/Game/Megascans/Surfaces/Fine_American_Road_sjfnbeaa/MI_Fine_American_Road_sjfnbeaa_2K.MI_Fine_American_Road_sjfnbeaa_2K"));
	GhostMaterial = Cast<UMaterialInterface>(GhostMat.TryLoad());
	FSoftObjectPath RVMat(TEXT("/Game/Assets/Materials/NewRuntimeVirtualTexture.NewRuntimeVirtualTexture"));
	RVTexture = Cast<URuntimeVirtualTexture>(RVMat.TryLoad());
	FBoxSphereBounds Box = Mesh->GetBounds();
	MeshLength = Box.BoxExtent.X * 2;
	// SplineComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// SplineComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Block);
}

ARoad::~ARoad()
{
}

void ARoad::Init()
{
	 CachedMeshes.SetNum(20, true);
	for (int i = 0; i < 20; ++i)
	{
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->RegisterComponent();
		SplineMeshComponent->SetStaticMesh(Mesh);
		SplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SplineMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block);
		SplineMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		SplineMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		SplineMeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		// SplineMeshComponent->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseSimpleAndComplex;
		SplineMeshComponent->RuntimeVirtualTextures.Add(RVTexture);
		SplineMeshComponent->SetMaterial(0, GhostMaterial); 
		SplineMeshComponent->SetGenerateOverlapEvents(true);

		SplineMeshComponent->AttachToComponent(
			SplineComponent,
			FAttachmentTransformRules::KeepWorldTransform
		);

		CachedMeshes[i] = SplineMeshComponent;
	}
}

bool ARoad::AddSplinePoint(FVector& WorldPoint, bool bShouldStore)
{
	SplineComponent->AddSplineWorldPoint(WorldPoint);
	if (SplineComponent->GetNumberOfSplinePoints() == 1)
	{
		SplineComponent->AddSplineWorldPoint(WorldPoint);
	}

	SplineComponent->UpdateSpline();

	if (bShouldStore)
	{
		TArray<TWeakObjectPtr<USplineMeshComponent>> a;
		MeshDistances.GetKeys(a);
		for (int i = 0; i < a.Num(); ++i)
		{
			FString str = FString::FromInt(i) + TEXT(" ") + FString::SanitizeFloat(MeshDistances.FindRef(a[i])) + TEXT(" SplineLen:") + FString::SanitizeFloat(SplineComponent->GetSplineLength());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *str);
			DrawDebugSphere(GetWorld(), SplineComponent->GetWorldLocationAtDistanceAlongSpline(MeshDistances.FindRef(a[i])) + (FVector::UpVector * 30 * SplineComponent->GetNumberOfSplinePoints()), 30, 10, FColor::MakeRandomColor(), false, 20);
		}

		UE_LOG(LogTemp, Warning, TEXT("================"));
		UE_LOG(LogTemp, Warning, TEXT("Length Point: %s"), *SplineComponent->GetWorldLocationAtDistanceAlongSpline(SplineComponent->GetSplineLength()).ToString());
		UE_LOG(LogTemp, Warning, TEXT("Length Point (- 1): %s"), *SplineComponent->GetWorldLocationAtDistanceAlongSpline(SplineComponent->GetSplineLength() - 1).ToString());
		UE_LOG(LogTemp, Warning, TEXT("Length Point (+ 1): %s"), *SplineComponent->GetWorldLocationAtDistanceAlongSpline(SplineComponent->GetSplineLength() + 100).ToString());

		// DrawDebugSphere(GetWorld(), SplineComponent->GetWorldLocationAtDistanceAlongSpline(SplineComponent->GetSplineLength()) + (FVector::UpVector * 30 * SplineComponent->GetNumberOfSplinePoints()), 30, 10, FColor::MakeRandomColor(), false, 20);
	}

	return false;
}

void ARoad::BreakIntersection(ARoadIntersection* Intersection)
{
	Intersection->DestroyConnections(this);
}

void ARoad::RemoveIntersection(ARoadIntersection* Intersection)
{
}

void ARoad::RotatePoint(int32 Index, float Value)
{
	Super::RotatePoint(Index, Value);
}

void ARoad::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

void ARoad::Update(float DeltaSeconds, FHitResult& Hit)
{
	if (SplineComponent->GetNumberOfSplinePoints() > 0 && Hit.bBlockingHit)
	{
		if (SplineComponent->GetNumberOfSplinePoints() > 1)
		{
			FAttachmentData Data;
			FindAttachmentPoint(Hit.Location, Data, TEXT("second"));
			SplineComponent->SetWorldLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, Data.Loc);
			FVector a(50, 100, 50);
			CreateCollisionBoxesAlongSpline(100, a);
		}
		// 	SplineComponent->RemoveSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, true);
		// AddSplinePoint(Hit.Location, false);

		// ClearSplineMeshes();
		GenerateSplineMesh(Mesh, false);
	}
}

void ARoad::SetGhostMaterialColorRGB(const FVector4d& Color)
{
	GhostMaterialInstance = UMaterialInstanceDynamic::Create(GhostMaterial, this);
	GhostMaterialInstance->SetVectorParameterValue(FName(TEXT("Color")), Color);
}

void ARoad::CreateCollisionBoxesAlongSpline(float Interval, FVector& Size){
	if (Interval < 1) return;
	int32 i = FMath::CeilToInt(SplineComponent->GetSplineLength() / Interval);
	float length = SplineComponent->GetSplineLength() / i;


	TArray<FHitResult> Rez;
	// TArray<AActor*> ignored;
	// ignored.Add(this);
	
	TArray<AActor*> roads;
	Attachments.GenerateValueArray(roads);
	roads.Add(Cast<AActor>(this));
	// Attachments.AddUnique(this);
	
	for (int32 j = 0; j < i; ++j)
	{
		FHitResult MiniRez;
		UKismetSystemLibrary::BoxTraceSingle(GetWorld(), SplineComponent->GetWorldLocationAtDistanceAlongSpline(j * length),
			SplineComponent->GetWorldLocationAtDistanceAlongSpline(j * length), Size, SplineComponent->GetWorldDirectionAtDistanceAlongSpline(j * length).Rotation(),
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel4), true, roads, EDrawDebugTrace::None, MiniRez, true, FColor::Red, FColor::Green, 0.1f);
		Rez.Add(MiniRez);
	}
	int32 c = 100;
	for (FHitResult HitResult : Rez)
	{
		if (HitResult.bBlockingHit)
		{
			// GEngine->AddOnScreenDebugMessage(c, 10, FColor::Red, FString::Printf(TEXT("Blocking, %s"), *HitResult.GetActor()->GetName()));
			SetGhostMaterialColorRGB(FVector4d(255, 0, 0, 1));
			return;
		}
		else
		{
		}
		++c;
	}
	SetGhostMaterialColorRGB(FVector4d(0, 255, 0, 1));

	// GEngine->AddOnScreenDebugMessage(c, 10, FColor::Green, TEXT("Free"));

}

void ARoad::UpdateIntersections()
{
	if (StartIntersection)
	{
		StartIntersection.Get()->CalculateSplines();
	}

	if (EndIntersection)
	{
		EndIntersection.Get()->CalculateSplines();
	}
}

void ARoad::DestructBuilding(FHitResult& HitData, float size)
{
	if (HitData.GetActor() == this && SplineComponent->GetNumberOfSplinePoints() > 1)
	{
		if (SplineComponent->GetSplineLength() <= size * 1.5f)
		{
			if (StartIntersection)
				StartIntersection->DisconnectRoad(this);
			if (EndIntersection)
				EndIntersection->DisconnectRoad(this);
			Destroy();
		}
		else
		{
			int32 SegmentNum = FMath::Floor(SplineComponent->GetSplineLength() / size);
			float SegmentSize = SplineComponent->GetSplineLength() / SegmentNum;
			float SelectedDistance = SplineComponent->GetDistanceAlongSplineAtSplineInputKey(SplineComponent->FindInputKeyClosestToWorldLocation(HitData.Location));
			float SelectedSegmentIndex = FMath::Floor(SelectedDistance / SegmentSize);

			if (SelectedSegmentIndex == 0)
			{
				if (StartIntersection)
					StartIntersection->DisconnectRoad(this);
				CutToFromStart(SegmentSize);
			}
			else if (SelectedSegmentIndex == SegmentNum - 1)
			{
				CutFromToEnd(SelectedSegmentIndex * SegmentSize);
				if (EndIntersection)
					EndIntersection->DisconnectRoad(this);
			}
			else
			{
				Split(SelectedSegmentIndex * SegmentSize, SegmentSize);
			}
		}
	}
}

int32 ARoad::GetClosestPoint(FVector& WorldPoint, USplineComponent* TargetSpline)
{
	if (TargetSpline->GetNumberOfSplinePoints() < 2) return -1;
	if (TargetSpline == nullptr) return -1;
	float dist = TargetSpline->GetDistanceAlongSplineAtSplineInputKey(TargetSpline->FindInputKeyClosestToWorldLocation(WorldPoint));
	float d1, d2;
	for (int i = TargetSpline->GetNumberOfSplinePoints() - 1; i > 0; --i)
	{
		d1 = TargetSpline->GetDistanceAlongSplineAtSplinePoint(i);
		d2 = TargetSpline->GetDistanceAlongSplineAtSplinePoint(i - 1);
		if (dist < d1 && dist > d2)
		{
			if (FMath::Abs(dist - d1) > FMath::Abs(dist - d2))
				return i - 1;
			return i;
		}
	}
	return -1;
}

int32 ARoad::GetDeletePoint(FVector& WorldPoint, USplineComponent* TargetSpline)
{
	if (TargetSpline->GetNumberOfSplinePoints() < 2) return -1;
	if (TargetSpline == nullptr) return -1;
	float dist = TargetSpline->GetDistanceAlongSplineAtSplineInputKey(TargetSpline->FindInputKeyClosestToWorldLocation(WorldPoint));
	float d1, d2;
	for (int i = TargetSpline->GetNumberOfSplinePoints() - 1; i > 0; --i)
	{
		d1 = TargetSpline->GetDistanceAlongSplineAtSplinePoint(i);
		d2 = TargetSpline->GetDistanceAlongSplineAtSplinePoint(i - 1);
		if (dist < d1 && dist > d2)
		{
			return i - 1;
		}
	}
	return -1;
}
//MAke connectable interface
bool ARoad::FindAttachmentPoint(FVector& WorldPoint, FAttachmentData& Data, FString type)
{
	FHitResult HitResults;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	TArray<AActor*> ignored;
	ignored.Add(this);
	UKismetSystemLibrary::BoxTraceSingle(GetWorld(), WorldPoint, WorldPoint, FVector(200, 200, 200), FRotator::ZeroRotator, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3), true, ignored, EDrawDebugTrace::ForOneFrame, HitResults, true, FColor::Orange, FColor::Blue, 0);
	Data.Loc = WorldPoint;

	if (!HitResults.bBlockingHit || HitResults.GetActor() == nullptr) return false;

	TObjectPtr<ARoadIntersection> Oth = Cast<ARoadIntersection>(HitResults.GetActor());
	if (Oth)
	{
		if (Oth == StartIntersection)
		{
			return false;
		}
		UE_LOG(LogTemp, Warning, TEXT("Attached To Intersection %s"), *Oth->GetName())
		Data.Intersection = Oth;
		Data.Loc = Oth->AttachPoint;
		Data.Dir = SplineComponent->GetTangentAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
		return true;
	}

	ARoad* Other = Cast<ARoad>(HitResults.GetActor());
	if (Other)
	{
		if (StartIntersection)
		{
			StartIntersection->Roads.Contains(Other);
			return false;
		}
		UE_LOG(LogTemp, Warning, TEXT("Attached To Road %s"), *Other->GetName());
		float dist = Other->SplineComponent->GetDistanceAlongSplineAtSplineInputKey(Other->SplineComponent->FindInputKeyClosestToWorldLocation(HitResults.Location));
		// DrawDebugLine(GetWorld(), HitResults.Location, HitResults.Location + FVector::UpVector * 100, FColor::Cyan, false, 10, 2, 20);
		Data.Road = Other;
		Data.Loc = Other->SplineComponent->GetWorldLocationAtDistanceAlongSpline(dist);
		Data.Dir = Other->SplineComponent->GetWorldTangentAtDistanceAlongSpline(dist);
		if(Attachments.Contains(type))
		{
			AActor** temp = Attachments.Find(type);
			*temp = Other;
		}
		else
			Attachments.Add(type, Other);

		return true;
	}
	else
	{
		Attachments.Remove(type);
	}
	return false;
}

void ARoad::RenderMeshAtSplineDistance(UStaticMesh* StaticMesh, float StartValue, float EndValue, int32 StorageIndex = -1)
{
	float StretchedMeshLength = EndValue - StartValue;
	FVector Start = SplineComponent->GetLocationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::World);
	FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local);
	StartTang = StartTang.GetSafeNormal() * StretchedMeshLength;
	// StartTang = StartTang.GetClampedToMaxSize(StretchedMeshLength);

	FVector End = SplineComponent->GetLocationAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::World);
	FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::Local);
	EndTang = StretchedMeshLength * EndTang.GetSafeNormal();
	// EndTang = EndTang.GetClampedToMaxSize(StretchedMeshLength);

	USplineMeshComponent* NewSplineMeshComponent = NewObject<USplineMeshComponent>(this);
	NewSplineMeshComponent->SetMobility(EComponentMobility::Movable);
	NewSplineMeshComponent->RegisterComponent();
	NewSplineMeshComponent->SetStaticMesh(StaticMesh);
	NewSplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	NewSplineMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Block);
	NewSplineMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	NewSplineMeshComponent->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
	// NewSplineMeshComponent->bDrawMeshCollisionIfSimple = true;
	// NewSplineMeshComponent->bDrawMeshCollisionIfComplex = true;

	NewSplineMeshComponent->AttachToComponent(
		SplineComponent,
		FAttachmentTransformRules::KeepWorldTransform
	);

	NewSplineMeshComponent->SetStartAndEnd(Start + FVector::UpVector, StartTang, End + FVector::UpVector, EndTang);
	if (StorageIndex != -1 && meshes.Num() > StorageIndex)
	{
		MeshDistances.Remove(meshes[StorageIndex]);
		meshes[StorageIndex] = NewSplineMeshComponent;
		MeshDistances.Add(NewSplineMeshComponent, EndValue);
		return;
	}
	meshes.AddUnique(NewSplineMeshComponent);
	MeshDistances.Add(NewSplineMeshComponent, EndValue);
}

void ARoad::GenerateStoredSplineMeshes()
{
	if (meshes.Num() == 0) return;

	RenderMeshAtSplineDistance(Mesh, 0, MeshDistances.FindRef(meshes[0]), 0);
	if (meshes.Num() == 1) return;
	for (int i = 1; i < meshes.Num(); ++i)
	{
		RenderMeshAtSplineDistance(Mesh, MeshDistances.FindRef(meshes[i - 1]), MeshDistances.FindRef(meshes[i]), i);
	}
}

void ARoad::GenerateSplineMesh(UStaticMesh* StaticMesh, bool bUseStoredMeshes)
{
	// if (SplineComponent->GetNumberOfSplinePoints() < 2) return;
	if (GetWorld() == nullptr) return;
	// if (bUseStoredMeshes)
	// {
	// 	GenerateStoredSplineMeshes();
	// 	return;
	// }
	meshes.Empty();
	MeshDistances.Empty();
	
	int32 index = 0;
	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints() - 1; ++i)
	{
		float SegmentLength = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i + 1) - SplineComponent->GetDistanceAlongSplineAtSplinePoint(i);
		int32 NumberOfSegments = FMath::CeilToInt(SegmentLength / MeshLength);
		if (NumberOfSegments == 0)
			NumberOfSegments = 1;
		float StretchedMeshLength = SegmentLength / NumberOfSegments;

		// if (StretchedMeshLength > MeshLength * 1.5f)
		// {
		// 	StretchedMeshLength /= 2;
		// 	NumberOfSegments *= 2;
		// }
		for (int j = 0; j < NumberOfSegments; ++j)
		{
			float StartValue = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) + StretchedMeshLength * j;
			float EndValue = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) + StretchedMeshLength * (j + 1);

			FVector Start = SplineComponent->GetLocationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::World);
			FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local);
			StartTang = StartTang.GetSafeNormal() * StretchedMeshLength;

			FVector End = SplineComponent->GetLocationAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::World);
			FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::Local);
			EndTang = StretchedMeshLength * EndTang.GetSafeNormal();

			if (index >= CachedMeshes.Num()) return;
			USplineMeshComponent* SplineMeshComponent = CachedMeshes[index];
			++index;
			SplineMeshComponent->RuntimeVirtualTextures.Add(RVTexture);
			SplineMeshComponent->SetMaterial(0, GhostMaterial);

			// NewSplineMeshComponent->SetStaticMesh(StaticMesh);
			// NewSplineMeshComponent->SetMaterial(0, GhostMaterialInstance);
			// NewSplineMeshComponent->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseSimpleAndComplex;
			//
			// NewSplineMeshComponent->AttachToComponent(
			// 	SplineComponent,
			// 	FAttachmentTransformRules::KeepWorldTransform
			// );
		
			SplineMeshComponent->SetStartAndEnd(Start + FVector::UpVector, StartTang, End + FVector::UpVector, EndTang);
			// SplineMeshComponent->UpdateMesh();
			// NewSplineMeshComponent->UpdateMesh();
			meshes.AddUnique(SplineMeshComponent);
			MeshDistances.Add(SplineMeshComponent, EndValue);
		}
	}

	for (int i = index; i < CachedMeshes.Num(); ++i)
	{
		CachedMeshes[i]->SetStartAndEnd(FVector::ZeroVector, FVector::ZeroVector, FVector::ZeroVector, FVector::ZeroVector);
	}
}

void ARoad::GenerateSplineMesh(UStaticMesh* StaticMesh, float StartDistance, float EndDistance)
{
	if (GetWorld() == nullptr) return;
	// meshes.Empty();
	// MeshDistances.Empty();

	
		float SegmentLength = EndDistance - StartDistance;
		int32 NumberOfSegments = FMath::CeilToInt(SegmentLength / MeshLength);
		if (NumberOfSegments == 0)
			NumberOfSegments = 1;
		float StretchedMeshLength = SegmentLength / NumberOfSegments;
		
		for (int j = 0; j < NumberOfSegments; ++j)
		{
			float StartValue = StartDistance + StretchedMeshLength * j;
			float EndValue = StartDistance + StretchedMeshLength * (j + 1);

			FVector Start = SplineComponent->GetLocationAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::World);
			FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(StartValue, ESplineCoordinateSpace::Local);
			StartTang = StartTang.GetSafeNormal() * StretchedMeshLength;

			FVector End = SplineComponent->GetLocationAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::World);
			FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(EndValue, ESplineCoordinateSpace::Local);
			EndTang = StretchedMeshLength * EndTang.GetSafeNormal();

			USplineMeshComponent* NewSplineMeshComponent = NewObject<USplineMeshComponent>(this);
			NewSplineMeshComponent->SetMobility(EComponentMobility::Movable);
			NewSplineMeshComponent->RegisterComponent();
			NewSplineMeshComponent->SetStaticMesh(StaticMesh);
			NewSplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			NewSplineMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block);
			NewSplineMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			NewSplineMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			NewSplineMeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
			NewSplineMeshComponent->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseSimpleAndComplex;
			NewSplineMeshComponent->SetMaterial(0, GhostMaterial); 
			NewSplineMeshComponent->SetGenerateOverlapEvents(true);

			NewSplineMeshComponent->AttachToComponent(
				SplineComponent,
				FAttachmentTransformRules::KeepWorldTransform
			);
		
			NewSplineMeshComponent->SetStartAndEnd(Start + FVector::UpVector, StartTang, End + FVector::UpVector, EndTang);
			NewSplineMeshComponent->UpdateMesh();
			// meshes.AddUnique(NewSplineMeshComponent);
			// MeshDistances.Add(NewSplineMeshComponent, EndValue);
		}
}

void ARoad::AddIntersection(ARoadIntersection* Intersection, TArray<TObjectPtr<USplineMeshComponent>> Components)
{
	
}

void ARoad::Tick(float DeltaSeconds)
{
#if WITH_EDITOR
	DrawDebugString(GetWorld(), SplineComponent->GetWorldLocationAtDistanceAlongSpline(SplineComponent->GetSplineLength() / 2) + FVector::UpVector * 100, FString::Printf(TEXT("Road:%s"), *GetName()), 0, FColor::Cyan, 0);
	DrawDebugSphere(GetWorld(), SplineComponent->GetWorldLocationAtDistanceAlongSpline(SplineComponent->GetSplineLength() / 2), 25, 10, FColor::Cyan, false, 0);


	if (StartIntersection)
	{
		DrawDebugString(GetWorld(), SplineComponent->GetWorldLocationAtSplinePoint(0) + FVector::UpVector * 100, FString::Printf(TEXT("[START]Road:%s, Name: %s"), *GetName(), *StartIntersection.GetName()), 0, FColor::Green, 0);
		DrawDebugSphere(GetWorld(), SplineComponent->GetWorldLocationAtSplinePoint(0), 25, 10, FColor::Green, false, 0);
	}

	if (EndIntersection)
	{
		DrawDebugString(GetWorld(), SplineComponent->GetWorldLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1) + FVector::UpVector * 100, FString::Printf(TEXT("[END]Road:%s Name: %s"), *GetName(), *EndIntersection.GetName()), 0, FColor::Red, 0);
		DrawDebugSphere(GetWorld(), SplineComponent->GetWorldLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1), 25, 10, FColor::Red, false, 0);
	}
#endif
}

void ARoad::CreateJunction(USplineComponent* Target, FVector& WorldPoint, float offset, bool bShouldStore)
{
	if (Target == nullptr || !Target->GetOwner()->IsA(ARoad::StaticClass())) return;
	if (!bShouldStore) return;
	ARoad* TargetRoad = Cast<ARoad>(Target->GetOwner());

	float TargetLength = Target->GetSplineLength();
	float dist = Target->GetDistanceAlongSplineAtSplineInputKey(Target->FindInputKeyClosestToWorldLocation(WorldPoint));
	FVector AttachPoint = Target->GetWorldLocationAtDistanceAlongSpline(dist);

	float OtherDist = SplineComponent->GetDistanceAlongSplineAtSplineInputKey(SplineComponent->FindInputKeyClosestToWorldLocation(AttachPoint));
	FIntersectionPoint Point;

	bool bIsEnd = false;
	if (OtherDist < SplineComponent->GetSplineLength() / 2)
	{
		Point = CutToFromStart(offset);
	}
	else
	{
		bIsEnd = true;
		Point = CutFromToEnd(SplineComponent->GetSplineLength() - offset);
	}
	// if (SplineComponent->GetSplineLength() > offset)
	// 	OtherDist = SplineComponent->GetSplineLength() - offset;
	// else
	// 	OtherDist = 0;

	float FirstDist, SecondDist;
	if (dist - offset > 0)
		FirstDist = dist - offset;
	else
		FirstDist = 0;

	if (dist + offset < TargetLength)
		SecondDist = dist + offset;
	else
		SecondDist = TargetLength - 1;

	TArray<ARoad*> rosenrot;
	TArray<FIntersectionPoint> points;

	ARoad* Split = TargetRoad->Split(FirstDist, SecondDist - FirstDist);

	// CutFromToEnd(OtherDist);
	FIntersectionPoint p1, p2;
	p1.Loc = TargetRoad->SplineComponent->GetLocationAtSplinePoint(TargetRoad->SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
	p1.Dir = TargetRoad->SplineComponent->GetDirectionAtSplinePoint(TargetRoad->SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);

	p2.Loc = Split->SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	p2.Dir = Split->SplineComponent->GetDirectionAtSplinePoint(0, ESplineCoordinateSpace::World) * -1;


	// p3.Loc = SplineComponent->GetWorldLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1);
	// p3.Dir = SplineComponent->GetDirectionAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
	// p3 = Point;

	for (FIntersectionPoint Point1 : points)
	{
		DrawDebugSphere(GetWorld(), Point1.Loc, 30, 10, FColor::Magenta, true);
	}

	DrawDebugBox(GetWorld(), AttachPoint, FVector(50, 50, 50), FColor::Turquoise, true);


	rosenrot.Add(TargetRoad);
	rosenrot.Add(Split);
	rosenrot.Add(this);

	points.Add(p1);
	points.Add(p2);
	points.Add(Point);

	ARoadIntersection* junction = GetWorld()->SpawnActor<ARoadIntersection>(ARoadIntersection::StaticClass(), WorldPoint, FRotator::ZeroRotator);
	junction->Init(rosenrot, points, AttachPoint, offset);
	TargetRoad->EndIntersection = junction;
	Split->StartIntersection = junction;
	if (bIsEnd)
	{
		EndIntersection = junction;
	}
	else
	{
		StartIntersection = junction;
	}


	// UGameManager* GameManager = Cast<UGameManager>(UGameplayStatics::GetGameInstance(GetWorld()));
	// if (GameManager->BuildingManager && GameManager->BuildingManager->Builder)
	// 	GameManager->BuildingManager->Builder->CreateBuilding();
}

void ARoad::CreateTwoRoadJunction(ARoad* ConstRoad, ARoad* MovingRoad, FVector WorldLocation, float offset)
{
	if (ConstRoad->SplineComponent->GetSplineLength() < offset) return;
	float ConstDist = ConstRoad->SplineComponent->GetDistanceAlongSplineAtSplineInputKey(ConstRoad->SplineComponent->FindInputKeyClosestToWorldLocation(WorldLocation));
	float MovingDist = MovingRoad->SplineComponent->GetDistanceAlongSplineAtSplineInputKey(MovingRoad->SplineComponent->FindInputKeyClosestToWorldLocation(WorldLocation));

	if (ConstDist < ConstRoad->SplineComponent->GetSplineLength() - offset && ConstDist > offset)
	{
		CreateJunction(ConstRoad->SplineComponent, WorldLocation, offset, true);
		return;
	}
	else
	{
		TArray<ARoad*> Roads;
		Roads.Add(ConstRoad);
		Roads.Add(MovingRoad);
		TArray<FIntersectionPoint> Points;

		float constmax = ConstRoad->SplineComponent->GetSplineLength();
		float movingMax = MovingRoad->SplineComponent->GetSplineLength();
		ARoadIntersection* Intersection = GetWorld()->SpawnActor<ARoadIntersection>(WorldLocation, FRotator::ZeroRotator);

		if (ConstDist > constmax / 2)
		{
			if (ConstRoad->EndIntersection)
			{
				FIntersectionPoint point = GetFinalAttachPoint(ConstDist, MovingDist, movingMax, MovingRoad, ConstRoad, ConstRoad->EndIntersection);
				ConstRoad->EndIntersection->AddConnection(MovingRoad, point, offset);
				Intersection->Destroy();
				return;
			}
			else
			{
				Points.Add(ConstRoad->CutFromToEnd(ConstDist - offset));
				ConstRoad->EndIntersection = Intersection;
			}
		}
		else
		{
			if (ConstRoad->StartIntersection)
			{
				FIntersectionPoint point = GetFinalAttachPoint(ConstDist, MovingDist, movingMax, MovingRoad, ConstRoad, ConstRoad->StartIntersection);
				ConstRoad->StartIntersection->AddConnection(MovingRoad, point, offset);
				Intersection->Destroy();
				return;
			}
			else
			{
				Points.Add(ConstRoad->CutToFromStart(ConstDist + offset));
				ConstRoad->StartIntersection = Intersection;
			}
		}

		FColor Color;
		if (MovingDist > movingMax / 2)
		{
			Points.Add(MovingRoad->CutFromToEnd(movingMax - offset));
			MovingRoad->EndIntersection = Intersection;
			Color = FColor::Green;
		}
		else
		{
			Points.Add(MovingRoad->CutToFromStart(0 + offset));
			MovingRoad->StartIntersection = Intersection;
			Color = FColor::Red;
		}
		DrawDebugBox(GetWorld(), WorldLocation, FVector::One() * 30, Color, false, 10);
		Intersection->Init(Roads, Points, WorldLocation, offset);
	}
}

void ARoad::DrawGhost(FHitResult& HitData, float size)
{
	if (HitData.GetActor() != this || SplineComponent->GetNumberOfSplinePoints() < 2) return;
	
	int32 SegmentNum = FMath::Floor(SplineComponent->GetSplineLength() / size);
	float SegmentSize = SplineComponent->GetSplineLength() / SegmentNum;
	float SelectedDistance = SplineComponent->GetDistanceAlongSplineAtSplineInputKey(SplineComponent->FindInputKeyClosestToWorldLocation(HitData.Location));
	float SelectedSegmentIndex = FMath::Floor(SelectedDistance / SegmentSize);

	GenerateSplineMesh(Mesh, SelectedSegmentIndex * SegmentSize, SelectedSegmentIndex * SegmentSize + size);
}

FIntersectionPoint ARoad::GetFinalAttachPoint(float ConstDist, float MovingDist, float MovingMax, ARoad* MovingRoad, ARoad* ConstRoad, ARoadIntersection* Intersection)
{
	int32 index = 0;
	float dist = ConstDist;
	if (MovingDist > MovingMax / 2)
	{
		index = MovingRoad->SplineComponent->GetNumberOfSplinePoints() - 1;
		dist = MovingRoad->SplineComponent->GetSplineLength();
	}
	FIntersectionPoint point;
	MovingRoad->SplineComponent->SetWorldLocationAtSplinePoint(index, Intersection->AttachPoint);
	point.Loc = Intersection->AttachPoint;
	point.Dir = MovingRoad->SplineComponent->GetWorldTangentAtDistanceAlongSpline(dist);
	return point;
}

FIntersectionPoint ARoad::CutToFromStart(float dist)
{
	// if (SplineComponent->GetSplineLength() <= dist || dist < 0) return;
	FIntersectionPoint Point;
	Point.Loc = SplineComponent->GetWorldLocationAtDistanceAlongSpline(dist);
	Point.Dir = SplineComponent->GetTangentAtDistanceAlongSpline(dist, ESplineCoordinateSpace::Local);
	TArray<FIntersectionPoint> points;
	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); ++i)
	{
		if (SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) > dist)
		{
			FIntersectionPoint p;
			p.Loc = SplineComponent->GetWorldLocationAtSplinePoint(i);
			p.Dir = SplineComponent->GetTangentAtSplinePoint(dist, ESplineCoordinateSpace::Local);
			points.Add(p);
		}
	}

	SplineComponent->ClearSplinePoints();
	SplineComponent->AddSplineWorldPoint(Point.Loc);
	SplineComponent->SetTangentAtSplinePoint(0, Point.Dir, ESplineCoordinateSpace::World);
	for (FIntersectionPoint IntersectionPoint : points)
	{
		SplineComponent->AddSplineWorldPoint(IntersectionPoint.Loc);
		SplineComponent->SetTangentAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, IntersectionPoint.Dir, ESplineCoordinateSpace::Local);
	}

	SplineComponent->UpdateSpline();

	// ClearSplineMeshes();
	GenerateSplineMesh(Mesh, true);
	DrawDebugDirectionalArrow(GetWorld(), Point.Loc, Point.Loc + Point.Dir, 10, FColor::Green, false, 10, 1, 5);
	Point.Dir = Point.Dir.GetSafeNormal() * -1;
	return Point;
}

FIntersectionPoint ARoad::CutFromToEnd(float dist)
{
	// if (SplineComponent->GetSplineLength() <= dist || dist < 0) return;
	FIntersectionPoint Point;
	Point.Loc = SplineComponent->GetWorldLocationAtDistanceAlongSpline(dist);
	Point.Dir = SplineComponent->GetWorldTangentAtDistanceAlongSpline(dist).GetSafeNormal();

	for (int i = SplineComponent->GetNumberOfSplinePoints() - 1; i > 0; --i)
	{
		if (SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) > dist)
			SplineComponent->RemoveSplinePoint(i);
	}
	SplineComponent->AddSplineWorldPoint(Point.Loc);
	SplineComponent->SetTangentAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, Point.Dir, ESplineCoordinateSpace::World, true);
	// ClearSplineMeshes();
	GenerateSplineMesh(Mesh, true);
	DrawDebugDirectionalArrow(GetWorld(), Point.Loc, Point.Loc + Point.Dir, 10, FColor::Yellow, false, 10, 1, 5);
	return Point;
}

ARoad* ARoad::Split(float CutPoint, float CutDistance)
{
	float StartDistance = CutPoint;
	float EndDistance = CutPoint + CutDistance;
	TArray<FIntersectionPoint> firstpoints;
	TArray<FIntersectionPoint> lastpoints;
	FIntersectionPoint e;
	e.Loc = SplineComponent->GetWorldLocationAtDistanceAlongSpline(EndDistance);
	e.Dir = SplineComponent->GetWorldTangentAtDistanceAlongSpline(EndDistance).GetSafeNormal() * CutDistance / 2;
	FIntersectionPoint s;
	s.Loc = SplineComponent->GetWorldLocationAtDistanceAlongSpline(StartDistance);
	s.Dir = SplineComponent->GetWorldTangentAtDistanceAlongSpline(StartDistance).GetSafeNormal() * CutDistance / 2;
	lastpoints.Add(e);
	bool updateEnd = false;
	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); ++i)
	{
		float CurrentDist = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i);
		FIntersectionPoint IntersectionPoint;
		if (CurrentDist < StartDistance)
		{
			IntersectionPoint.Loc = SplineComponent->GetWorldLocationAtSplinePoint(i);
			IntersectionPoint.Dir = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
			firstpoints.Add(IntersectionPoint);
		}
		else if (CurrentDist > EndDistance)
		{
			IntersectionPoint.Loc = SplineComponent->GetWorldLocationAtSplinePoint(i);
			IntersectionPoint.Dir = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
			lastpoints.Add(IntersectionPoint);
		}
		else
		{
			if (SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) == StartDistance)
			{
				DrawDebugString(GetWorld(), s.Loc, FString::Printf(TEXT("StartDist")), 0, FColor::Green, 5, false, 3);
				s.Dir = s.Dir.GetSafeNormal() * (SplineComponent->GetWorldLocationAtSplinePoint(i) - SplineComponent->GetWorldLocationAtSplinePoint(i - 1));
				// SplineComponent->SetTangentAtSplinePoint(i, FVector::Zero(), ESplineCoordinateSpace::World);
			}
			else if (SplineComponent->GetDistanceAlongSplineAtSplinePoint(i) == EndDistance)
			{
				updateEnd = true;
				DrawDebugString(GetWorld(), s.Loc, FString::Printf(TEXT("StartDist")), 0, FColor::Red, 5, false, 3);
				e.Dir = (SplineComponent->GetWorldLocationAtSplinePoint(i + 1) - SplineComponent->GetWorldLocationAtSplinePoint(i)) / 2;
				// SplineComponent->SetTangentAtSplinePoint(i, FVector::Zero(), ESplineCoordinateSpace::World);
			}
		}
	}
	firstpoints.Add(s);
	if (updateEnd)
		lastpoints[0].Dir = (lastpoints[1].Loc - lastpoints[0].Loc) / 2;

	DrawDebugCone(GetWorld(), SplineComponent->GetWorldLocationAtDistanceAlongSpline(StartDistance), FVector::UpVector, 100, 0, 0, 15, FColor::Cyan, false, 10);
	DrawDebugCone(GetWorld(), SplineComponent->GetWorldLocationAtDistanceAlongSpline(EndDistance), FVector::UpVector, 100, 0, 0, 15, FColor::Red, false, 10);

	ARoad* Road = GetWorld()->SpawnActor<ARoad>(GetActorLocation(), GetActorRotation());
	// Road->Init();
	SplineComponent->ClearSplinePoints();
	// ClearSplineMeshes();
	Road->SplineComponent->ClearSplinePoints();
	// Road->ClearSplineMeshes();

	for (FIntersectionPoint pf : firstpoints)
	{
		SplineComponent->AddSplineWorldPoint(pf.Loc);
		SplineComponent->SetTangentAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, pf.Dir, ESplineCoordinateSpace::World, true);
	}

	for (FIntersectionPoint pl : lastpoints)
	{
		Road->SplineComponent->AddSplineWorldPoint(pl.Loc);
		Road->SplineComponent->SetTangentAtSplinePoint(Road->SplineComponent->GetNumberOfSplinePoints() - 1, pl.Dir, ESplineCoordinateSpace::World, true);
	}


	GenerateSplineMesh(Mesh, false);
	Road->GenerateSplineMesh(Mesh, false);
	return Road;
}

TArray<TObjectPtr<USplineMeshComponent>> ARoad::DetectIntersectionMeshes(ARoad* Road, const FVector& StartLoc, const FVector& EndLoc, const FVector Rot)
{
	TArray<FHitResult> OutHits;
	UKismetSystemLibrary::BoxTraceMulti(GetWorld(), StartLoc, EndLoc, FVector(1, 1, 100), Rot.Rotation(), UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3), true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, OutHits, false);


	TArray<TObjectPtr<USplineMeshComponent>> OutComponents;
	int32 index = 0;
	for (FHitResult& OutHit : OutHits)
	{
		if (Road == OutHit.GetActor() && OutHit.GetComponent() && OutHit.GetComponent()->IsA(USplineMeshComponent::StaticClass()))
			OutComponents.Add(Cast<USplineMeshComponent>(OutHit.GetComponent()));

		// DrawDebugString(GetWorld(), OutHit.Location, FString::Printf(TEXT("Hit: %d"), index), 0, FColor::Green, 0);
		++index;
	}
	return OutComponents;
}
