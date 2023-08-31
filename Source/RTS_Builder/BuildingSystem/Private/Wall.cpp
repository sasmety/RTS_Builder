// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/Wall.h"

#include "Components/SplineComponent.h"

bool AWall::AddSplinePoint(FVector& WorldPoint)
{
	return Super::AddSplinePoint(WorldPoint);
}

void AWall::RotatePoint(int32 Index, float Value)
{
	Super::RotatePoint(Index, Value);
}

void AWall::BeginPlay()
{
	Super::BeginPlay();
}

void AWall::Update(float DeltaSeconds, FHitResult& Hit)
{
	Super::Update(DeltaSeconds, Hit);
}