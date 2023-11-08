// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderFactory.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Public/BuilderWall.h"
#include "RTS_Builder/GameManager.h"

UBuilderBase* UBuilderFactory::CreateBuilder(FBuildingData Data)
{
	
	UBuilderBase* Builder = NewObject<UBuilderBase>(this, Data.BuilderClass);
	Builder->Init(Cast<UGameManager>(UGameplayStatics::GetGameInstance(GetWorld())), Data);
	return Builder;
}
