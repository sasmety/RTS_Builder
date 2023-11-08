// Fill out your copyright notice in the Description page of Project Settings.
#include "RTS_Builder/BuildingSystem/Public/BuilderRoad.h"

#include "AsyncTreeDifferences.h"
#include "Interfaces/IPluginManager.h"
#include "RTS_Builder/BuildingSystem/Public/Road.h"
#include "Kismet/GameplayStatics.h"
#include "RTS_Builder/GameManager.h"
#include "RTS_Builder/RTSController.h"
#include "RTS_Builder/BuildingSystem/BuildingDestroyer.h"
#include "Slate/Private/Framework/Docking/SDockingSplitter.h"

UBuilderRoad::UBuilderRoad()
{
}

void UBuilderRoad::Init(UGameManager* Manager, const FBuildingData& Data)
{
	Super::Init(Manager, Data);
	BuildingData = Data;
	GameManager->AddInputObject(this);
	CreateBuilding();
	GEngine->AddOnScreenDebugMessage(47, 10, FColor::Yellow, FString::Printf(TEXT("Created Builder: Road")));
}

void UBuilderRoad::Destroy()
{
	GameManager->RemoveInputObject(this);
	if (CurrentBuilding)
	{
		CurrentBuilding->Destroy();
	}
	ConnectionData.Empty();
	GameManager->BuildingManager->Builder = nullptr;
}

void UBuilderRoad::CreateBuilding()
{
	if (Hit.bBlockingHit && GameManager)
	{
		ConnectionData.Empty();
		CurrentBuilding = GameManager->GetWorld()->SpawnActor<ARoad>(BuildingData.BuildingClass, Hit.Location, FRotator::ZeroRotator);
		// CurrentBuilding->Init();
	}
}

void UBuilderRoad::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// if (CurrentBuilding)
	// 	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Magenta, FString::Printf(TEXT("Current Road: %s"), *CurrentBuilding->GetName()));
	if (GameManager && GameManager->GetController())
	{
		bool bHit = GameManager->GetController()->CursorHit(ECC_GameTraceChannel1, TArray<AActor*>(), Hit, EDrawDebugTrace::None);

		if (CurrentBuilding)
		{
			if (bHit)
				CurrentBuilding->Update(DeltaSeconds, Hit);
		}
		// Closer();
	}
}

void UBuilderRoad::LeftPressed()
{
	if (!Hit.bBlockingHit) return;
	Super::LeftPressed();
	if (CurrentBuilding == nullptr)
		CreateBuilding();
	switch (Mode)
	{
	case EBuildingMode::Free:
		{
			// CurrentBuilding->ClearSpline();
			FVector Loc = Hit.Location;
			FAttachmentData Data;
			CurrentBuilding->FindAttachmentPoint(Loc, Data, TEXT("first"));
			CurrentBuilding->AddSplinePoint(Loc, true);
			AddConnectionData(Data);
			float len = CurrentBuilding->SplineComponent->GetSplineLength();
			Mode = EBuildingMode::Placed;
			break;
		}
	case EBuildingMode::Placed:
		{
			if (CurrentBuilding->GetSplinePointsNum() >= 2)
			{
				// FVector loc = CurrentBuilding->SplineComponent->GetWorldLocationAtDistanceAlongSpline(CurrentBuilding->SplineComponent->GetSplineLength() - 100);
				FVector loc = Hit.Location;
				FAttachmentData Data;
				CurrentBuilding->FindAttachmentPoint(loc, Data, TEXT("Second"));
				AddConnectionData(Data);
				AssignToConnections();

				CreateBuilding();
				CurrentBuilding->FindAttachmentPoint(loc, Data, "First");
				CurrentBuilding->AddSplinePoint(loc, true);
				AddConnectionData(Data);
			}
		}
		break;
	}
}

