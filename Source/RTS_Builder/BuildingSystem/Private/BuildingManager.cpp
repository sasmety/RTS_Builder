// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuildingManager.h"

#include "RTS_Builder/GameManager.h"
#include "RTS_Builder/BuildingSystem/BuilderFactory.h"
#include "RTS_Builder/BuildingSystem/BuildingDestroyer.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderRegion.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderRoad.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderStatic.h"
#include "RTS_Builder/BuildingSystem/Public/BuilderWall.h"
#include "RTS_Builder/BuildingSystem/Public/BuildingFixedSize.h"
#include "RTS_Builder/BuildingSystem/Public/BuildingResizable.h"
#include "RTS_Builder/BuildingSystem/Public/Road.h"

void UBuildingManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ItemDataTable.Add(TEXT("House"), FBuildingData(FName(TEXT("House")), ABuildingFixedSize::StaticClass(), UBuilderStatic::StaticClass(), FText::FromName(FName(TEXT("A building for people to live safe."))), 250, 100));
	ItemDataTable.Add(TEXT("Wall"),FBuildingData(FName(TEXT("Wall")), AWall::StaticClass(), UBuilderWall::StaticClass(), FText::FromName(FName(TEXT("A building for make town safe."))), 250, 100));
	ItemDataTable.Add(TEXT("Road"),FBuildingData(FName(TEXT("Road")), ARoad::StaticClass(), UBuilderRoad::StaticClass(), FText::FromName(FName(TEXT("A building for people to run faster."))), 250, 100));
	ItemDataTable.Add(TEXT("Farm"), FBuildingData(FName(TEXT("Farm")), ABuildingResizable::StaticClass(), UBuilderRegion::StaticClass(), FText::FromName(FName(TEXT("A building for plants to grow."))), 250, 100));
	ItemDataTable.Add(TEXT("Destroyer"), FBuildingData(FName(TEXT("Destroyer")), ARoad::StaticClass(), UBuildingDestroyer::StaticClass(), FText::FromName(FName(TEXT("A building for plants to grow."))), 250, 100));
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

void UBuildingManager::CreateBuilder(const FBuildingData& Data)
{
	UBuilderFactory* BuilderFactory = NewObject<UBuilderFactory>(this);
	if (Builder)
		Builder->Destroy();
	Builder = BuilderFactory->CreateBuilder(Data);
	Builder->Init(Cast<UGameManager>(GetGameInstance()), Data);
}


