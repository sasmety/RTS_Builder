// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Builder/BuildingSystem/Public/BuildingManager.h"
#include "Engine/GameInstance.h"
#include "GameManager.generated.h"

/**
 * 
 */

UCLASS()
class RTS_BUILDER_API UGameManager : public UGameInstance
{
	GENERATED_BODY()
private:
	UGameManager();
	virtual void Init() override;
	
	TArray<IMouseInterface*> InputObjects;
	UPROPERTY()
	ARTSController* Controller = nullptr;
public:
	void AddInputObject(UObject* Object);
	void RemoveInputObject(UObject* Object);
	TArray<IMouseInterface*> GetInputObjects();
	ARTSController* GetController()
	{
		return Controller;
	};

	void SetController(ARTSController* RTSController)
	{
		this->Controller = RTSController;
	}
	UPROPERTY()
	UBuildingManager* BuildingManager;
};