void UBuilderRoad::Closer()
{
	if (Hit.bBlockingHit)
	{
		TArray<AActor*> Roads;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoad::StaticClass(), Roads);

		for (AActor* Actor : Roads)
		{
			ARoad* Road = Cast<ARoad>(Actor);
			if (!Road) continue;
			// FVector loc = Road->SplineComponent->FindLocationClosestToWorldLocation(Hit.Location, ESplineCoordinateSpace::World);
			// DrawDebugPoint(GetWorld(), Road->SplineComponent->GetLocationAtSplineInputKey(Road->SplineComponent->FindInputKeyClosestToWorldLocation(loc), ESplineCoordinateSpace::World), 20, FColor::Black, false, 0, 2);
			// DrawDebugString(GetWorld(), Road->SplineComponent->GetLocationAtSplineInputKey(Road->SplineComponent->FindInputKeyClosestToWorldLocation(loc), ESplineCoordinateSpace::World), FString::Printf(TEXT("Inputkey: %f"), Road->SplineComponent->FindInputKeyClosestToWorldLocation(loc)), 0, FColor::White, 0, 0, 3);
		}
	}
}

void UBuilderRoad::AddConnectionData(const FAttachmentData& Data)
{
	if (Data.Intersection || Data.Road)
	{
		ConnectionData.Add(Data);
	}
}

bool UBuilderRoad::AssignToConnections()
{
	bool Cut = false;
	for (FAttachmentData& Data : ConnectionData)
	{
		if (Data.Intersection)
		{
			FIntersectionPoint point;
			point.Loc = Data.Loc;
			point.Dir = Data.Dir;
			Data.Intersection->AddConnection(CurrentBuilding, point, RoadCutSize);
		}
		else if (Data.Road)
		{
			CurrentBuilding->CreateTwoRoadJunction(Data.Road, CurrentBuilding, Data.Loc, RoadCutSize);
		}

		float dist = CurrentBuilding->SplineComponent->GetDistanceAlongSplineAtSplineInputKey(CurrentBuilding->SplineComponent->FindInputKeyClosestToWorldLocation(Data.Loc));
		if (dist > CurrentBuilding->SplineComponent->GetSplineLength() / 2)
		{
			Cut = true;
		}
	}
	return Cut;
}

void UBuilderRoad::LeftHold()
{
	Super::LeftHold();
}

void UBuilderRoad::LeftReleased()
{
	Super::LeftReleased();
}

void UBuilderRoad::ExitBuilder()
{
	UE_LOG(LogTemp, Warning, TEXT("Exit Builder"));
}

void UBuilderRoad::RightPressed()
{
	if (!CurrentBuilding)
	{
		ConnectionData.Empty();
		if (Hit.bBlockingHit && Hit.GetActor()->IsA(ARoad::StaticClass()))
		{
			Cast<ARoad>(Hit.GetActor())->DrawGhost(Hit, 1000);
		}
		return;
	}

	switch (Mode)
	{
	case EBuildingMode::Free:
		Destroy();
		break;
	case EBuildingMode::Placed:
		CurrentBuilding->Destroy();
		CurrentBuilding = nullptr;
		ConnectionData.Empty();
		Mode = EBuildingMode::Free;
		break;
	default:
		break;
	}
}

void UBuilderRoad::KeyPressed(FKey Key)
{
	UE_LOG(LogTemp, Warning, TEXT("Pressed: %s"), *Key.GetDisplayName().ToString());
	if (Key.GetDisplayName().ToString().Equals(TEXT("Escape")))
	{
		if (CurrentBuilding && CurrentBuilding->CurrentSection)
			CurrentBuilding->BreakIntersection(CurrentBuilding->CurrentSection);
	}
	else if (Key.GetDisplayName().ToString().ToLower().Equals(TEXT("x")))
	{
		GameManager->BuildingManager->Builder = NewObject<UBuildingDestroyer>();
		// GameManager->BuildingManager->Builder->Init(GameManager, BuildingData);
		GEngine->AddOnScreenDebugMessage(10, 10, FColor::Red, TEXT("DESTRUCTION MODE ACTIVATED"));
	}
	else if (Key.GetDisplayName().ToString().ToLower().Equals(TEXT("g")))
		Destroy();
	else if (Key.GetDisplayName().ToString().Equals(TEXT("Right Mouse Button")))
	{
		RightPressed();
	}
}
