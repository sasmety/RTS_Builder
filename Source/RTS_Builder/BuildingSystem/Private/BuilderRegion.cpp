// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Builder/BuildingSystem/Public/BuilderRegion.h"
#include "Kismet/GameplayStatics.h"
#include "RTS_Builder/GameManager.h"
#include "RTS_Builder/RTSController.h"
#include "RTS_Builder/BuildingSystem/Public/BuildingResizable.h"
#include "VT/RuntimeVirtualTexture.h"


UBuilderRegion::UBuilderRegion()
{
	
}

void UBuilderRegion::Init(UGameManager* Manager, const FBuildingData& Data)
{
	Super::Init(Manager, Data);
	BuildingData = Data;
	GameManager->AddInputObject(this);
	CreateBuilding();
	GEngine->AddOnScreenDebugMessage(47, 10, FColor::Yellow, FString::Printf(TEXT("Created Builder: Region")));
}


void UBuilderRegion::CreateBuilding()
{
	if (Hit.bBlockingHit && GameManager)
	{
		FActorSpawnParameters ActorSpawnParameters;
		CurrentBuilding = GameManager->GetWorld()->SpawnActor<ABuildingResizable>(BuildingData.BuildingClass, Hit.Location, FRotator::ZeroRotator, ActorSpawnParameters);
	}
}

void UBuilderRegion::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (GameManager && GameManager->GetController())
	{
		bool bHit = GameManager->GetController()->CursorHit(ECC_GameTraceChannel1, TArray<AActor*>(), Hit, EDrawDebugTrace::None);

		if (CurrentBuilding)
		{
			if (bHit)
				CurrentBuilding->Update(DeltaSeconds, Hit);
		}
	}
}

void UBuilderRegion::LeftPressed()
{
	Super::LeftPressed();
	if (CurrentBuilding == nullptr)
		CreateBuilding();

	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, TEXT("RegionClick"));
	
	if (CurrentBuilding && GameManager)
	{
		CurrentBuilding->AddSplinePoint(Hit.Location);
	}
}

void UBuilderRegion::LeftHold()
{
	Super::LeftHold();
}

void UBuilderRegion::LeftReleased()
{
	Super::LeftReleased();
}

void UBuilderRegion::Destroy()
{
	Super::Destroy();
	CurrentBuilding->Destroy();
}
