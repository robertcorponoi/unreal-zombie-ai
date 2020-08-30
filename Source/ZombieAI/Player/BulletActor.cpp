#include "BulletActor.h"
#include "../Zombie/ZombieCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

/**
 * Sets the default values of the BulletActor.
 */
ABulletActor::ABulletActor()
{
	// Load the BulletActor's mesh.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BulletStaticMeshAsset(TEXT("StaticMesh'/Game/FirstPerson/Meshes/FirstPersonProjectileMesh.FirstPersonProjectileMesh'"));

	// Create the sphere collider, set its radius, set it to have a collision profile
	// of Projectile and lastly add the `OnBulletHitComponent` method to respond to the
	// sphere collider making contact with another component.
	BulletSphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("BulletSphereCollider"));
	BulletSphereCollider->InitSphereRadius(20.f);
	BulletSphereCollider->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	BulletSphereCollider->OnComponentHit.AddDynamic(this, &ABulletActor::OnBulletHitComponent);
	RootComponent = BulletSphereCollider;

	// Create the bullet mesh, set it to the `BulletStaticMesh` loaded above and attach
	// it to the `BulletSphereCollider`.
	BulletStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletStaticMesh"));
	BulletStaticMesh->SetStaticMesh(BulletStaticMeshAsset.Object);
	BulletStaticMesh->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	BulletStaticMesh->UnWeldFromParent();
	BulletStaticMesh->BodyInstance.SetCollisionProfileName(TEXT("NoCollision"));
	BulletStaticMesh->SetupAttachment(RootComponent);

	// Create the ProjectileMovementComponent and set its speeds and default properties.
	BulletMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("BulletMovement"));
	BulletMovement->UpdatedComponent = BulletSphereCollider;
	BulletMovement->InitialSpeed = 3000.f;
	BulletMovement->MaxSpeed = 3000.f;
	BulletMovement->bRotationFollowsVelocity = true;
	BulletMovement->bShouldBounce = false;

	// Set the BulletActor to die after 3 seconds.
	InitialLifeSpan = 3.f;
}

/**
 * Called when the BulletActor hits another component.
 */
void ABulletActor::OnBulletHitComponent(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// We want to return early if anything is null as it could cause a crash otherwise.
	if ((OtherActor == nullptr) || (OtherActor == this) || (OtherComp == nullptr)) return;

	// Cast the `OtherActor` to a `ZombieCharacter` if we can and call its `TakeDamage` method.
	AZombieCharacter* ZombieCharacter = Cast<AZombieCharacter>(OtherActor);
	if (ZombieCharacter == nullptr) return;
	ZombieCharacter->Hit(Damage);

	// Finally destroy the the BulletActor so we don't end up with a bunch of bullets that
	// litter the level and impact performance.
	Destroy();
}
