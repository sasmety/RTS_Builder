// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderBase.h"
#include "BuilderWall.generated.h"

class ASplineBuilding;
class ABuildingSpline;
/**
 * 
 */
UCLASS()
class RTS_BUILDER_API UBuilderWall : public UBuilderBase
{
	GENERATED_BODY()
public:
	UBuilderWall();

	virtual void Init(UGameManager* Manager, FBuildingData& Data) override;
	virtual void ConstructBuilding() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void LeftPressed() override;
	virtual void LeftHold() override;
	virtual void LeftReleased() override;
	UPROPERTY()
	ASplineBuilding* CurrentBuilding;
	FHitResult Hit;
};
