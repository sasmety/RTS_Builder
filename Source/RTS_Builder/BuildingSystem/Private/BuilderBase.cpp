// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuilderBase.h"
#include "RTS_Builder/RTSController.h"
#include "RTS_Builder/BuildingSystem/BuildingDestroyer.h"


UBuilderBase::UBuilderBase()
{
}

UBuilderBase::~UBuilderBase()
{
}

void UBuilderBase::OnConstruct(ARTSController Controller, BuildingType::Type Building)
{

}

void UBuilderBase::CreateBuilding()
{
	
}

void UBuilderBase::Destroy()
{
	GameManager->RemoveInputObject(this);
	GameManager->BuildingManager->Builder = nullptr;
}

void UBuilderBase::Init(UGameManager* Manager, const FBuildingData& Data)
{
	GameManager = Manager;
	BuildingData = Data;
	GEngine->AddOnScreenDebugMessage(46, 10, FColor::Purple, FString::Printf(TEXT("Created Builder: %s"), *GetName()));
}

void UBuilderBase::Build()
{
}

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
	if (Key.GetDisplayName().ToString().Equals(TEXT("Space")))
		Destroy();
	else if (Key.GetDisplayName().ToString().Equals(TEXT("X")))
	{
		GameManager->BuildingManager->Builder = NewObject<UBuildingDestroyer>();
		GEngine->AddOnScreenDebugMessage(10, 10, FColor::Red, TEXT("DESTRUCTION MODE ACTIVATED"));
	}
	

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
