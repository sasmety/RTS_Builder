// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/GameplayStatics.h"
#include "RTS_Builder/GameManager.h"
#include "RTS_Builder/RTSController.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderRoad.h"
#include "RTS_Builder/BuildingSystem/Public/Road.h"

UBuilderRoad::UBuilderRoad()
{
}

void UBuilderRoad::Init(UGameManager* Manager, FBuildingData& Data)
{
	Super::Init(Manager, Data);
	BuildingData = Data;
	ConstructBuilding();
}

void UBuilderRoad::ConstructBuilding()
{
	if (Hit.bBlockingHit && GameManager)
	{
		FActorSpawnParameters ActorSpawnParameters;
		CurrentBuilding = GameManager->GetWorld()->SpawnActor<ARoad>(BuildingData.BuildingClass->StaticClass(), Hit.Location, FRotator::ZeroRotator, ActorSpawnParameters);
		CurrentBuilding->SetActorTickEnabled(true);
	}
}

void UBuilderRoad::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (GameManager && GameManager->GetController())
	{
		bool bHit = GameManager->GetController()->CursorHit(ECC_GameTraceChannel1, TArray<AActor*>(), Hit);

		if (CurrentBuilding)
		{
			if (bHit)
				CurrentBuilding->Update(DeltaSeconds, Hit);
		}
	}
}

void UBuilderRoad::LeftPressed()
{
	Super::LeftPressed();
	if (CurrentBuilding == nullptr)
		ConstructBuilding();

	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, TEXT("CLICKED"));
	
	if (CurrentBuilding && GameManager)
	{
		bool isCompleted = CurrentBuilding->AddSplinePoint(Hit.Location);
		if (isCompleted)
		{
			GameManager->BuildingManager->AddBuilding(CurrentBuilding);
			CurrentBuilding = nullptr;
		}
	}
}

void UBuilderRoad::LeftHold()
{
	Super::LeftHold();
}

void UBuilderRoad::LeftReleased()
{
	Super::LeftReleased();
}