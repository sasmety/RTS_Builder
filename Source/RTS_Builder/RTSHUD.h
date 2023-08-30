// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LivingEntity.h"
#include "MouseInterface.h"
#include "GameFramework/HUD.h"
#include "RTSHUD.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API ARTSHUD : public AHUD, public IMouseInterface
{
	GENERATED_BODY()
public:
	ARTSHUD();
	virtual void DrawHUD() override;
	virtual void BeginPlay() override;
	void AssignActorsInSelection(FVector2d& FirstPoint, FVector2d& LastPoint);
	virtual void LeftPressed() override;
	virtual void LeftHold() override;
	virtual void LeftReleased() override;
	virtual void KeyPressed(FKey Key) override;

	FVector* HitLocation = nullptr;
	UPROPERTY()
	FVector StartLocation;
	UPROPERTY()
	FVector CurrentLocation;
	UPROPERTY()
	bool bIsDrawing = false;
	UPROPERTY()
	TArray<ALivingEntity*> OutActors;
	UPROPERTY()
	class ARTSController* Controller;
};



