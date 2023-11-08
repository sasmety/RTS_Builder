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
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;
	void OnRender() override;
	void OnStartBuilding(ARTSController* Controller) override;
	// void OnClick(ARTSController* Controller) override;
	void AssignAssets() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UStaticMeshComponent* MeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UStaticMesh* StaticMesh;

	UPROPERTY()
	FHitResult HitResult;
	UPROPERTY()
	UBuildingAttachment* AttachmentComponent;
};
