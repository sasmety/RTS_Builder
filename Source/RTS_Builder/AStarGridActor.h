// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStarGridActor.generated.h"



USTRUCT()
struct FGridCell
{
	GENERATED_BODY()
public:
	int32 X;

	int32 Y;

	bool bIsBlocked;

	float G;

	float H;

	float F;

	FGridCell* Parent;
	FGridCell()
		: X(0), Y(0), bIsBlocked(false), G(0.0f), H(0.0f), F(0.0f), Parent(nullptr)
	{
	}
};

USTRUCT()
struct FGridArrayY
{
	GENERATED_BODY()
public:
	TArray<FGridCell*> grid;
	FGridCell* operator[](int32 i)
	{
		return grid[i];
	}
	void operator=(FGridCell* cell)
	{
		grid[cell->X] = cell;
	}
	void SetGrid(int32 x, FGridCell* cell)
	{
		grid[x] = cell;
	}
	
};

UCLASS()
class RTS_BUILDER_API UAStarGridActor : public UObject
{
	GENERATED_BODY()

public:
	UAStarGridActor();

protected:
	UPROPERTY(EditDefaultsOnly)
	int32 GridWidth = 50;

	UPROPERTY(EditDefaultsOnly)
	int32 GridHeight = 50;

	UPROPERTY()
	int32 CellSize = 100;

	UPROPERTY(EditAnywhere)
	FVector GridOffset = FVector(0,0,0);

	UPROPERTY()
	TArray<FGridArrayY> Grid;

public:
	UFUNCTION(BlueprintCallable)
	void InitializeGrid();
	void ConvertToWorldGrid(FVector WorldLocation, int32 CellSize, int32& GridX, int32& GridY);

	UFUNCTION(BlueprintCallable)
	TArray<FVector> FindPath(FVector& StartLocation, FVector& TargetLocation);
	FVector ConvertGridCellToWorldLocation(FGridCell* GridCell);

public:
	FGridCell* GetGridCell(int32 GridX, int32 GridY);

	TArray<FGridCell*> GetNeighboringCells(FGridCell* CurrentCell);

	float CalculateDistance(FGridCell* Start, FGridCell* Target);

	TArray<FVector> ReconstructPath(FGridCell* EndCell);
};
