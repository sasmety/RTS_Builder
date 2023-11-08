// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingBase.h"
#include "SplineBuilding.generated.h"

struct FAttachmentData;
class ARoad;
/**
 * 
 */
UCLASS()
class RTS_BUILDER_API ASplineBuilding : public ABuildingBase
{
	GENERATED_BODY()
public:
	ASplineBuilding();
	int32 GetSplinePointsNum();
	virtual void OnClick(FVector& Point);
	virtual int32 GetClosestPoint(FVector& WorldPoint, USplineComponent* TargetSpline);
	// TOptional<FVector> FindAttachmentPoint(FVector& WorldPoint, TArray<AActor*>& AttachmentActors);
	virtual bool FindAttachmentPoint(FVector& WorldPoint, FAttachmentData& Data, FString type);
	virtual void BeginPlay() override;
	virtual bool AddSplinePoint(FVector& WorldPoint, bool bShouldStore);
	virtual void RotatePoint(int32 Index, float Value);
	virtual void GenerateSplineMesh(UStaticMesh* Mesh, bool bUseStoredMeshes);
	void ClearSpline();
	void ClearSplineMeshes();
	virtual void Update(float DeltaSeconds, FHitResult& Hit) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	bool SnapSplineToLandscape(float Start, float End, float Interval, USplineComponent* SplineComp, TArray<FVector>& OutArray);
	TArray<FVector> FindPathBetweenPoints(const FVector& StartCoord, const FVector& EndCoord, TArray<AActor*> AttachedActors);
	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USplineComponent* SplineComponent;

	UPROPERTY(EditDefaultsOnly)
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* Mesh;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PillarMesh;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* BuildingMaterial;
	
	UPROPERTY(EditDefaultsOnly)
	URuntimeVirtualTexture* RVTexture;
	
	UPROPERTY(EditDefaultsOnly)
	bool bShouldCastShadows;
};
