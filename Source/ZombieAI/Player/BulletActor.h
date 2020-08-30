#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletActor.generated.h"

/**
 * The BulletActor is the projectile that's shot out of the PlayerCharacter's gun.
 */
UCLASS()
class ZOMBIEAI_API ABulletActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties.
	ABulletActor();

	// The static mesh of the BulletActor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* BulletStaticMesh;

	// The sphere collider of the BulletActor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* BulletSphereCollider;

	// The projectile movement component of the BulletActor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UProjectileMovementComponent* BulletMovement;

	// The damage this BulletActor should do.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Damage;

public:	
	/**
	 * Called when the BulletActor hits another component.
	 */
	UFUNCTION()
	void OnBulletHitComponent(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
