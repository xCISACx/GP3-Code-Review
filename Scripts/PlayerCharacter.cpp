// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"
#include "Characters/Components/TankComponent.h"
#include "GameMode/GP3GameModeBase.h"

#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "StaticMeshAttributes.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement=  GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	}
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom);

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Attack Area"));
	AttackCollision->SetupAttachment(RootComponent);

}

void APlayerCharacter::StartCombatTimer()
{
	GetWorldTimerManager().SetTimer(CombatTimerHandle, this, &APlayerCharacter::OnCombatTimerExpired, CombatExpirationTime, false);

	bIsInCombat = true;
}

void APlayerCharacter::StopCombatTimer()
{
	GetWorldTimerManager().ClearTimer(CombatTimerHandle);
	
	bIsInCombat = false;
}

void APlayerCharacter::OnCombatTimerExpired()
{
	bIsInCombat = false;
	GetCharacterMovement()->MaxWalkSpeed = DefaultMoveSpeed;
}

void APlayerCharacter::StartDashCooldownTimer()
{
	GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, this, &APlayerCharacter::OnDashCooldownTimerExpired, DashCooldown, false);
	bCanDash = false;
}

void APlayerCharacter::StopDashCooldownTimer()
{
	GetWorldTimerManager().ClearTimer(DashCooldownTimerHandle);
	
	bCanDash = true;
}

void APlayerCharacter::OnDashCooldownTimerExpired()
{
	bCanDash = true;
}

void APlayerCharacter::StartDashTimer()
{
	GetWorldTimerManager().SetTimer(DashTimerHandle, this, &APlayerCharacter::OnDashTimerExpired, DashDuration, false);
	GetCharacterMovement()->GroundFriction = 0.f;
	bIsDashing = true;
	bCanMove = false;
}

void APlayerCharacter::StopDashTimer()
{
	GetWorldTimerManager().ClearTimer(DashTimerHandle);
}

void APlayerCharacter::OnDashTimerExpired()
{
	ResetDash();
}

void APlayerCharacter::ResetDash()
{
	GetCharacterMovement()->GroundFriction = 8.f;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
	bIsDashing = false;
	bCanMove = true;
	StopDashCooldownTimer();
	StartDashCooldownTimer();
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();

			Subsystem->AddMappingContext(GroundMappingContext, 0);
		}
	}

	TankComponent = FindComponentByClass<UTankComponent>();
	GameMode = Cast<AGP3GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsOverloaded)
	{
		OverloadedDrain();
	}
}

#pragma region INPUT

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked< UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Movement Bindings
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerCharacter::ResetMovementVector);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &APlayerCharacter::Dash);
		
		//Looking Bindings
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Attack);
		
		EnhancedInputComponent->BindAction(FireTransferAction, ETriggerEvent::Triggered, this, &APlayerCharacter::FireTankTranfer);
		EnhancedInputComponent->BindAction(WindTransferAction, ETriggerEvent::Triggered, this, &APlayerCharacter::WindTankTranfer);
		EnhancedInputComponent->BindAction(StormTransferAction, ETriggerEvent::Triggered, this, &APlayerCharacter::EarthTankTranfer);
		EnhancedInputComponent->BindAction(TankDrainAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TankDrainPressed);
		EnhancedInputComponent->BindAction(QuestInteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::QuestTankTransfer);
	}

}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	MoveAxisVector = Value.Get<FVector2D>();

	if (!bCanMove) return;

	if (GetController())
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (bIsInCombat)
		{
			bUseControllerRotationPitch = false;
			bUseControllerRotationYaw = true;
			bUseControllerRotationRoll = true;

			AddMovementInput(ForwardDirection, MoveAxisVector.Y);
			AddMovementInput(RightDirection, MoveAxisVector.X);
		}
		else
		{
			bUseControllerRotationPitch = false;
			bUseControllerRotationYaw = false;
			bUseControllerRotationRoll = false;

			AddMovementInput(ForwardDirection, MoveAxisVector.Y);
			AddMovementInput(RightDirection, MoveAxisVector.X);
		}
	}
}

void APlayerCharacter::ResetMovementVector()
{
	MoveAxisVector = FVector2D::ZeroVector;
}

