// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderStatic.h"
#include "RTS_Builder/GameManager.h"
#include "Engine/DataTable.h"
#include "Public/BuildingFixedSize.h"


void UBuilderStatic::Init(UGameManager* Manager, FBuildingData& Data)
{
	Super::Init(Manager, Data);
	BuildingData = Data;
	ConstructBuilding();
}

void UBuilderStatic::ConstructBuilding()
{
	Super::ConstructBuilding();
	if (Hit.bBlockingHit && GameManager)
	{
		FActorSpawnParameters ActorSpawnParameters;
		CurrentBuilding = GameManager->GetWorld()->SpawnActor<ABuildingFixedSize>(BuildingData.BuildingClass->StaticClass(), Hit.Location, FRotator::ZeroRotator, ActorSpawnParameters);
		CurrentBuilding->SetActorTickEnabled(true);
	}
}

void UBuilderStatic::LeftHold()
{
	Super::LeftHold();
}

void UBuilderStatic::LeftPressed()
{
	Super::LeftPressed();
}

void UBuilderStatic::LeftReleased()
{
	Super::LeftReleased();
}

void UBuilderStatic::KeyPressed(FKey Key)
{
	Super::KeyPressed(Key);
}

void UBuilderStatic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool UBuilderStatic::IsTickable() const
{
	return Super::IsTickable();
}

bool UBuilderStatic::IsTickableInEditor() const
{
	return Super::IsTickableInEditor();
}

bool UBuilderStatic::IsTickableWhenPaused() const
{
	return Super::IsTickableWhenPaused();
}

TStatId UBuilderStatic::GetStatId() const
{
	return Super::GetStatId();
}
