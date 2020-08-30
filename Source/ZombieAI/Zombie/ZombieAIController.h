#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "ZombieAIController.generated.h"

/**
 * The ZombieAIController is the AIController that manages the states and movement
 * of the ZombieCharacter.
 */
UCLASS()
class ZOMBIEAI_API AZombieAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AZombieAIController();

	// The ZombieCharacter that the ZombieAIController is controlling.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Zombie)
	class AZombieCharacter* ZombieCharacter;

	// The ZombieCharacter's perception component.
	UPROPERTY(VisibleDefaultsOnly)
	class UAIPerceptionComponent* ZombiePerception;

	// The ZombieCharacter's sight sense component.
	UPROPERTY(VisibleDefaultsOnly)
	class UAISenseConfig_Sight* ZombieSight;

	// The radius around the ZombieCharacter that the PlayerCharacter will be sensed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Zombie)
	float ZombieSightRadius = 500.f;

	// The radius around the ZombieCharacter which they'll lose sight of the PlayerCharacter.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Zombie)
	float ZombieLoseSightRadius = ZombieSightRadius + 50.f;

	// The amount of time that the ZombieCharacter will remember the PlayerCharacter after
	// seeing them.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Zombie)
	float ZombieSightMaxAge = 5.f;

	// The timer used to pause between `Roam` calls.
	FTimerHandle RoamIdleTimer;

	// The timer handle used to pause between chasing and roaming.
	FTimerHandle ChaseIdleTimer;

protected:
	/**
	 * Called when the game starts.
	 */
	virtual void BeginPlay() override;

	/**
	 * Called when the ZombieAIController takes over the ZombieCharacter.
	 *
	 * @param ZombieCharacter The ZombieCharacter pawn.
	 */
	virtual void OnPossess(APawn* ZombiePawn) override;

	/**
	 * Called when the AIController's perception is updated.
	 */
	UFUNCTION()
	void OnTargetPerceptionUpdate(AActor* Actor, FAIStimulus Stimulus);

	/**
	 * Called when a move request has been completed.
	 */
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	/**
	 * Checks to see if the PlayerCharacter should be idling or roaming and then proceeds to do so. If
	 * the ZombieCharacter's `PreviousState` was `CHASE` and the ZombieCharacter is supposed to roam then
	 * the `StartLocation` will be updated to be the current location as we don't want the ZombieCharacter
	 * to go all the way back to the initial `StartLocation`.
	 */
	void IdleOrRoam();

	/**
	 * Called to make the ZombieCharacter roam to a different location within its
	 * roam radius.
	 */
	void Roam();

	/**
	 * Called to make the ZombieCharacter chase the PlayerCharacter.
	 *
	 * @param PlayerCharacter The PlayerCharacter to chase.
	 */
	void Chase(class APlayerCharacter* PlayerCharacter);

	/**
	 * Called to make the ZombieCharacter stop chasing the PlayerCharacter and go
	 * back to being idle/roaming.
	 */
	void StopChase();

	/**
	 * Called when an actor enters the ZombieCharacter's DamageCollider.
	 */
	UFUNCTION()
	void OnComponentEnterDamageCollider(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * Called when an actor leaves the ZombieCharacter's DamageCollider.
	 */
	UFUNCTION()
	void OnComponentLeaveDamageCollider(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
