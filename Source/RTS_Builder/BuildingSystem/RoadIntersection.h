// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "RoadIntersection.generated.h"


USTRUCT()
struct FIntersectionPoint
{
	GENERATED_BODY()
public:
	FVector Loc, Dir;

};

class ARoad;
UCLASS()
class RTS_BUILDER_API ARoadIntersection : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoadIntersection();
	void Init(TArray<ARoad*> road, TArray<FIntersectionPoint> point, FVector& AttachLoc, float TanLength);
	// void Init(FVector& FirstLoc, FVector& FirstDir, FVector& SecondLoc, FVector& SecondDir, FVector& EndLoc, FVector& EndDir);
	void CalculateSplines();
	void DrawArrow(FVector& Loc, FVector& Dir, float len, FColor color);
	TArray<FVector> GenerateSplineMesh(USplineComponent* SplineComponent, UStaticMesh* StaticMesh, float HeightOffset, UMaterialInstanceDynamic* Material, float Direction);
	void PropagateSplineDeformationToMesh(const USplineComponent* InSplineComponent, FVector& InOutVector);
	void AddConnection(ARoad* Road, FIntersectionPoint& Point, float CutSize);
	void DisconnectRoad(ARoad* Road);
	void ClearSplines();
	void DestroyConnections(ARoad* Road);
	void UpdateRoadData(ARoad* Road, FIntersectionPoint Data);
	void SortRoadsByAnglesAccordingToCenter();
	TArray<FVector> DrawBorderDebugLocationsOfSpline(USplineComponent* SplineComponent, int32 Direction, USplineMeshComponent* InSplineMeshComponent, float Start, float End);
	void DebugEdge(const FVector& Start, const FVector& End);
	TArray<FVector> GetLocs(USplineMeshComponent* SplineMeshComponent);
	TArray<FVector> CalculatePoints(USplineComponent* SplineComponent, int32 Direction, int32 NumOfDesiredPoints);

	
	UStaticMesh* MakeMesh(TArray<FVector> locations);
	TArray<TObjectPtr<USplineComponent>> Splines;
	TArray<ARoad*> Roads;
	TMap<ARoad*, FIntersectionPoint> Connections;
	FVector AttachPoint;
	UPROPERTY()
	UMaterialInterface* GhostMaterial;
	UPROPERTY()
	UMaterialInterface* NonLineMaterial;
	UPROPERTY()
	URuntimeVirtualTexture* RVTexture;

	UPROPERTY()
	UStaticMeshComponent* RootMesh;


	UPROPERTY()
	UStaticMesh* Mesh;
	float MeshLength = 100;
	float TangentLength = 100;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
