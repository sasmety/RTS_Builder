// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingFixedSize.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderBase.h"
#include "UObject/Object.h"
#include "BuilderStatic.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API UBuilderStatic : public UBuilderBase
{
	GENERATED_BODY()

public:
	virtual void CreateBuilding() override;
	virtual void Init(UGameManager* Manager, const FBuildingData& Data) override;
	virtual void LeftHold() override;
	void Build() override;
	void Destroy() override;
	virtual void LeftPressed() override;
	virtual void LeftReleased() override;
	virtual void KeyPressed(FKey Key) override;
	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override;
	bool IsTickableWhenPaused() const override;
	TStatId GetStatId() const override;

	UPROPERTY()
	ABuildingFixedSize* CurrentBuilding = nullptr;
};
