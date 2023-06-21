// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Characters/PlayerCharacter.h"
#include "GP3GameInstance.generated.h"


/**
 * 
 */
UCLASS()
class GP3_TEAM4_API UGP3GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
	FVector GetCurrentCheckpointLocation() const { return CurrentCheckpointLocation; }

	UFUNCTION(BlueprintCallable)
	void SetCurrentCheckpointLocation(FVector Location) { CurrentCheckpointLocation = Location; }

private:

	APlayerCharacter* Player;
	
	FVector CurrentCheckpointLocation;
	
};
