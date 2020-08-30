#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ZombieAnimInstance.generated.h"

/**
 * Manages the booleans needed by the animation blueprint to decide what
 * animation needs to be run.
 */
UCLASS()
class ZOMBIEAI_API UZombieAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// Indicates whether the ZombieCharacter is roaming or not.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsRoaming;

	// Indicates whether the ZombieCharacter is chasing or not.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsChasing;

	// Indicates whether the ZombieCharacter is attacking or not.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsAttacking;

	// Indicates whether the ZombieCharacter is dying or not.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsDying;

	// Used by the animation blueprint to update the animation properties above
	// and decide what animations to play.
	UFUNCTION(BlueprintCallable, Category = "UpdateAnimationProperties")
	void UpdateAnimationProperties();
};
