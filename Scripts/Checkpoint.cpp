// Fill out your copyright notice in the Description page of Project Settings.


#include "Checkpoint/Checkpoint.h"

#include "GP3GameInstance.h"
#include "Characters/PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "GameMode/GP3GameModeBase.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ACheckpoint::ACheckpoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Set up the box trigger for player overlap
	TriggerBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBoxComponent;

	// Bind the overlap event
	TriggerBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnBoxTriggerBeginOverlap);

}

// Called when the game starts or when spawned
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACheckpoint::OnBoxTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* PlayerPawn = Cast<APawn>(OtherActor);

	if (PlayerPawn)
	{
		APlayerController* PlayerController = Cast<APlayerController>(PlayerPawn->GetController());

		if (PlayerController)
		{
			// Set the new spawn location and offset its Z axis so the player spawns above the terrain
			FVector NewSpawnLocation = GetActorLocation() + FVector(0.f, 0.f, ZOffset);
			Cast<UGP3GameInstance>(GetGameInstance())->SetCurrentCheckpointLocation(NewSpawnLocation);

			// Debug saved location to screen
			FVector Location = Cast<UGP3GameInstance>(GetGameInstance())->GetCurrentCheckpointLocation();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Location.ToString());
		}
	}
}