// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Builder/BuildingSystem/Public/SplineBuilding.h"
#include "UObject/Object.h"
#include "Wall.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API AWall : public ASplineBuilding
{
	GENERATED_BODY()
public:
	bool AddSplinePoint(FVector& WorldPoint) override;
	void RotatePoint(int32 Index, float Value) override;
	void BeginPlay() override;
	void Update(float DeltaSeconds, FHitResult& Hit) override;
};
