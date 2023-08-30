// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingAttachment.h"
#include "BuildingBase.h"

#include "BuildingFixedSize.generated.h"
/**
 * 
 */
UCLASS()
class RTS_BUILDER_API ABuildingFixedSize : public ABuildingBase
{
	GENERATED_BODY()
public:
	ABuildingFixedSize();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRender() override;
	virtual void DrawGrid(int32 x, int32 y) override;
	virtual void UpdateGrid() override;
	virtual void UpdateDuringBuilding(FVector Endloc, FHitResult& Hit) override;
	virtual void StickToGrid(FVector& Location) override;
	virtual void OnStartBuilding(ARTSController* Controller) override;
	virtual void OnClick(ARTSController* Controller) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UStaticMeshComponent* MeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UStaticMesh* StaticMesh;

	UPROPERTY()
	FHitResult HitResult;
	UPROPERTY()
	UBuildingAttachment* AttachmentComponent;
};
