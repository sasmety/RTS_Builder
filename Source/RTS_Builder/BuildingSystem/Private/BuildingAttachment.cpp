// Fill out your copyright notice in the Description page of Project Settings.


#include "RTS_Builder/BuildingSystem/Public/BuildingAttachment.h"
#include "RTS_Builder/BuildingSystem/Public/BuildingFixedSize.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UBuildingAttachment::UBuildingAttachment()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBuildingAttachment::BeginPlay()
{
	Super::BeginPlay();
	SplitSocketNames();
	/*ABuildingFixedSize* Owner = Cast<ABuildingFixedSize>(GetOwner());
	if (Owner)
	{
		for (FName SocketName : Owner->MeshComponent->GetAllSocketNames())
		{
			TArray<FString> SplitStrings;
			
			SocketName.ToString().ParseIntoArray(SplitStrings, TEXT("."), true);
			TArray<TPair<FTransform, bool>> s;
			TPair<FTransform, bool> a;
			a.Key = Owner->MeshComponent->GetSocketTransform(SocketName);
			a.Value = false;
			s.Add(a);
			if (Attachments.Contains(SplitStrings[0]))
			{
				Attachments.Find(SplitStrings[0])->Add(a);				
			}
			else
			{
				Attachments.Add(SplitStrings[0], s);	
			}
		}
		TArray<FString> a;
		Attachments.GetKeys(a);
		
		for (FString A : a)
		{
			TArray<TPair<FTransform, bool>> x = Attachments.FindRef(A);
			int32 index = 0;
			for (TPair<FTransform, bool> X : x)
			{
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Yellow, FString::Printf(TEXT("i:%d, Location: %f,%f,%f, Rotation: %f,%f,%f, Scale: %f,%f,%f, isEmpty:%d"),
					index,
					X.Key.GetLocation().X, X.Key.GetLocation().Y, X.Key.GetLocation().Z,
					X.Key.GetRotation().X, X.Key.GetRotation().Y, X.Key.GetRotation().Z,
					X.Key.GetScale3D().X, X.Key.GetScale3D().Y, X.Key.GetScale3D().Z,
					X.Value));
				index++;
			} 
		}
		for (FString String : a)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::White, String);
		} 
	}

	Owner = Cast<ABuildingFixedSize>(GetOwner());


	// ...*/
}

void UBuildingAttachment::SplitSocketNames()
{
	ABuildingFixedSize* Owner = Cast<ABuildingFixedSize>(GetOwner());
	if (Owner)
	{
		for (FName SocketName : Owner->MeshComponent->GetAllSocketNames())
		{
			TArray<FString> SplitStrings;
			SocketName.ToString().ParseIntoArray(SplitStrings, TEXT("."), true);

			FSocketData* NewSocketData = new FSocketData();
			NewSocketData->Socket = SocketName;
			NewSocketData->Transform = Owner->MeshComponent->GetSocketTransform(SocketName);
			NewSocketData->bIsEmpty = true;
			NewSocketData->CompatibleActors = SplitStrings;
			Sockets.Add(NewSocketData);
		}


		for (FSocketData* Socket : Sockets)
		{
			for (FString CompatibleActor : Socket->CompatibleActors)
			{
				if (Components.Contains(CompatibleActor))
				{
					Components[CompatibleActor].Add(Socket);
				}
				else
				{
					TArray<FSocketData*> Arr;
					Arr.Add(Socket);
					Components.Add(CompatibleActor, Arr);
				}
			}
		}
	}
}

FSocketData* UBuildingAttachment::FindSocket(FString ClassName, FVector& CursorLocation)
{
	if (Components.Contains(ClassName))
	{
		TArray<TPair<double, FSocketData*>> AvailableSockets;

		for (FSocketData* Socket : Components[ClassName])
		{
			if (Socket->bIsEmpty)
			{
				TPair<double, FSocketData*> x;
				x.Key = UKismetMathLibrary::Vector_DistanceSquared(Socket->Transform.GetLocation(), CursorLocation);
				x.Value = Socket;
				AvailableSockets.Add(x);
			}
		}
		AvailableSockets.Sort([](const TPair<double, FSocketData*>& A, const TPair<double, FSocketData*>& B)
		{
			return A.Key < B.Key;
		});
		if (AvailableSockets.Num() > 0)
		{
			return AvailableSockets[0].Value;
		}
	}
	return false;
}


// Called every frame
void UBuildingAttachment::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	int index = 0;
	for (const TTuple<FString, TArray<FSocketData*>>& Component : Components)
	{
		FString tag = Component.Key;
		for (FSocketData* Value : Component.Value)
		{
			GEngine->AddOnScreenDebugMessage(166 + index, 10, FColor::Emerald, FString::Printf(TEXT("Component: %s, Socket: %s, BIsEmpty: %d"), *tag, *Value->Socket.ToString(), Value->bIsEmpty));
			index++;
		} 
	} 
	GEngine->AddOnScreenDebugMessage(200, 10, FColor::Emerald, FString::Printf(TEXT("---------------------------")));
}
