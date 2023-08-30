// Fill out your copyright notice in the Description page of Project Settings.

#include "AStarGridActor.h"

#include "LandscapeProxy.h"

UAStarGridActor::UAStarGridActor()
{
}

void UAStarGridActor::InitializeGrid()
{
    int32 width = GridWidth * 2 + 1;
    int32 height = GridHeight * 2 + 1;
    
    Grid.Empty();
    Grid.SetNum(height);
    
    for (int32 y = 0; y < height; ++y)
    {
        Grid[y].grid.Empty();
        Grid[y].grid.SetNum(width);

        for (int32 x = 0; x < width; ++x)
        {
            FGridCell* cell = new FGridCell;
            cell->X = x - GridWidth;
            cell->Y = y - GridHeight;
            cell->bIsBlocked = false;
            cell->G = 0.0f;
            cell->H = 0.0f;
            cell->F = 0.0f;
            cell->Parent = nullptr;
            Grid[y].SetGrid(x, cell);
        }
    }
}

void UAStarGridActor::ConvertToWorldGrid(FVector WorldLocation, int32 cellsize, int32& GridX, int32& GridY)
{
    GridX = FMath::FloorToInt(WorldLocation.X / cellsize);
    GridY = FMath::FloorToInt(WorldLocation.Y / cellsize);
}

TArray<FVector> UAStarGridActor::FindPath(FVector& StartLocation, FVector& TargetLocation)
{
    int32 StartGridX, StartGridY;
    ConvertToWorldGrid(StartLocation, CellSize, StartGridX, StartGridY);

    int32 TargetGridX, TargetGridY;
    ConvertToWorldGrid(TargetLocation, CellSize, TargetGridX, TargetGridY);

    
    FGridCell* StartCell = GetGridCell(StartGridX, StartGridY);
    FGridCell* TargetCell = GetGridCell(TargetGridX, TargetGridY);

    if (!StartCell || !TargetCell || StartCell->bIsBlocked || TargetCell->bIsBlocked)
    {
        // Başlangıç veya hedef hücre geçersiz veya engellenmişse yol bulunamadı
        return TArray<FVector>();
    }
    
    if (StartCell == TargetCell)
    {
        return TArray<FVector>();
    }

    TArray<FGridCell*> OpenSet;
    TArray<FGridCell*> ClosedSet;

    OpenSet.Add(StartCell);
    int tryIndex = 0;
    while (OpenSet.Num() > 0 && tryIndex < 100)
    {
        // En düşük F değerine sahip hücreyı seç
        FGridCell* CurrentCell = OpenSet[0];
        int32 CurrentIndex = 0;

        for (int32 i = 1; i < OpenSet.Num(); ++i)
        {
            if (OpenSet[i]->F < CurrentCell->F)
            {
                CurrentCell = OpenSet[i];
                CurrentIndex = i;
            }
        }

        // Hedefe ulaşıldıysa, yol bulundu
        if (CurrentCell == TargetCell)
        {
            OpenSet.Empty();
            ClosedSet.Empty();
            return ReconstructPath(CurrentCell);
        }

        // Seçilen hücreyi açık kümeden kaldır ve kapalı kümeye ekle
        OpenSet.RemoveAt(CurrentIndex);
        ClosedSet.Add(CurrentCell);

        // Komşu hücreleri al
        TArray<FGridCell*> Neighbors = GetNeighboringCells(CurrentCell);
        for (FGridCell* Neighbor : Neighbors)
        {
            if (ClosedSet.Contains(Neighbor) || Neighbor->bIsBlocked)
            {
                continue; // Zaten kapalı kümeye dahilse veya engellenmişse geç
            }

            float GScore = CurrentCell->G + CalculateDistance(CurrentCell, Neighbor);

            if (!OpenSet.Contains(Neighbor))
            {
                OpenSet.Add(Neighbor);
            }
            else if (GScore >= Neighbor->G)
            {
                continue; // Daha kötü bir yoldan geldiyse geç
            }

            Neighbor->Parent = CurrentCell;
            Neighbor->G = GScore;
            Neighbor->H = CalculateDistance(Neighbor, TargetCell);
            Neighbor->F = Neighbor->G + Neighbor->H;
        }
        
        tryIndex++;
    }

    // Hedefe ulaşılamadı, yol bulunamadı
    OpenSet.Empty();
    ClosedSet.Empty();
    return TArray<FVector>();
}

FVector UAStarGridActor::ConvertGridCellToWorldLocation(FGridCell* GridCell)
{
    FVector WorldLocation(GridCell->X * CellSize, GridCell->Y * CellSize, 0.0f);
    return WorldLocation + GridOffset;
}

FGridCell* UAStarGridActor::GetGridCell(int32 GridX, int32 GridY)
{
    if (GridX < -GridWidth || GridX > GridWidth || GridY < -GridHeight || GridY > GridHeight)
    {
        return nullptr;
    }
    
    int32 correctedGridX = GridX + GridWidth;
    int32 correctedGridY = GridY + GridHeight;

    
    if (GridWidth != 50 || GridHeight != 50)
    {
        UE_LOG(LogTemp, Warning, TEXT("Grid: [%d, %d]"), correctedGridX, correctedGridY);
        UE_LOG(LogTemp, Display, TEXT("X: %d, Y: %d"), GridX, GridY);
        UE_LOG(LogTemp, Display, TEXT("Width: %d, Height: %d"), GridWidth, GridHeight);
        UE_LOG(LogTemp, Display, TEXT("====================="))
        return nullptr;
    }
    return Grid[correctedGridY][correctedGridX];
}

TArray<FGridCell*> UAStarGridActor::GetNeighboringCells(FGridCell* CurrentCell)
{
    TArray<FGridCell*> Neighbors;

    int32 CurrentX = CurrentCell->X;
    int32 CurrentY = CurrentCell->Y;

    for (int32 xOffset = -1; xOffset <= 1; ++xOffset)
    {
        for (int32 yOffset = -1; yOffset <= 1; ++yOffset)
        {
            if (xOffset == 0 && yOffset == 0)
            {
                continue; // Kendi hücresini atla
            }

            int32 NeighborX = CurrentX + xOffset;
            int32 NeighborY = CurrentY + yOffset;

            
            FGridCell* Neighbor = GetGridCell(NeighborX, NeighborY);

            if (Neighbor)
            {
                Neighbors.Add(Neighbor);
            }
        }
    }

    return Neighbors;
}

float UAStarGridActor::CalculateDistance(FGridCell* Start, FGridCell* Target)
{
    FVector StartLocation(Start->X * CellSize, Start->Y * CellSize, 0.0f);
    FVector TargetLocation(Target->X * CellSize, Target->Y * CellSize, 0.0f);

    return FVector::Dist(StartLocation, TargetLocation);
}

TArray<FVector> UAStarGridActor::ReconstructPath(FGridCell* EndCell)
{
    TArray<FVector> Path;

    FGridCell* CurrentCell = EndCell;
    while (CurrentCell)
    {
        FVector WorldLocation(CurrentCell->X * CellSize, CurrentCell->Y * CellSize, 0.0f);
        Path.Insert(WorldLocation, 0);
        FGridCell* TempCell = CurrentCell->Parent;
        CurrentCell->Parent = nullptr;
        CurrentCell = TempCell;
    }

    return Path;
}

