// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSHUD.h"

#include "RTSController.h"
#include "Kismet/GameplayStatics.h"
#define FAST_PRINT(x, y, z) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("x:%f, y:%f, z:%f"), x,y,z));}


ARTSHUD::ARTSHUD(){
}

void ARTSHUD::BeginPlay()
{
	Super::BeginPlay();
	Controller = Cast<ARTSController>(GetOwningPlayerController());
	HitLocation = &(Controller->CursorHitLocation);
}


void ARTSHUD::DrawHUD()
{
	Super::DrawHUD();
	if (bIsDrawing)
	{
		FVector2d Start;
		FVector2d End;
		
		UGameplayStatics::ProjectWorldToScreen(Controller, StartLocation, Start, false);
		UGameplayStatics::ProjectWorldToScreen(Controller, CurrentLocation, End, false);
		
		const float LocationX = Start.X;
		const float LocationY = Start.Y;
		const float Width = End.X - Start.X;
		const float Height = End.Y - Start.Y;
		
		DrawRect(FColor(0,150,30,100), LocationX, LocationY, Width, Height);
		
		AssignActorsInSelection(Start, End);
	}
}


void ARTSHUD::LeftPressed()
{
	bIsDrawing = true;
	StartLocation = *HitLocation;
	CurrentLocation = *HitLocation;
}

void ARTSHUD::LeftHold()
{
	CurrentLocation = *HitLocation;
}

void ARTSHUD::LeftReleased()
{
	bIsDrawing = false;
}

void ARTSHUD::KeyPressed(FKey Key)
{
}

void ARTSHUD::AssignActorsInSelection(FVector2d& FirstPoint, FVector2d& LastPoint)
{
	GetActorsInSelectionRectangle(FirstPoint, LastPoint, Controller->SelectedActors);
	
	/*for (AActor* OutActor : OutActors)
	{
		UStaticMeshComponent* Mesh = OutActor->FindComponentByClass<UStaticMeshComponent>();
		if (Mesh)
		{
			Mesh->SetRenderCustomDepth(false);
		}
	}
	OutActors.Empty();
	GetActorsInSelectionRectangle(FirstPoint, LastPoint, OutActors);
	for (AActor* OutActor : OutActors)
	{
		UStaticMeshComponent* Mesh = OutActor->FindComponentByClass<UStaticMeshComponent>();
		if (Mesh)
		{
			Mesh->SetRenderCustomDepth(true);
		}
	}*/
}
