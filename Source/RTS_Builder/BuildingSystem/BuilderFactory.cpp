// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderFactory.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Public/BuilderWall.h"
#include "RTS_Builder/GameManager.h"

UBuilderBase* UBuilderFactory::CreateBuilder(FBuildingData Data)
{
	
	UBuilderBase* Builder = NewObject<UBuilderBase>(Data.BuilderClass);
	Builder->Init(Cast<UGameManager>(UGameplayStatics::GetGameInstance(GetWorld())), Data);
	return Builder;
	// switch (Name)
	// {
	// case FName("Wall"):
	// 	return NewObject<UBuilderWall>();
	// case FName("Road"):
	// 	return NewObject<UBuilderWall>();
	// case FName("Static"):
	// 	return NewObject<UBuilderWall>();
	// case FName("Region"):
	// 	return NewObject<UBuilderWall>();
	// case FName("Attachment"):
	// 	return NewObject<UBuilderWall>();
	// default:
	// 	return nullptr;
	// }
}
