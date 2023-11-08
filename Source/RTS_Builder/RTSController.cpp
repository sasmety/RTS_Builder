// Fill out your copyright notice in the Description page of Project Settings.

#include "RTSController.h"

#include "AStarGridActor.h"
#include "DrawDebugHelpers.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "Layers/Layer.h"
#include "MouseInterface.h"
#include "VectorTypes.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/HUD.h"
#include "RTS_Builder/BuildingSystem/Public/BuildingBase.h"
#include "ProceduralMeshComponent.h"
#include "AStarGridActor.h"
#include "AI/NavigationSystemBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BuildingSystem/Public/BuilderBase.h"
#include "BuildingSystem/Public/BuildingManager.h"
#include "BuildingSystem/Public/SplineBuilding.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"


ARTSController::ARTSController()
{
	PrimaryActorTick.bCanEverTick = true;
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviourTree"));
}

void ARTSController::BeginPlay()
{
	Super::BeginPlay();
	SpringArm = GetPawn()->FindComponentByClass<USpringArmComponent>();
	CameraComponent = GetPawn()->FindComponentByClass<UCameraComponent>();
	SetShowMouseCursor(true);
	GameManager = Cast<UGameManager>(GetGameInstance());
	Interface = Cast<IMouseInterface>(GetHUD());
	
	if (GameManager)
	{
		GameManager->SetController(this);
	}
	BuilderInterface = Cast<IMouseInterface>(GameManager);
	GridManager = NewObject<UAStarGridActor>();
	GridManager->InitializeGrid();
}

void ARTSController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("MouseLeftClick", IE_Pressed, this, &ARTSController::OnLeftClick);
	InputComponent->BindAction("MouseLeftClick", IE_Released, this, &ARTSController::OnLeftReleased);

	InputComponent->BindAction("MouseRightClick", IE_Pressed, this, &ARTSController::OnRightPressed);
	InputComponent->BindAction("MouseRightClick", IE_Released, this, &ARTSController::OnRightReleased);

	InputComponent->BindAxis("RotateCameraRight", this, &ARTSController::CameraRotateX);
	InputComponent->BindAxis("RotateCameraUp", this, &ARTSController::CameraRotateY);

	InputComponent->BindAxis(TEXT("MoveRight"));
	InputComponent->BindAxis(TEXT("MoveForward"));

	InputComponent->BindAxis("Zoom", this, &ARTSController::Zoom);
	InputComponent->BindAxis("ZoomKeyboard");
	InputComponent->BindAction("AnyKey", IE_Pressed, this, &ARTSController::KeyPressed);
	InputComponent->BindAction("AnyKey", IE_Released, this, &ARTSController::KeyReleased);


	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ARTSController::RotateBuilding);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ARTSController::Escape);
}

void ARTSController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CameraMove(0);
	FHitResult Hit;
	// CursorHit(ECC_Visibility, TArray<AActor*>(), Hit);

	
	Zoom(InputComponent->GetAxisValue(FName("ZoomKeyboard")));


	if (bIsLeftClicked && !bIsBuildMode)
	{
		// Interface->LeftHold();
		// if (GameManager)
		// {
		// 	for (IMouseInterface* InputObject : GameManager->GetInputObjects())
		// 	{
		// 		InputObject->LeftHold();
		// 	} 
		// }
	}
}

void ARTSController::KeyPressed(FKey Key)
{
	if (GameManager)
	{
		for (IMouseInterface* InputObject : GameManager->GetInputObjects())
		{
			InputObject->KeyPressed(Key);
		} 
	}
}

void ARTSController::KeyReleased(FKey key)
{
}

void ARTSController::CameraMove(float AxisValue)
{
	const double Forward = InputComponent->GetAxisValue(FName("MoveForward"));
	const double Right = InputComponent->GetAxisValue(FName("MoveRight"));

	const FVector MovementDirection = FVector(Forward, Right, 0.f) * 100.f * GetWorld()->DeltaTimeSeconds *
		CameraMoveSpeed;
	const FVector RotatedVector = GetPawn()->GetActorRotation().RotateVector(MovementDirection);

	GetPawn()->SetActorLocation(RotatedVector + GetPawn()->GetActorLocation());
}

void ARTSController::CameraRotateX(float AxisValue)
{
	if (!bIsRightClicked)
		return;
	GetPawn()->AddActorWorldRotation(
		FRotator(0.f, GetWorld()->DeltaTimeSeconds * AxisValue * CameraRotationSpeed, 0.f));
}

void ARTSController::CameraRotateY(float AxisValue)
{
	if (!bIsRightClicked)
		return;
	SpringArm->AddLocalRotation(FRotator(
		GetWorld()->DeltaTimeSeconds * AxisValue * CameraRotationSpeed, 0.f,
		0.f));
	SpringArm->SetRelativeRotation(FRotator(
		FMath::Clamp(SpringArm->GetRelativeRotation().GetComponentForAxis(EAxis::Y), -70.f, -20.f),
		0.f,
		0.f));
}

void ARTSController::Zoom(float AxisValue)
{
	if (bIsBuildMode && GhostBuilding)
	{
		GhostBuilding->AddActorWorldRotation(FRotator(0, AxisValue * 90, 0));
		return;
	}
	float target = SpringArm->TargetArmLength + AxisValue * -100;
	SpringArm->TargetArmLength = GetSmoothTargetArmLength(AxisValue, target);
}

float ARTSController::GetSmoothTargetArmLength(const float AxisValue, float& CamZoomDestination)
{
	constexpr float MinZoomDistance = 500;
	constexpr float MaxZoomDistance = 3000;


	CamZoomDestination = FMath::Clamp(CamZoomDestination, MinZoomDistance, MaxZoomDistance);

	return FMath::FInterpTo(SpringArm->TargetArmLength, CamZoomDestination, GetWorld()->DeltaTimeSeconds, 50);
}

