// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuilderRoad.h"
#include "SplineBuilding.h"
#include "Components/SplineMeshComponent.h"
#include "RTS_Builder/BuildingSystem/RoadIntersection.h"
#include "UObject/Object.h"
#include "Road.generated.h"

/**
 * 
 */

UCLASS()
class RTS_BUILDER_API ARoad : public ASplineBuilding
{
	GENERATED_BODY()
public:
	ARoad();
	~ARoad();
	void Init();
	bool AddSplinePoint(FVector& WorldPoint, bool bShouldStore) override;
	void BreakIntersection(ARoadIntersection* Intersection);
	void RemoveIntersection(ARoadIntersection* Intersection);
	void RotatePoint(int32 Index, float Value) override;
	void BeginPlay() override;
	void Update(float DeltaSeconds, FHitResult& Hit) override;
	void SetGhostMaterialColorRGB(const FVector4d& Color);
	void CreateCollisionBoxesAlongSpline(float Interval, FVector& Size);
	UFUNCTION()
	void UpdateIntersections();
	void DestructBuilding(FHitResult& HitData, float size) override;
	virtual int32 GetClosestPoint(FVector& WorldPoint, USplineComponent* TargetSpline) override;
	int32 GetDeletePoint(FVector& WorldPoint, USplineComponent* TargetSpline);
	bool FindAttachmentPoint(FVector& WorldPoint, FAttachmentData& Data, FString type);
	void GenerateStoredSplineMeshes();
	void RenderMeshAtSplineDistance(UStaticMesh* StaticMesh, float StartValue, float EndValue, int32 StorageIndex);
	void GenerateSplineMesh(UStaticMesh* StaticMesh, bool bUseStoredMeshes) override;
	void GenerateSplineMesh(UStaticMesh* StaticMesh, float StartDistance, float EndDistance);
	void CreateJunction(USplineComponent* Target, FVector& WorldPoint, float offset, bool bShouldStore);
	void CreateTwoRoadJunction(ARoad* ConstRoad, ARoad* MovingRoad, FVector WorldLocation, float offset);
	void DrawGhost(FHitResult& HitData, float size);
	FIntersectionPoint GetFinalAttachPoint(float ConstDist, float MovingDist, float MovingMax, ARoad* MovingRoad, ARoad* ConstRoad, ARoadIntersection* Intersection);
	FIntersectionPoint CutToFromStart(float dist);
	FIntersectionPoint CutFromToEnd(float dist);
	ARoad* Split(float CutPoint, float CutDistance);
	TArray<TObjectPtr<USplineMeshComponent>> DetectIntersectionMeshes(ARoad* Road, const FVector& StartLoc, const FVector& EndLoc, FVector Rot);
	void AddIntersection(ARoadIntersection* Intersection, TArray<TObjectPtr<USplineMeshComponent>> Components);
	virtual void Tick(float DeltaSeconds) override;

	TMap<int32, TArray<ARoadIntersection*>> RoadConnections;
	UPROPERTY()
	ARoadIntersection* CurrentSection = nullptr;
	float MeshLength = 100.f;
	TObjectPtr<ARoadIntersection> StartIntersection = nullptr;
	TObjectPtr<ARoadIntersection> EndIntersection = nullptr;

	UPROPERTY()
	TMap<FString, AActor*> Attachments;  
	
	TMap<TWeakObjectPtr<USplineMeshComponent>, float> MeshDistances;
	TArray<TObjectPtr<USplineMeshComponent>> meshes;

	TArray<TObjectPtr<USplineMeshComponent>> CachedMeshes;

private:
	UPROPERTY()
	UMaterialInterface* GhostMaterial;
	UPROPERTY()
	UMaterialInstanceDynamic* GhostMaterialInstance;
};
