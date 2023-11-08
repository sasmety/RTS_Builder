#include "RTS_Builder/BuildingSystem/Public/BuildingBase.h"

#include "LandscapeDataAccess.h"
#include "MathUtil.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RTS_Builder/RTSController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#define Print_Vector(Vector, Comment, Color)                         \
{                                          \
GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1, Color, FString::Printf(TEXT("%s, %f, %f, %f"), Comment, Vector.X, Vector.Y, Vector.Z)); \
}


ABuildingBase::ABuildingBase()
{
	RootComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Root"));;
	RootComponent = RootComp;
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Grid"));
	ProceduralMeshComponent->SetupAttachment(GetRootComponent());

	Vectors.Add(FVector(0, 0, 0));
	Vectors.Add(FVector(0, 100, 0));
	Vectors.Add(FVector(100, 100, 0));
	Vectors.Add(FVector(100, 0, 0));
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);
	Triangles.Add(2);
	UV.Add(FVector2D(0, 0));
	UV.Add(FVector2D(0, 1));
	UV.Add(FVector2D(1, 0));
	UV.Add(FVector2D(1, 1));
	Normals.Add(FVector(0, 0, 1));
	Normals.Add(FVector(0, 0, 1));
	Normals.Add(FVector(0, 0, 1));
	Normals.Add(FVector(0, 0, 1));

	Colors.SetNum(Vectors.Num());
	Tangents.SetNum(Vectors.Num());
}

void ABuildingBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ABuildingBase::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscapeProxy::StaticClass(), Actors);
	TArray<TOptional<float>> locs;
	TMap<int32, float> finals;
	LandscapeProxy = Cast<ALandscapeProxy>(Actors[0]);
	TOptional<float> loc1 = (LandscapeProxy->GetHeightAtLocation(FVector(0,0,0), EHeightfieldSource::Complex));
	TOptional<float> loc2 = (LandscapeProxy->GetHeightAtLocation(FVector(0,100,0), EHeightfieldSource::Complex));
	TOptional<float> loc3 = (LandscapeProxy->GetHeightAtLocation(FVector(100,0,0), EHeightfieldSource::Complex));
	TOptional<float> loc4 = (LandscapeProxy->GetHeightAtLocation(FVector(-100,0,0), EHeightfieldSource::Complex));
	TOptional<float> loc5 = (LandscapeProxy->GetHeightAtLocation(FVector(0,-100,0), EHeightfieldSource::Complex));
	TArray<float> floats;
	int32 x,y;
	LandscapeProxy->GetHeightValues(x,y, floats);
	if (true)
	{
		
	}
	
	
}

//TODO Serious optimization needed!!
void ABuildingBase::Tick(float DeltaTime)
{
}

void ABuildingBase::OnClick(ARTSController* Ctrl)
{
}

float ABuildingBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ABuildingBase::OnStartBuilding(ARTSController* Ctrl)
{
}

void ABuildingBase::Repair()
{
}

void ABuildingBase::DrawGrid(int32 x, int32 y)
{
}

void ABuildingBase::UpdateGrid()
{
}

void ABuildingBase::RemoveGrid()
{
	ProceduralMeshComponent->ClearAllMeshSections();
}

void ABuildingBase::OnRender()
{
}

void ABuildingBase::Update(float DeltaSeconds, FHitResult& Hit)
{
	
}

void ABuildingBase::UpdateDuringBuilding(FVector Endloc, FHitResult& Hit)
{
	HitUnderCursor = Hit;
}

void ABuildingBase::StickToGrid(FVector& Location)
{
}

bool ABuildingBase::CanBeBuilt()
{
	return true;
}

void ABuildingBase::SetGhostMaterial()
{
}

void ABuildingBase::SetDefaultMaterial()
{
}

void ABuildingBase::AssignAssets()
{
}

void ABuildingBase::DestructBuilding(FHitResult& HitData, float size)
{
	if(HitData.GetActor())
		HitData.GetActor()->Destroy();
}


APlayerController* ABuildingBase::GetPlayerController()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GEngine->GetWorld(), 0);
	return PlayerController;
}
