// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuilderWall.h"

#include "Kismet/GameplayStatics.h"
#include "RTS_Builder/GameManager.h"
#include "RTS_Builder/RTSController.h"
#include "RTS_Builder/BuildingSystem/Public/SplineBuilding.h"
#include "RTS_Builder/BuildingSystem/Public/Wall.h"


UBuilderWall::UBuilderWall()
{
}

void UBuilderWall::Init(UGameManager* Manager, FBuildingData& Data)
{
	Super::Init(Manager, Data);
	BuildingData = Data;
	ConstructBuilding();
}


void UBuilderWall::ConstructBuilding()
{
	if (Hit.bBlockingHit && GameManager)
	{
		FActorSpawnParameters ActorSpawnParameters;
		CurrentBuilding = GameManager->GetWorld()->SpawnActor<AWall>(BuildingData.BuildingClass->StaticClass(), Hit.Location, FRotator::ZeroRotator, ActorSpawnParameters);
		CurrentBuilding->SetActorTickEnabled(true);
	}
}

void UBuilderWall::Tick(float DeltaSeconds)
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

void UBuilderWall::LeftPressed()
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

void UBuilderWall::LeftHold()
{
	Super::LeftHold();
}

void UBuilderWall::LeftReleased()
{
	Super::LeftReleased();
}

