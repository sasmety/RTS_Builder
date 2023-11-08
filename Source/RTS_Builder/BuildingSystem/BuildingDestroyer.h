// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Public/BuilderBase.h"
#include "UObject/Object.h"
#include "BuildingDestroyer.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API UBuildingDestroyer : public UBuilderBase
{
	GENERATED_BODY()
public:
	void LeftPressed() override;
	virtual void Init(UGameManager* Manager, const FBuildingData& Data) override;
};
