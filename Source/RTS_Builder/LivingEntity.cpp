// Fill out your copyright notice in the Description page of Project Settings.


#include "LivingEntity.h"

#include "Kismet/KismetMathLibrary.h"
#include "Tasks/AITask_MoveTo.h"

// Sets default values
ALivingEntity::ALivingEntity()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = ControllerAI;
}

// Called when the game starts or when spawned
void ALivingEntity::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALivingEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALivingEntity::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

