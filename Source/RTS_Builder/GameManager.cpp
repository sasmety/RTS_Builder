// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"

#include "Kismet/GameplayStatics.h"

UGameManager::UGameManager()
{
	
}

void UGameManager::Init()
{
	Super::Init();
	BuildingManager = GetSubsystem<UBuildingManager>();
}

void UGameManager::AddInputObject(UObject* Object)
{
	IMouseInterface* InterfaceObject = Cast<IMouseInterface>(Object);
	if (InterfaceObject)
		InputObjects.AddUnique(InterfaceObject);
}

void UGameManager::RemoveInputObject(UObject* Object)
{
	IMouseInterface* InterfaceObject = Cast<IMouseInterface>(Object);
	if (InterfaceObject)
		InputObjects.Remove(InterfaceObject);
}

TArray<IMouseInterface*> UGameManager::GetInputObjects()
{
	return InputObjects;
}


