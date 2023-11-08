// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingBase.h"
#include "RTS_Builder/PointActor.h"
#include "RTS_Builder/MouseInterface.h"
#include "BuildingResizable.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API ABuildingResizable : public ABuildingBase
{
	GENERATED_BODY()

public:
	ABuildingResizable(const FObjectInitializer& OI);
	virtual void BeginPlay() override;
	void AddSplinePoint(FVector& WorldPoint);
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	bool DidResized();
	//UStaticMesh* ConvertProceduralMeshToStaticMesh(UProceduralMeshComponent* ProcMesh);
	virtual void UpdateGrid() override;
	virtual void OnRender() override;
	virtual void UpdateDuringBuilding(FVector Endloc, FHitResult& Hit) override;
	void AddPointToSpline(USplineComponent* SplineComponent, int32 Index, FVector Location);
	virtual void StickToGrid(FVector& Location) override;
	virtual void OnClick(ARTSController* Controller) override;
	void FindIndexBetweenPoints(USplineComponent* SplineComponent, FVector Location, int32& LowerIndex,
	                            int32& UpperIndex);
	virtual void OnStartBuilding(ARTSController* Controller) override;
	void DrawPointActors(TArray<FVector>& Locations);
	UStaticMesh* MakeMesh(TArray<FVector> locations);
	FVector GetCurrentSplinePoint(USplineComponent* SplineComp, FVector Loc);
	TArray<FHitResult> MakeHit(FVector StartLoc, FVector EndLoc, FRotator Rot, TArray<AActor*> IgnoredActors,
	                           int32 type);
	void FindMeshPoints(TArray<FVector>& path);



	int32 Sign(int32 value);
	FVector FindNearestSplinePointToWorldLocation(USplineComponent* SplineComponent, const FVector& ClickLocation,
	                                              int& OutIndex);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Building")
	UInstancedStaticMeshComponent* InSceneBuildings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	class UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	class UStaticMesh* SphereMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	UMaterialInterface* MaterialVirtual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	URuntimeVirtualTexture* RuntimeVirtualTextureMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSubclassOf<APointActor> SpawnItem = APointActor::StaticClass();

	UPROPERTY()
	UStaticMeshComponent* RootMesh;

	FVector StartLocation;
	FVector EndLocation;
	bool bIsBuilding = false;
	UPROPERTY()
	APointActor* component = nullptr;
	UPROPERTY()
	TArray<APointActor*> boxes;
	//TObjectPtr<ARTSController> Controller;
};
