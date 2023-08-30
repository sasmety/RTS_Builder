// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LandscapeProxy.h"
#include "MouseInterface.h"
#include "RTS_Builder/BuildingSystem/Public/BuildingBase.h"
#include "AStarGridActor.h"
#include "GameManager.h"
#include "LivingEntity.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerController.h"
#include "RTSController.generated.h"

/**
 * 
 */

class UBuilderBase;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class RTS_BUILDER_API ARTSController : public APlayerController
{
	GENERATED_BODY()

public:
	ARTSController();
	void OnLeftClick();
	void OnLeftReleased();
	void OnRightPressed();
	void OnRightReleased();
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void KeyPressed(FKey Key);
	void KeyReleased(FKey key);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void CameraMove(float AxisValue);

	TObjectPtr<UAStarGridActor> GridManager;

	UFUNCTION(BlueprintCallable)
	void CameraRotateY(float DeltaSeconds);

	UFUNCTION(BlueprintCallable)
	void CameraRotateX(float DeltaSeconds);

	float GetSmoothTargetArmLength(float AxisValue, float& CamZoomDestination);

	UFUNCTION(BlueprintCallable)
	void Zoom(float AxisValue);
	
	UFUNCTION(BlueprintCallable)
	bool CursorHit(ECollisionChannel CollisionChannel, TArray<AActor*> IgnoredActors, FHitResult& HitResult);

	UFUNCTION(BlueprintCallable)
	void SetGhostBuilding(TSubclassOf<ABuildingBase> Building);

	UFUNCTION(BlueprintCallable)
	void ExitBuildMode();

	UFUNCTION(BlueprintCallable)
	void StickBuildingToGrid();

	UMaterialInstanceDynamic* CreateMaterialInstance(FLinearColor Color, float Opacity);
	void SwitchLandscapeMaterialParamaters();
	void DebugAtWorldPoints(TArray<FVector> grids);

	UPROPERTY(EditAnywhere)
	UBlackboardComponent* BlackboardComponent;

	UPROPERTY(EditAnywhere)
	UBehaviorTreeComponent* BehaviorTreeComponent;

	UFUNCTION(BlueprintCallable)
	void RotateBuilding();
	void Escape();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	double CameraMoveSpeed = 30.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	double CameraRotationSpeed = 130.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	USpringArmComponent* SpringArm;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	UCameraComponent* CameraComponent;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	bool bReverseYRotation;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	float TargetFOV;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	bool bIsRightClicked;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	bool bIsLeftClicked;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	bool bIsBuildMode;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	int32 GridX;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	int32 GridY;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	class ABuildingBase* GhostBuilding;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FVector CursorHitLocation;

	IMouseInterface* Interface;
	IMouseInterface* BuilderInterface;

	UPROPERTY()
	UGameManager* GameManager = nullptr;

	UPROPERTY(EditAnywhere, Category = "Configuration");
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	ALandscapeProxy* LandscapeProxy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TSubclassOf<ABuildingBase> SpawnItem = ABuildingBase::StaticClass();

	FGridCell* StartCell;

	FGridCell* EndCell;

	FVector StartLoc;

	FRotator BuildingRotation = FRotator(0,0,0);
	
	UPROPERTY()
	TArray<ALivingEntity*> SelectedActors;
};
