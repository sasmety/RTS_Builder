// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingBase.h"
#include "Components/ActorComponent.h"
#include "BuildingAttachment.generated.h"

USTRUCT()
struct FSocketData
{
	GENERATED_BODY()
public:
	FName Socket;
	FTransform Transform;
	bool bIsEmpty = true;
	TArray<FString> CompatibleActors;

	FSocketData(const FName& Socket, const FTransform& Transform, bool bIsEmpty,
		const TArray<FString>& CompatibleActors)
		: Socket(Socket),
		  Transform(Transform),
		  bIsEmpty(bIsEmpty),
		  CompatibleActors(CompatibleActors)
	{
	}

	FSocketData(){}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTS_BUILDER_API UBuildingAttachment : public UActorComponent
{
	GENERATED_BODY()
public:	
	// Sets default values for this component's properties
	UBuildingAttachment();
	
public:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SplitSocketNames();
	FSocketData* FindSocket(FString ClassName, FVector& CursorLocation);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	TMap<FString, TArray<TPair<FTransform, bool>>> Attachments;
	TArray<FSocketData*> Sockets;
	TMap<FString, TArray<FSocketData*>> Components;
};
