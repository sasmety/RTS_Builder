#pragma once

#include "CoreMinimal.h"
#include "LandscapeProxy.h"
#include "NavModifierComponent.h"
#include "RTS_Builder/People.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "BuildingBase.generated.h"

class ARTSController;

// UENUM(BlueprintType)
// enum class EBuildingMode : uint8 {
// 	BM_Follow       UMETA(DisplayName="FollowCursor"),
// 	BM_StaticStart        UMETA(DisplayName="StaticStartLocation"),
// 	BM_StaticLocation        UMETA(DisplayName="StaticLocation"),
// };

UCLASS()
class RTS_BUILDER_API ABuildingBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildingBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void OnClick(ARTSController* Controller);
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnStartBuilding(ARTSController* Controller);
	void Repair();
	virtual void DrawGrid(int32 x, int32 y);
	virtual void UpdateGrid();
	void RemoveGrid();
	virtual void OnRender();
	virtual void Update(float DeltaSeconds, FHitResult& Hit);
	virtual void UpdateDuringBuilding(FVector Endloc, FHitResult& Hit);
	virtual void StickToGrid(FVector& Location);
	virtual bool CanBeBuilt();
	virtual void SetGhostMaterial();
	virtual void SetDefaultMaterial();
	virtual void AssignAssets();
	virtual void DestructBuilding(FHitResult& HitData, float size);
	
	APlayerController* GetPlayerController();
	
	UPROPERTY(EditDefaultsOnly)
	class UNavModifierComponent* NavModifier;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Stats")
	class USceneComponent* RootComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UProceduralMeshComponent* ProceduralMeshComponent;
	
	
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stats")
	int32 SizeX;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stats")
	int32 SizeY;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stats")
	float Health;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stats")
	bool b_IsCompleted;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stats")
	bool bShouldStartTicking = false;
	// UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stats")
	// EBuildingMode BuildingMode = EBuildingMode::BM_Follow;
	
	
	FVector Res;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh", meta=(MakeEditWidget=true))
	TArray<FVector> Vertices;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	UMaterialInterface* MaterialRed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	UMaterialInterface* MaterialGreen;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Stats")
	UMaterialInstance* OverlayMaterialInstance;
	UPROPERTY()
	float ActorLocationZ;
	UPROPERTY()
	class ALandscapeProxy* LandscapeProxy;

	FHitResult HitUnderCursor;
	

	TArray<FVector> Vectors;
	TArray<FVector> Result;
	TArray<FVector> Normals;
	TArray<int32> Triangles;
	TArray<FVector2d> UV;
	TArray<FColor> Colors;
	TArray<FProcMeshTangent> Tangents;

	TObjectPtr<ARTSController> Controller;

};
