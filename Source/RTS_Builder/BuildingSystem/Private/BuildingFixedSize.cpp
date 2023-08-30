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

void ABuildingFixedSize::OnConstruction(const FTransform& Transform)
{
	FVector Min, Max;
	FVector Box, Origin;

	if (MeshComponent)
	{
		MeshComponent->GetLocalBounds(Min, Max);
		Res = FVector(((FMath::Abs(Max.X) - FMath::Abs(Min.X))) / 2, ((FMath::Abs(Max.Y) - FMath::Abs(Min.Y))) / 2, 0);
		GetActorBounds(false, Origin, Box, true);
		SizeX = FMath::CeilToInt(Box.X * 2 / 100);
		SizeY = FMath::CeilToInt(Box.Y * 2 / 100);
		MeshComponent->AddLocalOffset(-Res);
	}
}

void ABuildingFixedSize::OnClick(ARTSController* Ctrl)
{
	Super::OnClick(Ctrl);
	FActorSpawnParameters Params;
	Params.bNoFail = true;

	if (AttachmentComponent)
	{
		FSocketData* Socket = AttachmentComponent->FindSocket(GetClass()->GetName(), Controller->CursorHitLocation);
		if (Socket)
		{
			 ABuildingFixedSize* building = GetWorld()->SpawnActor<ABuildingFixedSize>(GetClass(), Socket->Transform.GetLocation(),
			                                           Socket->Transform.GetRotation().Rotator(), Params);

			building->AttachToActor(AttachmentComponent->GetOwner(), FAttachmentTransformRules::KeepWorldTransform, Socket->Socket);
			Socket->bIsEmpty = false;
			return;
		}
	}

	ABuildingFixedSize* actor = GetWorld()->SpawnActor<ABuildingFixedSize>(GetClass(), GetActorLocation(), GetActorRotation(), Params);
	//actor->MeshComponent->bFillCollisionUnderneathForNavmesh = true;
	
}

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
	GEngine->AddOnScreenDebugMessage(2, 5, FColor::Orange, FString::Printf(TEXT("SizeX: %d, SizeY: %d"), SizeX, SizeY));
	GEngine->AddOnScreenDebugMessage(7, 5, FColor::Red,
	                                 (MeshComponent->GetComponentLocation() - GetActorLocation()).ToString());
}

void ABuildingFixedSize::OnRender()
{
}

void ABuildingFixedSize::DrawGrid(int32 x, int32 y)
{
	FVector BoxExtent, Origin;
	int32 count = 0;

	for (int i = 0; i < SizeX; ++i)
	{
		for (int j = 0; j < SizeY; ++j)
		{
			for (int k = 0; k < Vectors.Num(); ++k)
			{
				GetActorBounds(false, Origin, BoxExtent, true);
				FVector CeiledExtent = FVector(SizeX * 100 / 2, SizeY * 100 / 2, -1);
				FVector vec = Vectors[k] + FVector(i * 100, j * 100, 0) - CeiledExtent;
				Result.Add(vec);
			}
			ProceduralMeshComponent->CreateMeshSection(count, Result, Triangles, Normals, UV, Colors, Tangents, false);
			Result.Empty();
			++count;
		}
	}
}

void ABuildingFixedSize::UpdateGrid()
{
	if (ProceduralMeshComponent->GetNumSections() > 0)
	{
		for (int i = 0; i < ProceduralMeshComponent->GetNumSections(); ++i)
		{
			ProceduralMeshComponent->GetProcMeshSection(i)->ProcVertexBuffer[0].Position + FVector(50, 50, 0);
			//GetWorld()->LineTraceSingleByChannel()
		}
		return;
	}
	FVector BoxExtent, Origin;
	GetActorBounds(false, Origin, BoxExtent, true);

	int32 count = 0;
	FHitResult HitR;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	for (int i = 0; i < SizeX; ++i)
	{
		for (int j = 0; j < SizeY; ++j)
		{
			bool IsOverExtended = false;
			int32 t = SizeY * i + j;
			for (int k = 0; k < Vectors.Num(); ++k)
			{
				GetActorBounds(false, Origin, BoxExtent, true);
				FVector CeiledExtent = FVector(SizeX * 100 / 2, SizeY * 100 / 2, 0);
				FVector vec = Vectors[k] + FVector(i * 100, j * 100, 1) - CeiledExtent;

				//if (k == 0)
				//{
				//	CastTraceFromLocal(HitResult, FVector(vec.X + 50, vec.Y + 50, 200),FVector::UpVector * -1000, false, ActorsToIgnore, GetTransform());
				//}
				//vec.Z = UKismetMathLibrary::InverseTransformLocation(GetTransform(), HitResult.Location).Z + 1;

				//FMatrix rot = FRotationMatrix(HitResult.ImpactNormal.Rotation());
				//vec = rot.TransformPosition(vec);
				Result.Add(vec);
			}

			ProceduralMeshComponent->CreateMeshSection(count, Result, Triangles, Normals, UV, Colors, Tangents, true);
			if (IsOverExtended || !HitR.bBlockingHit)
			{
				ProceduralMeshComponent->SetMaterial(t, MaterialRed);
				MeshComponent->SetMaterial(0, MaterialRed);
			}
			else
			{
				ProceduralMeshComponent->SetMaterial(t, MaterialGreen);
				MeshComponent->SetMaterial(0, MaterialGreen);
			}

			Result.Empty();
			++count;
		}
	}
}

void ABuildingFixedSize::UpdateDuringBuilding(FVector Endloc, FHitResult& Hit)
{
	Super::UpdateDuringBuilding(Endloc, Hit);
	HitResult = Hit;
	UpdateGrid();
}

void ABuildingFixedSize::StickToGrid(FVector& Location)
{
	int32 x, y;
	FVector& loc = Controller->CursorHitLocation;
	GEngine->AddOnScreenDebugMessage(20, 1, FColor::Green,
	                                 FString::Printf(TEXT("Location: %f, %f, %f"), loc.X, loc.Y, loc.Z));

	if (Controller)
	{
		TSubclassOf<UActorComponent> ComponentClass = UBuildingAttachment::StaticClass();
		if (HitResult.GetActor() && HitResult.GetActor()->GetComponentByClass(ComponentClass))
		{
			AttachmentComponent = Cast<UBuildingAttachment>(HitResult.GetActor()->GetComponentByClass(ComponentClass));
			FSocketData* Socket = AttachmentComponent->FindSocket(GetClass()->GetName(), Controller->CursorHitLocation);
			if (Socket)
			{
				MeshComponent->SetMaterial(0, MaterialGreen);
				SetActorTransform(Socket->Transform);
				return;
			}
		}

		AttachmentComponent = nullptr;
		x = loc.X / 100;
		y = loc.Y / 100;
		FVector Origin, Box;
		GetActorBounds(false, Origin, Box, true);
		SetActorLocation(FVector(
			x * 100 - (int)Box.X % 100,
			y * 100 - (int)Box.Y % 100,
			Location.Z));
		SetActorRotation(FRotator::ZeroRotator);
	}
}
