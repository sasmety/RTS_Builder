// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Public/BuilderBase.h"
#include "UObject/Object.h"
#include "BuilderFactory.generated.h"

/**
 * 
 */
UCLASS()
class RTS_BUILDER_API UBuilderFactory : public UObject
{
	GENERATED_BODY()
public:
	UBuilderBase* CreateBuilder(FBuildingData Data);
};
