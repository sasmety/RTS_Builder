// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Builder/BuildingSystem/Public/BuilderStatic.h"

#include "RTS_Builder/BuildingSystem/Public/BuildingFixedSize.h"
#include "RTS_Builder/GameManager.h"
#include "Engine/DataTable.h"

#include "RTS_Builder/RTSController.h"


void UBuilderStatic::Init(UGameManager* Manager, const FBuildingData& Data)
{
	Super::Init(Manager, Data);
	BuildingData = Data;
	CreateBuilding();
	GEngine->AddOnScreenDebugMessage(47, 10, FColor::Yellow, FString::Printf(TEXT("Created Builder: Static")));

}

void UBuilderStatic::CreateBuilding()
{
	Super::CreateBuilding();
	if (GameManager)
	{
		FActorSpawnParameters ActorSpawnParameters;
		CurrentBuilding = GameManager->GetWorld()->SpawnActor<ABuildingFixedSize>(BuildingData.BuildingClass);
	}
}

void UBuilderStatic::LeftHold()
{
	Super::LeftHold();
}

void UBuilderStatic::Build()
{
	CurrentBuilding->SetDefaultMaterial();
	CreateBuilding();
	//CurrentBuilding->MeshComponent->SetMaterial(0, CurrentBuilding->MaterialGreen);
}

void UBuilderStatic::Destroy()
{
	Super::Destroy();
	CurrentBuilding->Destroy();
}

void UBuilderStatic::LeftPressed()
{
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, TEXT("StaticClick"));

	if (CurrentBuilding && CurrentBuilding->CanBeBuilt())
	{
		Build();
		CreateBuilding();
	}
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
	if (CurrentBuilding)
	{
		GameManager->GetController()->CursorHit(ECC_GameTraceChannel1, TArray<AActor*>(), Hit, EDrawDebugTrace::None);
		if (Hit.bBlockingHit)
		{
			CurrentBuilding->SetActorLocation(Hit.Location);
		}
	}
	
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
