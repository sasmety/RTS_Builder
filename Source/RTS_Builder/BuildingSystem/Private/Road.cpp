// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/Road.h"

#include "Components/SplineComponent.h"

bool ARoad::AddSplinePoint(FVector& WorldPoint)
{
	SplineComponent->AddSplineWorldPoint(WorldPoint);
	
	return true;
}

void ARoad::RotatePoint(int32 Index, float Value)
{
	Super::RotatePoint(Index, Value);
}

void ARoad::BeginPlay()
{
	Super::BeginPlay();
}

void ARoad::Update(float DeltaSeconds, FHitResult& Hit)
{
	if (SplineComponent->GetNumberOfSplinePoints() > 0 && Hit.bBlockingHit)
	{
		if (SplineComponent->GetNumberOfSplinePoints() > 1)
			SplineComponent->RemoveSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, true);
		SplineComponent->AddSplineWorldPoint(Hit.Location);
		GenerateSplineMesh(0, SplineComponent->GetNumberOfSplinePoints() - 1, Mesh);
	}
}

int32 ARoad::GetClosestPoint(FVector& WorldPoint, USplineComponent* TargetSpline)
{
	if (TargetSpline->GetNumberOfSplinePoints() < 2) return 0;
	if (TargetSpline == nullptr) return 0;
	float dist = TargetSpline->GetDistanceAlongSplineAtSplineInputKey(TargetSpline->FindInputKeyClosestToWorldLocation(WorldPoint));
	float d1, d2;
	if (TargetSpline->GetSplineLength() / 2 < dist)
	{
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
	} else
	{
		for (int i = 0; i < TargetSpline->GetNumberOfSplinePoints() - 1; ++i)
		{
			d1 = TargetSpline->GetDistanceAlongSplineAtSplinePoint(i);
			d2 = TargetSpline->GetDistanceAlongSplineAtSplinePoint(i - 1);
			if (dist < d2 && dist > d1)
			{
				if (FMath::Abs(dist - d1) > FMath::Abs(dist - d2))
					return i - 1;
				return i;
			}
		}
	}
	
}

TOptional<FVector> ARoad::FindAttachmentPoint(FVector& WorldPoint, TArray<AActor*>& AttachmentActors, FRotator& Rot)
{
	

	return Super::FindAttachmentPoint(WorldPoint, AttachmentActors, Rot);
}
