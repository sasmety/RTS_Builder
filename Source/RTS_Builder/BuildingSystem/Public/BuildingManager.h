// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingBase.h"
#include "Engine/DataTable.h"
#include "RTS_Builder/MouseInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BuildingManager.generated.h"

/**
 * 
 */

class UBuilderBase;
// class UBuilderBase;
UENUM(BlueprintType)
namespace BuildingType
{
	enum Type
	{
		Road,
		Wall,
		Region,
		Static,
		None
	};
}

USTRUCT(BlueprintType)
struct FBuildingData : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FName Name;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<ABuildingBase> BuildingClass;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UBuilderBase> BuilderClass;
	UPROPERTY(BlueprintReadWrite)
	FText Desc;
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
	UPROPERTY(BlueprintReadWrite)
	double Cost;
	UPROPERTY(BlueprintReadWrite)
	double Health;

	FBuildingData() = default;

	FBuildingData(const FName& Name, const TSubclassOf<ABuildingBase>& BuildingClass, const TSubclassOf<UBuilderBase>& BuilderClass, const FText& Desc, double Cost, double Health)
	{
		this->Name = Name;
		this->BuildingClass = BuildingClass;
		this->BuilderClass = BuilderClass;
		this->Desc = Desc;
		this->Cost = Cost;
		this->Health = Health;
	}

	FBuildingData& operator=(const FBuildingData& Other)
	{
		if (this == &Other)
			return *this;
		Name = Other.Name;
		BuildingClass = Other.BuildingClass;
		BuilderClass = Other.BuilderClass;
		Desc = Other.Desc;
		Cost = Other.Cost;
		Health = Other.Health;
		return *this;
	}
};

UCLASS(BlueprintType)
class RTS_BUILDER_API UBuildingManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void AddBuilding(ABuildingBase* Building);
	void RemoveBuilding(ABuildingBase* Building);
	void RenderBuildings();
	UFUNCTION(BlueprintCallable)
	void CreateBuilder(const FBuildingData& Data);
	UPROPERTY()
	UBuilderBase* Builder;
	UPROPERTY()
	TArray<ABuildingBase*> Buildings;
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FBuildingData> ItemDataTable;
};
