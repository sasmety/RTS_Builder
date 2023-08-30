// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingBase.h"
#include "RTS_Builder/RTSController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BuildingSpline.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API ABuildingSpline : public ABuildingBase, public INavAgentInterface
{
	GENERATED_BODY()
public:
	ABuildingSpline(const FObjectInitializer& OI);
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void OnClick(ARTSController* Controller) override;
	virtual void DrawGrid(int32 x, int32 y) override;
	virtual void UpdateGrid() override;
	virtual void OnRender() override;
	virtual void UpdateDuringBuilding(FVector Endloc, FHitResult& Hit) override;
	virtual void StickToGrid(FVector& Location) override;
	virtual void OnStartBuilding(ARTSController* Ctrl) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual FVector GetNavAgentLocation() const override;
	virtual const FNavAgentProperties& GetNavAgentPropertiesRef() const override;
	void MakeRoad(FVector Start, FVector End);
	void CreateRoadSegment();
	void Resize();
	bool DidResized();
	
	TArray<FVector> FindPathBetweenPoints(const FVector& StartPoint, const FVector& EndPoint);
	FVector FindNearestSplinePointToWorldLocation(USplineComponent* SplineComponent, const FVector& ClickLocation, int& OutIndex);
	TArray<FVector> DivideVector(const FVector& Start, const FVector& End, float DistanceInterval);
	TArray<FHitResult> MakeHit(FVector StartLoc, FVector EndLoc, FVector Size, FRotator Rot, TArray<AActor*> IgnoredActors, EDrawDebugTrace::Type
	                           DebugType);
	
	

	int32 Sign(int32 value);
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Building")
	float AgentHeight = 144;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Building")
	float AgentHalfRadius = 13;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Building")
	class UInstancedStaticMeshComponent* InSceneBuildings;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Building")
	USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	class UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	class UStaticMesh* PillarMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	UMaterialInterface* MaterialVirtual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	URuntimeVirtualTexture* RuntimeVirtualTextureMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSubclassOf<UNavigationQueryFilter> QueryFilter;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FVector TangentStart = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FVector TangentEnd = FVector::ZeroVector;
	
	FVector StartLocation;
	FVector EndLocation;
	TOptional<FVector> StartTan;
	TOptional<FVector> StartPoint;
	TOptional<FVector> EndTan;
	TOptional<FVector> EndPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool bIsBuilding = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool bCanBeBuilt = true;
	bool hasmoved = false;

	UPROPERTY(EditAnywhere)
	float MeshLength = 200;
	

	UPROPERTY()
	FNavAgentProperties NavAgent;
	UPROPERTY()
	ANavigationData* NavData;


	//UPROPERTY()
	//USplineComponent* SplineComponent;

};


