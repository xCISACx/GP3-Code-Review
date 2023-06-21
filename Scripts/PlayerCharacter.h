// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Items/ItemTypes.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UBoxComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;

class UTankComponent;
class AGP3GameModeBase;

UCLASS()
class GP3_TEAM4_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void GotCollectable(ECollectableType CollectableType, int CollectValue, bool& bIsCollectSuccess);

	UFUNCTION(BlueprintCallable)
	EActionState GetState() { return ActionState; }

	UFUNCTION(BlueprintCallable)
	void SetAbilityLevel(ECollectableType Type);

	UFUNCTION(BlueprintCallable)
	EAbilityLevel GetStormLevel() { return CurrentStormLevel; }

	UFUNCTION(BlueprintCallable)
	EAbilityLevel GetWindLevel() { return CurrentWindLevel; }

	UFUNCTION(BlueprintCallable)
	EAbilityLevel GetFireLevel() { return CurrentFireLevel; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bIsInCombat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DefaultMoveSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StrafeMoveSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float CombatExpirationTime = 1.f;

	UPROPERTY()
	FTimerHandle CombatTimerHandle;

	UFUNCTION()
	void StartCombatTimer();
	
	UFUNCTION()
	void StopCombatTimer();

	UFUNCTION()
	void OnCombatTimerExpired();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bCanDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float DashCooldown = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float DashDuration = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bIsDashing = false;

	UPROPERTY()
	FTimerHandle DashCooldownTimerHandle;

	UFUNCTION()
	void StartDashCooldownTimer();
	
	UFUNCTION()
	void StopDashCooldownTimer();

	UFUNCTION()
	void OnDashCooldownTimerExpired();

	UPROPERTY()
	FTimerHandle DashTimerHandle;

	UFUNCTION()
	void StartDashTimer();
	
	UFUNCTION()
	void StopDashTimer();

	UFUNCTION()
	void OnDashTimerExpired();

	UFUNCTION(BlueprintCallable)
	void ResetDash();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called for movement input 
	void Move(const FInputActionValue& Value);
	void Dash();
	void ResetMovementVector();

	// Called for looking input 
	void Look(const FInputActionValue& Value);

	void Attack(const FInputActionValue& Value);

	//Elements Input
	void EarthTankTranfer(const FInputActionValue& Value);

	void WindTankTranfer(const FInputActionValue& Value);

	void FireTankTranfer(const FInputActionValue& Value);

	void TankDrainPressed(const FInputActionValue& Value);

	void QuestTankTransfer(const FInputActionValue& Value);

	//Combat Input
	void AttackSequence();

	UFUNCTION(BlueprintCallable)
	void CanAttack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();
	UFUNCTION(BlueprintCallable)
	void Hitting();
	UFUNCTION(BlueprintCallable)
	void Dashing();

	UFUNCTION(BlueprintCallable)
	void HeavyAttack();

	UFUNCTION(BlueprintCallable)
	float GetEarthCollectPercengate() {return (float)EarthCollectCount/ (float)EarthMaxAmmount;}
	UFUNCTION(BlueprintCallable)
	float GetWindCollectPercengate() {return (float)FireCollectCount / (float)FireMaxAmmount;}
	UFUNCTION(BlueprintCallable)
	float GetFireCollectPercengate() { return (float)WindCollectCount / (float)WindMaxAmmount; }



private:

	void OverloadedDrain();

	void HandleQuestTank(ECollectableType Type, int Value ,bool bTankFull);

	int GetPocketAmount(ECollectableType Type);

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess= "true"))
	USpringArmComponent* CameraBoom;
	 
	UPROPERTY(EditAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UBoxComponent* AttackCollision;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* GroundMappingContext;

	UPROPERTY(EditAnywhere,  Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere,  Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* FireTransferAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* WindTransferAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StormTransferAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* TankDrainAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* QuestInteractAction;
	


	UPROPERTY(EditAnywhere, Category = Input)
	float DashDistance = 1000.f;

	UPROPERTY(EditAnywhere, Category = Input)
	FVector2D MoveAxisVector;
	
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;

	UPROPERTY(BlueprintReadWrite, meta =(AllowPrivateAccess ="true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UTankComponent* TankComponent;

	UPROPERTY(EditDefaultsOnly, Category = Collect)
	int EarthMaxAmmount = 100;
	UPROPERTY(EditDefaultsOnly, Category = Collect)
	int FireMaxAmmount = 100;
	UPROPERTY(EditDefaultsOnly, Category = Collect)
	int WindMaxAmmount = 100;
	UPROPERTY(EditDefaultsOnly, Category = Collect)
	int TankFlowRate = 1;
	UPROPERTY(EditDefaultsOnly, Category = Collect)
	int TankDrainRate = 1;
	UPROPERTY(EditDefaultsOnly, Category = Collect)
	float DrainTimer = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = Collect)
	float MaximumInteractionRange = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta= (AllowPrivateAccess ="true"))
	EAbilityLevel CurrentFireLevel = EAbilityLevel::EAL_Level0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta= (AllowPrivateAccess ="true"))
	EAbilityLevel CurrentStormLevel = EAbilityLevel::EAL_Level0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta= (AllowPrivateAccess ="true"))
	EAbilityLevel CurrentWindLevel = EAbilityLevel::EAL_Level0;

	int ComboCount = 0;
	int EarthCollectCount = 0;
	int FireCollectCount = 0;
	int WindCollectCount = 0;
	float Timer = 0.0f;
	bool bIsOverloaded = false;

	AGP3GameModeBase* GameMode;
};
