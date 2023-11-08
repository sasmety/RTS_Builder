// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuilderBase.h"
#include "BuildingResizable.h"
#include "BuilderRegion.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API UBuilderRegion : public UBuilderBase
{
	GENERATED_BODY()
public:
	UBuilderRegion();
	virtual void Init(UGameManager* Manager, const FBuildingData& Data) override;
	virtual void CreateBuilding() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void LeftPressed() override;
	virtual void LeftHold() override;
	virtual void LeftReleased() override;
	virtual void Destroy() override;
	UPROPERTY()
	ABuildingResizable* CurrentBuilding;
};
