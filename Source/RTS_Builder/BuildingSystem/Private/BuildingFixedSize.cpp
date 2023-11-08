// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuildingFixedSize.h"

#include <string>

#include "RTS_Builder/BuildingSystem/Public/BuildingAttachment.h"
#include "RTS_Builder/RTSController.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

ABuildingFixedSize::ABuildingFixedSize()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	MeshComponent->SetupAttachment(GetRootComponent());
}

void ABuildingFixedSize::AssignAssets()
{
	FSoftObjectPath MeshPath(TEXT("/Game/Assets/Buildings/lowpolyhouse.lowpolyhouse"));	
	StaticMesh = Cast<UStaticMesh>(MeshPath.TryLoad());
	if (StaticMesh)
		MeshComponent->SetStaticMesh(StaticMesh);
}

void ABuildingFixedSize::OnConstruction(const FTransform& Transform)
{
	AssignAssets();
}

// void ABuildingFixedSize::OnClick(ARTSController* Ctrl)
// {
// 	Super::OnClick(Ctrl);
// 	FActorSpawnParameters Params;
// 	Params.bNoFail = true;
//
// 	if (AttachmentComponent)
// 	{
// 		FSocketData* Socket = AttachmentComponent->FindSocket(GetClass()->GetName(), Controller->CursorHitLocation);
// 		if (Socket)
// 		{
// 			 ABuildingFixedSize* building = GetWorld()->SpawnActor<ABuildingFixedSize>(GetClass(), Socket->Transform.GetLocation(),
// 			                                           Socket->Transform.GetRotation().Rotator(), Params);
//
// 			building->AttachToActor(AttachmentComponent->GetOwner(), FAttachmentTransformRules::KeepWorldTransform, Socket->Socket);
// 			Socket->bIsEmpty = false;
// 			return;
// 		}
// 	}
//
// 	ABuildingFixedSize* actor = GetWorld()->SpawnActor<ABuildingFixedSize>(GetClass(), GetActorLocation(), GetActorRotation(), Params);
// 	//actor->MeshComponent->bFillCollisionUnderneathForNavmesh = true;
// 	
// }

void ABuildingFixedSize::OnStartBuilding(ARTSController* Ctrl)
{
	Super::OnStartBuilding(Controller);
	Controller = Ctrl;
	//UpdateGrid();
}

void ABuildingFixedSize::BeginPlay()
{
	Super::BeginPlay();
}

void ABuildingFixedSize::Tick(float DeltaSeconds)
{
	
}

void ABuildingFixedSize::OnRender()
{
}


