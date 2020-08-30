#include "ZombieAnimInstance.h"
#include "ZombieCharacter.h"
#include "../Player/PlayerCharacter.h"
#include "Math/Rotator.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

/**
 * Used by the animation blueprint to update the animation properties above
 * and decide what animations to play.
 */
void UZombieAnimInstance::UpdateAnimationProperties()
{
	// Try to get the Pawn being animated and return if a nullptr.
	APawn* ZombiePawn = TryGetPawnOwner();
	if (ZombiePawn == nullptr) return;

	// Try to cast the Pawn to our ZombieCharacter since that's the only
	// thing we want to animate.
	AZombieCharacter* ZombieCharacter = Cast<AZombieCharacter>(ZombiePawn);
	if (ZombieCharacter == nullptr) return;

	// Set the variables that are dependent on states.
	bIsRoaming = ZombieCharacter->State == ZombieStates::ROAM;
	bIsChasing = ZombieCharacter->State == ZombieStates::CHASE;
	bIsAttacking = ZombieCharacter->State == ZombieStates::ATTACK;
	bIsDying = ZombieCharacter->State == ZombieStates::DEAD;
}
