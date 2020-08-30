#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class USkeletalMeshComponent;

/**
 * The PlayerCharacter is the main player of the game.
 */
UCLASS()
class ZOMBIEAI_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties.
	APlayerCharacter();

	// The skeletal mesh of the PlayerCharacter's body.
	UPROPERTY(VisibleDefaultsOnly)
	USkeletalMeshComponent* PlayerSkeletalMesh;

	// The skeletal mesh of the PlayerCharacter's gun.
	UPROPERTY(VisibleDefaultsOnly)
	USkeletalMeshComponent* GunSkeletalMesh;

	// The location on the gun where the BulletActors should spawn from.
	UPROPERTY(VisibleDefaultsOnly)
	class USceneComponent* BulletSpawnLocation;

	// The first-person camera of the PlayerCharacter.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* PlayerCamera;

	// Used to register sense for the PlayerCharacter that are detectable by the ZombieCharacter.
	UPROPERTY(VisibleDefaultsOnly)
	class UAIPerceptionStimuliSourceComponent* PlayerStimuliSource;

	// The gun's offset from the PlayerCharacter's location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	FVector GunOffset;

	// The AnimMontage to play when the gun is fired. This is set automatically
	// in the constructor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	class UAnimMontage* GunFireAnimation;

	// The amount of damage each shot of the PlayerCharacter's gun does.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	float Damage = 10.f;

protected:
	/**
	 * Called to bind functionality to input.
	 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * Called when the MoveForwardBackward input axis is used.
	 */
	void MoveForwardBackward(float Value);

	/**
	 * Called when the MoveLeftRight input axis is used.
	 */
	void MoveLeftRight(float Value);

	/**
	 * Called when the "Fire" input action button is pressed.
	 */
	void Fire();
};
