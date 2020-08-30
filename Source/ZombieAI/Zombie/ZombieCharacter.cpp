#include "ZombieCharacter.h"
#include "ZombieAIController.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

/**
 * Sets the default values for the ZombieCharacter.
 */
AZombieCharacter::AZombieCharacter()
{
	// Load the assets needed for the ZombieCharacter.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>ZombieSkeletalMeshAsset(TEXT("SkeletalMesh'/Game/Models/ZombieJill/jill.jill'"));
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint>ZombieAnimAsset(TEXT("AnimBlueprint'/Game/Blueprints/ZombieAnimBlueprint.ZombieAnimBlueprint'"));

	// Create the ZombieCharacter's skeletal mesh and set it to be the mesh loaded above.
	ZombieSkeletalMesh = GetMesh();
	ZombieSkeletalMesh->SetSkeletalMesh(ZombieSkeletalMeshAsset.Object);
	ZombieSkeletalMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	ZombieSkeletalMesh->SetAnimInstanceClass(ZombieAnimAsset.Object->GeneratedClass);
	ZombieSkeletalMesh->SetupAttachment(RootComponent);

	// Create the DamageCollider and set it so that it extends out about as far as the
	// ZombieCharacter's arm would extend when attacking.
	ZombieDamageCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("ZombieDamageCollider"));
	ZombieDamageCollider->SetRelativeLocation(FVector(70.f, 0.f, 40.f));
	ZombieDamageCollider->SetBoxExtent(FVector(30.f, 30.f, 20.f));
	ZombieDamageCollider->SetGenerateOverlapEvents(true);
	ZombieDamageCollider->SetCollisionProfileName(TEXT("Trigger"));
	ZombieDamageCollider->SetupAttachment(RootComponent);

	// Helps orient the PatrolCharacter so that when it walks it doesn't face the
	// Waypoint but instead the direction that it's walking.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 200.0f, 0.0f);

	// Set the default AIController of the class.
	AIControllerClass = AZombieAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

/**
 * Called when the game starts.
 */
void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set the starting location of the ZombieCharacter.
	StartLocation = GetActorLocation();
}

/**
 * Called to make the ZombieCharacter take damage and check to see if the
 * ZombieCharacter needs to die.
 */
void AZombieCharacter::Hit(float Damage)
{
	// Take the damage to apply from the ZombieCharacter's damage.
	Health -= Damage;

	// If the ZombieCharacter's `Health` is at or below zero then we have take care of disabling the
	// ZombieAIController.
	if (Health <= 0.f)
	{
		// Have the ZombieAIController un possess the ZombieCharacter and then destroy the
		// ZombieAIController so it doesn't give any more input to the ZombieCharacter.
		AAIController* ZombieAIController = Cast<AAIController>(GetController());
		if (ZombieAIController == nullptr) return;
		ZombieAIController->UnPossess();
		ZombieAIController->Destroy();

		// Put the ZombieCharacter in the `DEAD` state so that the animation blueprint will play
		// the zombie dying animation.
		ToDeadState();

		// Now we set a timer for the length of the dying animation to make sure that if we have to
		// destroy the ZombieCharacter, we don't do it until the animation has finished playing.
		UWorld* World = GetWorld();
		if (World == nullptr) return;
		World->GetTimerManager().SetTimer(DeathAnimationTimer, this, &AZombieCharacter::AfterDeathAnimationFinished, DyingAnimationLengthInSeconds);
	}
}

/**
 * Called after the death animation finishes playing.
 */
void AZombieCharacter::AfterDeathAnimationFinished()
{
	// Now that the dying animation has finished playing we can see if we need to Destroy
	// the ZombieCharacter.
	if (SecondsAfterDeathBeforeDestroy == 0.f)
	{
		// The ZombieCharacter should be destroyed immediately so we don't need to set a
		// timer.
		Destroy();
	}
	else if (SecondsAfterDeathBeforeDestroy > 0.f)
	{
		// We have to wait some time before the ZombieCharacter should be Destroyed so we
		// set a timer that runs the method to destroy the ZombieCharacter.
		SetLifeSpan(SecondsAfterDeathBeforeDestroy);
	}
}

/**
 * Called to transition the ZombieCharacter to the IDLE state.
 */
void AZombieCharacter::ToIdleState()
{
	PreviousState = State;
	State = ZombieStates::IDLE;
}

/**
 * Called to transition the ZombieCharacter to the ROAM state.
 */
void AZombieCharacter::ToRoamState()
{
	PreviousState = State;
	State = ZombieStates::ROAM;

	UCharacterMovementComponent* ZombieMovement = GetCharacterMovement();
	if (ZombieMovement != nullptr)
	{
		ZombieMovement->MaxWalkSpeed = RoamSpeed;
	}
}

/**
 * Called to transition the ZombieCharacter to the CHASE state.
 */
void AZombieCharacter::ToChaseState()
{
	PreviousState = State;
	State = ZombieStates::CHASE;

	UCharacterMovementComponent* ZombieMovement = GetCharacterMovement();
	if (ZombieMovement != nullptr)
	{
		ZombieMovement->MaxWalkSpeed = ChaseSpeed;
	}
}

/**
 * Called to transition the ZombieCharacter to the ATTACK state.
 */
void AZombieCharacter::ToAttackState()
{
	PreviousState = State;
	State = ZombieStates::ATTACK;
}

/**
 * Called to transition the ZombieCharacter to the DEAD state.
 */
void AZombieCharacter::ToDeadState()
{
	PreviousState = State;
	State = ZombieStates::DEAD;
}