void APlayerCharacter::Dash()
{
	FVector DashDirection;

	// if tne player is not in combat and inputting a direction, we make them dash towards their forward vctor
	
	if (MoveAxisVector.Length() > 0 && !bIsInCombat)
	{
		DashDirection = UKismetMathLibrary::GetForwardVector(GetActorRotation());
	}
	
	// if tne player is in combat and inputting a direction, we make them dash towards the transformed input direction
	else if (MoveAxisVector.Length() > 0 && bIsInCombat)
	{
		DashDirection = FVector(MoveAxisVector.Y, MoveAxisVector.X, 0.f);
		const auto TransformedDirection = UKismetMathLibrary::TransformDirection(GetActorTransform(), DashDirection);
		
		DashDirection = TransformedDirection;
		DashDirection.Z = 0;
	}
	
	// otherwise, the player dodges backwards
	else
	{
		DashDirection = -GetActorForwardVector();
		DashDirection.Normalize();
	}
	
	FVector DashVelocity = DashDirection * DashDistance;

	if (bCanDash)
	{
		// start the dash timer so the dashing state is reset when it finishes
		StopDashTimer();
		StartDashTimer();
		
		GetCharacterMovement()->GroundFriction = 0.f;
		bIsDashing = true;
		bCanMove = false;
		
		GetCharacterMovement()->AddImpulse(DashVelocity, true);

		// ignore enemies while dashing
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

		// start the dash cooldown timer so the player can dash again when it finishes
		StopDashCooldownTimer();
		StartDashCooldownTimer();
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController())
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::Attack(const FInputActionValue& Value)
{
	if (!bIsInCombat) bIsInCombat = true;

	GetCharacterMovement()->MaxWalkSpeed = StrafeMoveSpeed;

	// make player face camera forward when attack starts
	FVector CameraForwardVector = FollowCamera->GetForwardVector();

	FRotator TargetRotation = FRotator(0.f, CameraForwardVector.Rotation().Yaw, 0.f);
	SetActorRotation(TargetRotation);
	
	if (ActionState == EActionState::EAS_Unoccupied || ActionState == EActionState::EAS_CanAttack)
	{
		AttackSequence();
		if (TankComponent)
		{
			TankComponent->DrainEssence(TankDrainRate, bIsOverloaded);
			SetAbilityLevel(ECollectableType::ECT_Earth);
			SetAbilityLevel(ECollectableType::ECT_Wind);
			SetAbilityLevel(ECollectableType::ECT_Fire);
		}
		ActionState = EActionState::EAS_Attacking;
	}
}

//NAME CHANGED IT"S STORM ELEMENT NOW
void APlayerCharacter::EarthTankTranfer(const FInputActionValue& Value)
{
	if (!TankComponent) return;
	bool bTankFull = false;
	if (EarthCollectCount == 0) return;
	TankComponent->AddEssence(ECollectableType::ECT_Earth, TankFlowRate, bTankFull);
	SetAbilityLevel(ECollectableType::ECT_Earth);
	if (CurrentStormLevel == EAbilityLevel::EAL_Level1)
	{
	UE_LOG(LogTemp, Warning, TEXT("CURRENT Storm LEVEL IS: levl1"));
	}
	if (CurrentStormLevel == EAbilityLevel::EAL_Level2)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENT Storm LEVEL IS: levl2"));
	}
	if (CurrentStormLevel == EAbilityLevel::EAL_Level3)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENT Storm LEVEL IS: levl3"));
	}
	if (!bTankFull)
	{
		EarthCollectCount -= TankFlowRate;
	}
}

void APlayerCharacter::WindTankTranfer(const FInputActionValue& Value)
{
	if (!TankComponent) return;
	bool bTankFull = false;
	if (WindCollectCount == 0) return;
	TankComponent->AddEssence(ECollectableType::ECT_Wind, TankFlowRate, bTankFull);
	SetAbilityLevel(ECollectableType::ECT_Wind);
	if (CurrentWindLevel == EAbilityLevel::EAL_Level1)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENT Wind LEVEL IS: levl1"));
	}
	if (CurrentWindLevel == EAbilityLevel::EAL_Level2)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENT Wind LEVEL IS: levl2"));
	}
	if (CurrentWindLevel == EAbilityLevel::EAL_Level3)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENT Wind LEVEL IS: levl3"));
	}
	if (!bTankFull)
	{

		WindCollectCount -= TankFlowRate;
	}
}

void APlayerCharacter::FireTankTranfer(const FInputActionValue& Value)
{
	if (!TankComponent) return;
	bool bTankFull = false;
	if (FireCollectCount == 0) return;
	TankComponent->AddEssence(ECollectableType::ECT_Fire, TankFlowRate, bTankFull);
	SetAbilityLevel(ECollectableType::ECT_Fire);
	if (CurrentFireLevel == EAbilityLevel::EAL_Level1)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENT FIRE LEVEL IS: levl1"));
	}
	if (CurrentFireLevel == EAbilityLevel::EAL_Level2)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENT FIRE LEVEL IS: levl2"));
	}
	if (CurrentFireLevel == EAbilityLevel::EAL_Level3)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENT FIRE LEVEL IS: levl3"));
	}
	if (!bTankFull)
	{
		FireCollectCount -= TankFlowRate;
	}
}


void APlayerCharacter::TankDrainPressed(const FInputActionValue& Value)
{
	TankComponent->DrainTank(1);
	SetAbilityLevel(ECollectableType::ECT_Earth);
	SetAbilityLevel(ECollectableType::ECT_Wind);
	SetAbilityLevel(ECollectableType::ECT_Fire);
}

