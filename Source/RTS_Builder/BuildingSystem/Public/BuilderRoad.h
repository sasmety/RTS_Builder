// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuilderBase.h"
#include "UObject/Object.h"
#include "BuilderRoad.generated.h"

class ARoadIntersection;
class ARoad;
/**
 * 
*/

USTRUCT()
struct FAttachmentData
{
	GENERATED_BODY()
public:
	TObjectPtr<ARoadIntersection> Intersection = nullptr;
	TObjectPtr<ARoad> Road = nullptr;
	FVector Loc;
	FVector Dir;

	FAttachmentData() = default;

	FAttachmentData(const TObjectPtr<ARoad>& Road, const FVector& Loc, const FVector& Dir)
		: Road(Road),
		  Loc(Loc),
		  Dir(Dir)
	{
	}

	FAttachmentData(const TObjectPtr<ARoadIntersection>& Intersection, const FVector& Loc, const FVector& Dir)
		: Intersection(Intersection),
		  Loc(Loc),
		  Dir(Dir)
	{
	}
};

UENUM()
enum class EBuildingMode : uint8
{
	Free UMETA(DisplayName = "Attached"),
	Placed UMETA(DisplayName = "Follow")
};
UCLASS()
class RTS_BUILDER_API UBuilderRoad : public UBuilderBase
{
	GENERATED_BODY()
public:
	UBuilderRoad();
	virtual void Init(UGameManager* Manager, const FBuildingData& Data) override;
	void Destroy() override;
	virtual void CreateBuilding() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void LeftPressed() override;
	void Closer();
	void AddConnectionData(const FAttachmentData& Data);
	bool AssignToConnections();
	virtual void LeftHold() override;
	virtual void LeftReleased() override;
	void ExitBuilder();
	void RightPressed();
	virtual void KeyPressed(FKey Key) override;

	TArray<FAttachmentData> ConnectionData;
	
	TMap<TObjectPtr<ARoad>, FVector> ConnectedRoads;
	TMap<TObjectPtr<ARoadIntersection>, FVector> ConnectedIntersections;
	
	UPROPERTY()
	ARoad* CurrentBuilding = nullptr;
	EBuildingMode Mode;

private:
	float RoadCutSize = 200;
	
};
