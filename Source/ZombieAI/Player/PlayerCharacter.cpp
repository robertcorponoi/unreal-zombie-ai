#include "PlayerCharacter.h"
#include "BulletActor.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

/**
 * Sets the default value from the PlayerCharacter
 */
APlayerCharacter::APlayerCharacter()
{
	// Load the player and gun skeletal meshes.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerSkeletalMeshAsset(TEXT("SkeletalMesh'/Game/FirstPerson/Character/Mesh/SK_Mannequin_Arms.SK_Mannequin_Arms'"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> GunSkeletalMeshAsset(TEXT("SkeletalMesh'/Game/FirstPerson/FPWeapon/Mesh/SK_FPGun.SK_FPGun'"));

	// Load the gun fire animation.
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> GunAnimBlueprintAsset(TEXT("AnimBlueprint'/Game/FirstPerson/Animations/FirstPerson_AnimBP.FirstPerson_AnimBP'"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> GunFireAnimationAsset(TEXT("AnimMontage'/Game/FirstPerson/Animations/FirstPersonFire_Montage.FirstPersonFire_Montage'"));

	// Create the first person camera, set its relative location and attach it to the
	// capsule component.
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetRelativeLocation(FVector(-40.f, 2.f, 70.f));
	PlayerCamera->bUsePawnControlRotation = true;
	PlayerCamera->SetupAttachment(GetCapsuleComponent());

	// Create the player mesh component and set up its position and defaults and
	// lastly attach it to the PlayerCamera.
	PlayerSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PlayerSkeletalMesh"));
	PlayerSkeletalMesh->SetSkeletalMesh(PlayerSkeletalMeshAsset.Object);
	PlayerSkeletalMesh->SetRelativeLocationAndRotation(FVector(-0.5f, -4.5f, -155.f), FRotator(2.f, -20.f, 5.f));
	PlayerSkeletalMesh->SetOnlyOwnerSee(true);
	PlayerSkeletalMesh->SetAnimInstanceClass(GunAnimBlueprintAsset.Object->GeneratedClass);
	PlayerSkeletalMesh->CastShadow = false;
	PlayerSkeletalMesh->bCastDynamicShadow = false;
	PlayerSkeletalMesh->CanCharacterStepUpOn = ECB_Yes;
	PlayerSkeletalMesh->SetupAttachment(PlayerCamera);
	PlayerSkeletalMesh->SetHiddenInGame(false, true);

	// Create the gun mesh component and set up its defaults and lastly attach it to
	// the RootComponent and the grip point of the PlayerSkeletalMesh.
	GunSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunSkeletalMesh"));
	GunSkeletalMesh->SetSkeletalMesh(GunSkeletalMeshAsset.Object);
	GunSkeletalMesh->SetOnlyOwnerSee(true);
	GunSkeletalMesh->CastShadow = false;
	GunSkeletalMesh->bCastDynamicShadow = false;
	GunSkeletalMesh->CanCharacterStepUpOn = ECB_Yes;
	GunSkeletalMesh->SetBoundsScale(2.f);
	GunSkeletalMesh->SetupAttachment(PlayerSkeletalMesh, TEXT("GripPoint"));
	GunSkeletalMesh->SetupAttachment(RootComponent);
	GunSkeletalMesh->AttachToComponent(PlayerSkeletalMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Create the stimuli source and set it to register as a source for `AISense_Sight`.
	PlayerStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PlayerStimuliSource"));
	PlayerStimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());

	// Set the gun fire animation to the montage loaded above.
	GunFireAnimation = GunFireAnimationAsset.Object;

	// Set the size of the PlayerCharacter's capsule collider.
	GetCapsuleComponent()->InitCapsuleSize(55.f, 100.f);
	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_Yes;

	// Set the default offset for where the BulletActors should spawn.
	GunOffset = FVector(100.f, 0.f, 10.f);

	// Set the PlayerCharacter to be the default player of the game.
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

/**
 * Called to bind functionality to input.
 */
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// Bind the forward, backward, left, and right movement input axis to the
	// `MoveForwardBackward` and `MoveLeftRight` methods.
	PlayerInputComponent->BindAxis("MoveForwardBackward", this, &APlayerCharacter::MoveForwardBackward);
	PlayerInputComponent->BindAxis("MoveLeftRight", this, &APlayerCharacter::MoveLeftRight);

	// Bind the mouse x and y axis to controlling the yaw and pitch.
	PlayerInputComponent->BindAxis("LookUpDown", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookLeftRight", this, &APawn::AddControllerPitchInput);

	// Bind the jump input action to the default Character jump logic.
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind the fire input action to the `Fire` method.
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::Fire);
}

/**
 * Called when the MoveForwardBackward input axis is used.
 */
void APlayerCharacter::MoveForwardBackward(float Value)
{
	if (Value != 0.f) AddMovementInput(GetActorForwardVector(), Value);
}

/**
 * Called when the MoveLeftRight input axis is used.
 */
void APlayerCharacter::MoveLeftRight(float Value)
{
	if (Value != 0.f) AddMovementInput(GetActorRightVector(), Value);
}

/**
 * Called when the Fire input action is pressed.
 */
void APlayerCharacter::Fire()
{
	// Return early if `GetWorld()` returns a nullptr.
	UWorld* const World = GetWorld();
	if (World == nullptr) return;

	const FRotator SpawnRotation = GetControlRotation();

	// Since the `BulletSpawnLocation` is in camera space, we have to transform it to
	// world space before offsetting it from the character location to find the final
	// bullet spawn location.
	const FVector SpawnLocation = ((BulletSpawnLocation != nullptr) ? BulletSpawnLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

	// Set Spawn Collision Handling Override.
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	// Spawn the BulletActor and pass the `Damage` value over to it.
	ABulletActor* BulletActor = World->SpawnActorDeferred<ABulletActor>(ABulletActor::StaticClass(), FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
	BulletActor->Damage = Damage;
	UGameplayStatics::FinishSpawningActor(BulletActor, FTransform(SpawnRotation, SpawnLocation, FVector(1.f, 1.f, 1.f)));

	// Get the animation object for the PlayerCharacter's body mesh and play the fire animation.
	UAnimInstance* AnimInstance = PlayerSkeletalMesh->GetAnimInstance();
	if (AnimInstance == nullptr) return;

	AnimInstance->Montage_Play(GunFireAnimation, 1.f);
}
