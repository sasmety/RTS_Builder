#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "People.generated.h"

UCLASS()
class RTS_BUILDER_API APeople : public ACharacter
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APeople();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Born();
	void Die();
	void Attack();
	void Move();
	void AssignWork();

	FString FirstName;
	FString LastName;
	uint16 Age;
	float Health;
	
};