void ARTSController::OnLeftClick()
{
	bIsLeftClicked = true;
	Interface->LeftPressed();
	if (GameManager)
	{
		TArray<IMouseInterface*> mous = GameManager->GetInputObjects();
		for (IMouseInterface* InputObject : mous)
		{
			InputObject->LeftPressed();
		} 
	}
}

void ARTSController::OnLeftReleased()
{
	bIsLeftClicked = false;
	Interface->LeftReleased();
	if (GameManager)
	{
		for (IMouseInterface* InputObject : GameManager->GetInputObjects())
		{
			InputObject->LeftReleased();
		} 
	}
}

void ARTSController::OnRightPressed()
{
	bIsRightClicked = true;
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this, false);
	SetShowMouseCursor(false);
	if (!SelectedActors.IsEmpty())
	{
		for (ALivingEntity* SelectedActor : SelectedActors)
		{
			AAIController* Cont = Cast<AAIController>(SelectedActor->GetController());
			if (Cont)
			{
				Cont->MoveToLocation(CursorHitLocation);
			}
		} 
	}
}

void ARTSController::OnRightReleased()
{
	bIsRightClicked = false;
	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(this, nullptr, EMouseLockMode::DoNotLock, false, false);
	SetShowMouseCursor(true);
}


bool ARTSController::CursorHit(ECollisionChannel CollisionChannel, TArray<AActor*> IgnoredActors, FHitResult& HitResult, EDrawDebugTrace::Type DebugType)
{
	FVector Loc, Dir;
	FHitResult Hit;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActors(IgnoredActors);
	DeprojectMousePositionToWorld(Loc, Dir);
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), Loc, Dir * 5000 + Loc, UEngineTypes::ConvertToTraceType(CollisionChannel), false, IgnoredActors, EDrawDebugTrace::None, Hit, true);
	// GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(CollisionChannel), false, Hit);
	//GetWorld()->LineTraceSingleByChannel(Hit, Loc, Dir * 5000 + Loc, CollisionChannel, CollisionParameters);
	
	HitResult = Hit;
	return Hit.bBlockingHit;
}

void ARTSController::SetGhostBuilding(TSubclassOf<ABuildingBase> Buildings)
{
	bIsBuildMode = true;
	SwitchLandscapeMaterialParamaters();
	if (Buildings->IsInA(ASplineBuilding::StaticClass()))
	{
		UBuilderBase* BuilderBase = NewObject<UBuilderBase>();
	} else return;
	
	if (GhostBuilding)
	{
		GhostBuilding->Destroy();	
	}
	FActorSpawnParameters ActorSpawnParameters;
	// GhostBuilding = GetWorld()->SpawnActor<ABuildingBase>(Building, CursorHitLocation, BuildingRotation, ActorSpawnParameters);
	// GhostBuilding->OnStartBuilding(this);
	// GhostBuilding->SetActorTickEnabled(true);
}

void ARTSController::ExitBuildMode()
{
	if (GhostBuilding)
	{
		GhostBuilding->Destroy();
	}
	bIsBuildMode = false;
	SwitchLandscapeMaterialParamaters();
}

void ARTSController::StickBuildingToGrid()
{
	if (GhostBuilding)
	{
	// 	int32 Grid_x = CursorHitLocation.X / 100;
	// 	int32 Grid_y = CursorHitLocation.Y / 100;
	// 	FVector Origin, Box;
	// 	FHitResult HitResult;
	// 	FCollisionQueryParams Params;
	// 	TArray<AActor*> ActorsToIgnore;
	// 	Params.AddIgnoredActors(ActorsToIgnore);
	// 	GetWorld()->LineTraceSingleByChannel(HitResult, CursorHitLocation + FVector::UpVector * 500,
	// 	                                     CursorHitLocation + FVector::UpVector * -500, ECC_GameTraceChannel1, Params);
	// 	GhostBuilding->StickToGrid(CursorHitLocation);
	//
	// 	if (HitResult.bBlockingHit)
	// 	{
	// 		/*GhostBuilding->SetActorLocation(FVector(
	// 			Grid_x * 100 - (int)Box.X % 100,
	// 			Grid_y * 100 - (int)Box.Y % 100,
	// 			HitResult.Location.Z));*/
	// 		//GhostBuilding->StickToGrid(CursorHitLocation);
	// 	}
	}
}

void ARTSController::RotateBuilding()
{
	BuildingRotation.Add(0,90,0);
}

void ARTSController::Escape()
{
	if (bIsBuildMode)
	{
		ExitBuildMode();
	}
}

UMaterialInstanceDynamic* ARTSController::CreateMaterialInstance(FLinearColor Color, float Opacity)
{
	if (Material)
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity);
		return DynamicMaterial;
	}
	return nullptr;
}


void ARTSController::SwitchLandscapeMaterialParamaters()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscapeProxy::StaticClass(), Actors);
	LandscapeProxy = Cast<ALandscapeProxy>(Actors[0]);

	LandscapeProxy->SetLandscapeMaterialScalarParameterValue(TEXT("Subgrid"), 1);
	LandscapeProxy->SetLandscapeMaterialScalarParameterValue(TEXT("MainGrid"), 1);
}

void ARTSController::DebugAtWorldPoints(TArray<FVector> grids)
{
	for (FVector Grid : grids)
	{
		DrawDebugBox(GetWorld(), Grid, FVector(20,20,1000), FColor::Orange, false, 100);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, FString::Printf(TEXT("x:%f, y:%f, z:%f"), Grid.X, Grid.Y, Grid.Z));
	}
}
