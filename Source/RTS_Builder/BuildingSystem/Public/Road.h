// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SplineBuilding.h"
#include "UObject/Object.h"
#include "Road.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API ARoad : public ASplineBuilding
{
	GENERATED_BODY()
public:
	bool AddSplinePoint(FVector& WorldPoint) override;
	void RotatePoint(int32 Index, float Value) override;
	void BeginPlay() override;
	void Update(float DeltaSeconds, FHitResult& Hit) override;
	virtual int32 GetClosestPoint(FVector& WorldPoint, USplineComponent* TargetSpline) override;
	virtual TOptional<FVector> FindAttachmentPoint(FVector& WorldPoint, TArray<AActor*>& AttachmentActors, FRotator& Rot) override;
};
