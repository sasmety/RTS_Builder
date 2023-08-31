// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuilderBase.h"
#include "Road.h"
#include "UObject/Object.h"
#include "BuilderRoad.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API UBuilderRoad : public UBuilderBase
{
	GENERATED_BODY()
public:
	UBuilderRoad();
	virtual void Init(UGameManager* Manager, FBuildingData& Data) override;
	virtual void ConstructBuilding() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void LeftPressed() override;
	virtual void LeftHold() override;
	virtual void LeftReleased() override;
	UPROPERTY()
	ARoad* CurrentBuilding;
	
};
