// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingDestroyer.h"

#include "../GameManager.h"
#include "../RTSController.h"
#include "Kismet/GameplayStatics.h"

void UBuildingDestroyer::Init(UGameManager* Manager, const FBuildingData& Data)
{
	Super::Init(Manager, Data);
	GameManager->AddInputObject(this);
	this->GameManager = Manager;
}


void UBuildingDestroyer::LeftPressed()
{
	if (GameManager)
	{
		GameManager->GetController()->CursorHit(ECC_Visibility, TArray<AActor*>(), Hit, EDrawDebugTrace::ForDuration);
		if (Hit.bBlockingHit && Hit.GetActor() && Hit.GetActor()->IsA(ABuildingBase::StaticClass()))
			Cast<ABuildingBase>(Hit.GetActor())->DestructBuilding(Hit, 1200);
	}
}
