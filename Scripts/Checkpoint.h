// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

UCLASS()
class GP3_TEAM4_API ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void OnBoxTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                              const FHitResult& SweepResult);
	
	ACheckpoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float ZOffset = 50.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UBoxComponent* TriggerBoxComponent;
};
