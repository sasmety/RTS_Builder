  // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingBase.h"
#include "BuildingManager.h"
#include "RTS_Builder/MouseInterface.h"
#include "UObject/NoExportTypes.h"
#include "BuilderBase.generated.h"

/**
 * 
 */

  class UGameManager;

  namespace BuildingType
  {
	  enum Type;
  }

class UBuildingManager;
UCLASS()
class RTS_BUILDER_API UBuilderBase : public UObject, public IMouseInterface, public FTickableGameObject
{
	GENERATED_BODY()
public:
	UBuilderBase();
	~UBuilderBase();
	void OnConstruct(ARTSController Controller, BuildingType::Type Building);
	virtual void ConstructBuilding();
	void ExitBuilder();
	virtual void Init(UGameManager* Manager, FBuildingData& Data);
	void Ticker();

	virtual void LeftHold() override;
	virtual void LeftPressed() override;
	virtual void LeftReleased() override;
	virtual void KeyPressed(FKey Key) override;

	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override;
	bool IsTickableWhenPaused() const override;
	TStatId GetStatId() const override;

	UPROPERTY()
	UGameManager* GameManager = nullptr;
	FBuildingData BuildingData;
};
