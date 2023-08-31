// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuildingManager.h"

#include "RTS_Builder/GameManager.h"
#include "RTS_Builder/BuildingSystem/BuilderFactory.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderWall.h"

void UBuildingManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("BuildingManager Has Been Started."))
}

void UBuildingManager::AddBuilding(ABuildingBase* Building)
{
	Buildings.Add(Building);
}

void UBuildingManager::RemoveBuilding(ABuildingBase* Building)
{
	Buildings.Remove(Building);
}

void UBuildingManager::RenderBuildings()
{
	for (ABuildingBase* Building : Buildings)
	{
		Building->OnRender();
	}
}

void UBuildingManager::CreateBuilder(FBuildingData& Data)
{
	UBuilderFactory* BuilderFactory = NewObject<UBuilderFactory>(this);
	Builder = BuilderFactory->CreateBuilder(Data);
}