void APlayerCharacter::QuestTankTransfer(const FInputActionValue& Value)
{
	float Distance;
	bool bIsTankFull = false;
	AActor* QuestTank = GameMode->GetClosestActor(Distance);

	if (QuestTank && Distance < MaximumInteractionRange)
	{
		ECollectableType AskedType;
		UTankComponent* QuestTankComponent = QuestTank->FindComponentByClass<UTankComponent>();
		if (QuestTankComponent)
		{
			AskedType = QuestTankComponent->GetAskedType();
			if (GetPocketAmount(AskedType) > 0)
			{
				QuestTankComponent->AddSelectedType(AskedType, TankFlowRate, bIsTankFull);
				HandleQuestTank(AskedType, TankFlowRate , bIsTankFull);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest tank has no tank Component"));
		}
		
	}
}

#pragma endregion


void APlayerCharacter::GotCollectable(ECollectableType CollectableType, int CollectValue, bool& bIsCollectSuccess)
{

	switch (CollectableType)
	{
	case ECollectableType::ECT_Earth:
		if (EarthCollectCount < EarthMaxAmmount)
		{
			EarthCollectCount += CollectValue;
			if (EarthCollectCount >= EarthMaxAmmount)
			{
				EarthCollectCount = EarthMaxAmmount;
			}
			bIsCollectSuccess = true;
		}
		break;
	case ECollectableType::ECT_Wind:
		if (WindCollectCount < WindMaxAmmount)
		{
			WindCollectCount += CollectValue;
			if (WindCollectCount >= WindMaxAmmount)
			{
				WindCollectCount = WindMaxAmmount;
			}
			bIsCollectSuccess = true;
		}
		break;
	case ECollectableType::ECT_Fire:
		if (FireCollectCount < FireMaxAmmount)
		{
			FireCollectCount += CollectValue;
			if (FireCollectCount >= FireMaxAmmount)
			{
				FireCollectCount = FireMaxAmmount;
			}
			bIsCollectSuccess = true;
		}
		break;
	default:
		bIsCollectSuccess = false;
		break;
	}
}

void APlayerCharacter::AttackSequence()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		switch (ComboCount)
		{
			case 0:
				AnimInstance->Montage_Play(AttackMontage);
				break;
			case 1:
				AnimInstance->Montage_JumpToSection("Attack2");
				break;
			case 2:
				AnimInstance->Montage_JumpToSection("Attack3");
				break;
			default:
				break;
		}
	}
}

void APlayerCharacter::CanAttack()
{
	ComboCount++;
	ActionState = EActionState::EAS_CanAttack;
}

void APlayerCharacter::Hitting()
{
	ActionState = EActionState::EAS_HitEnemies;
}

void APlayerCharacter::Dashing()
{
	ActionState = EActionState::EAS_Dashing;
}

void APlayerCharacter::HeavyAttack()
{
	ActionState = EActionState::EAS_HeavyAttack;
}

void APlayerCharacter::AttackEnd()
{
	OnCombatTimerExpired();
	ActionState = EActionState::EAS_Unoccupied;
	ComboCount = 0;
}

void APlayerCharacter::SetAbilityLevel(ECollectableType Type)
{
	if (!TankComponent) return;
	switch (Type)
	{
	case ECollectableType::ECT_Earth:
		CurrentStormLevel = TankComponent->CheckTypeLevel(ECollectableType::ECT_Earth);
		break;
	case ECollectableType::ECT_Wind:
		CurrentWindLevel = TankComponent->CheckTypeLevel(ECollectableType::ECT_Wind);
		break;
	case ECollectableType::ECT_Fire:
		CurrentFireLevel = TankComponent->CheckTypeLevel(ECollectableType::ECT_Fire);
		break;
	default:
		break;
	}
}

void APlayerCharacter::OverloadedDrain()
{
	if (!TankComponent) return;
	Timer += GetWorld()->GetDeltaSeconds();
	if (Timer >= DrainTimer)
	{
		TankComponent->DrainEssence(TankDrainRate, bIsOverloaded);
		Timer = 0;
	}
}

void APlayerCharacter::HandleQuestTank(ECollectableType Type, int Value, bool bTankFull)
{
	if (bTankFull) return;
	switch (Type)
	{
	case ECollectableType::ECT_Earth:
		EarthCollectCount-= Value;
		break;
	case ECollectableType::ECT_Wind:
		WindCollectCount -= Value;
		break;
	case ECollectableType::ECT_Fire:
		FireCollectCount -= Value;
		break;
	default:
		break;
	}
}

int APlayerCharacter::GetPocketAmount(ECollectableType Type)
{
	switch (Type)
	{
	case ECollectableType::ECT_Earth:
		return EarthCollectCount;
		break;
	case ECollectableType::ECT_Wind:
		return WindCollectCount;
		break;
	case ECollectableType::ECT_Fire:
		return FireCollectCount;
		break;
	default:
		return 0;
		break;
	}
}




