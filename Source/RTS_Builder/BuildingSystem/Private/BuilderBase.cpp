// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuilderBase.h"
#include "RTS_Builder/RTSController.h"

UBuilderBase::UBuilderBase()
{
		
}

UBuilderBase::~UBuilderBase()
{
}

void UBuilderBase::OnConstruct(ARTSController Controller, BuildingType::Type Building)
{

}

void UBuilderBase::ConstructBuilding()
{
	
}

void UBuilderBase::ExitBuilder()
{
	GameManager->RemoveInputObject(this);
	GameManager->BuildingManager->Builder = nullptr;
}

void UBuilderBase::Init(UGameManager* Manager, FBuildingData& Data)
{
	GameManager = Manager;
	GameManager->AddInputObject(this);
}

void UBuilderBase::Ticker(){}

void UBuilderBase::Tick(float DeltaTime)
{
	GEngine->AddOnScreenDebugMessage(22, 10, FColor::Cyan, FString::Printf(TEXT("Ticking Builder.")));
}

bool UBuilderBase::IsTickable() const
{
	return true;
}

void UBuilderBase::LeftHold()
{
}

void UBuilderBase::LeftPressed()
{
}

void UBuilderBase::LeftReleased()
{
}

void UBuilderBase::KeyPressed(FKey Key)
{
	if (Key.GetDisplayName().ToString().Equals(TEXT("Escape")))
		ExitBuilder();
}

bool UBuilderBase::IsTickableInEditor() const
{
	return true;
}

bool UBuilderBase::IsTickableWhenPaused() const
{
	return false;
}

TStatId UBuilderBase::GetStatId() const
{
	return TStatId();
}

void ConstructBuilding(BuildingType::Type building)
{
	
}
