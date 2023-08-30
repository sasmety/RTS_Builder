// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuilderBase.h"
#include "BuildingBase.h"
#include "RTS_Builder/MouseInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BuildingManager.generated.h"

/**
 * 
 */
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

struct FBuildingData;

USTRUCT()
struct FBuildingCategory
{
	GENERATED_BODY()
	FName Name;
	TArray<FBuildingData> Buildings;
};

USTRUCT()
struct FBuildingData
{
	GENERATED_BODY()
public:
	FName Name;
	TSubclassOf<ABuildingBase> BuildingClass;
	TSubclassOf<UBuilderBase> BuilderClass;
	FText Desc;
	UTexture2D Icon;
	double Cost;
	double Health;
	FBuildingCategory Category;

	FBuildingData(const FName& Name, const TSubclassOf<ABuildingBase>& BuildingClass, const TSubclassOf<UBuilderBase>& BuilderClass, const FText& Desc, const UTexture2D& Icon, double Cost, double Health, const FBuildingCategory& Category)
		: Name(Name),
		  BuildingClass(BuildingClass),
		  BuilderClass(BuilderClass),
		  Desc(Desc),
		  Icon(),
		  Cost(Cost),
		  Health(Health),
		  Category(Category)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Green, TEXT("classic"));
	}

	FBuildingData(const FBuildingData& Other)
		: Name(Other.Name),
		  BuildingClass(Other.BuildingClass),
		  BuilderClass(Other.BuilderClass),
		  Desc(Other.Desc),
		  Icon(),
		  Cost(Other.Cost),
		  Health(Other.Health),
		  Category(Other.Category)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Green, TEXT("single"));
	}

	FBuildingData(FBuildingData&& Other) noexcept
		: Name(std::move(Other.Name)),
		  BuildingClass(std::move(Other.BuildingClass)),
		  BuilderClass(std::move(Other.BuilderClass)),
		  Desc(std::move(Other.Desc)),
		  Icon(),
		  Cost(Other.Cost),
		  Health(Other.Health),
		  Category(std::move(Other.Category))
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Green, TEXT("Double noexcept"));
	}

	FBuildingData& operator=(FBuildingData&& Other) noexcept
	{
		if (this == &Other)
			return *this;
		Name = std::move(Other.Name);
		BuildingClass = std::move(Other.BuildingClass);
		BuilderClass = std::move(Other.BuilderClass);
		Desc = std::move(Other.Desc);
		Cost = Other.Cost;
		Health = Other.Health;
		Category = std::move(Other.Category);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Green, TEXT("Double noexcept assign"));
		return *this;
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
		Category = Other.Category;
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Green, TEXT("assign signle"));
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
	void SelectBuilder(BuildingType::Type BuildingType);

	UPROPERTY()
	UBuilderBase* Builder;
	UPROPERTY()
	TArray<ABuildingBase*> Buildings;

	bool bIsBuildingMode = false;
};
