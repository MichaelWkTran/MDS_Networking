#include "MDS_NetworkingCharacter.h"
#include "MDS_NetworkingProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" //for FXRMotionControllerBase::RightHandSourceId
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

AMDS_NetworkingCharacter::AMDS_NetworkingCharacter()
{
	m_fCurrentHealth = 100.0f;
	m_fBaseTurnRate = 45.0f;
	m_fBaseLookUpRate = 45.0f;

	m_bUsingMotionControllers = false;

	//Setup Components--------------------------------------------------------------------------------------------------------------------------------------------
	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);

	//Create a CameraComponent	
	m_pFPCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	m_pFPCameraComponent->SetupAttachment(GetCapsuleComponent());
	
	m_pFPCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.0f));
	m_pFPCameraComponent->bUsePawnControlRotation = true;

	//Create a mesh component that will be used when being viewed from a '1st person' view
	m_pMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	m_pMesh1P->SetupAttachment(m_pFPCameraComponent);
	
	m_pMesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	m_pMesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
	
	m_pMesh1P->SetOnlyOwnerSee(true);
	m_pMesh1P->bCastDynamicShadow = false;
	m_pMesh1P->CastShadow = false;

	//Create a mesh component that will be used when being viewed from a '3rd person' view
	m_pMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));

	//Create a gun mesh component
	m_pFPGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));

	m_pFPGun->SetOnlyOwnerSee(false);
	m_pFPGun->bCastDynamicShadow = false;
	m_pFPGun->CastShadow = false;
	//FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	m_pFPGun->SetupAttachment(RootComponent);

	m_pFPMuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	m_pFPMuzzleLocation->SetupAttachment(m_pFPGun);
	m_pFPMuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	//Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	//Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	//are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	//Create VR Controllers.
	m_pMotionControllerR = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	m_pMotionControllerR->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	m_pMotionControllerR->SetupAttachment(RootComponent);
	m_pMotionControllerL = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	m_pMotionControllerL->SetupAttachment(RootComponent);

	//Create a gun and attach it to the right-hand VR controller.
	//Create a gun mesh component
	m_pVRGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	m_pVRGun->SetOnlyOwnerSee(false);
	m_pVRGun->bCastDynamicShadow = false;
	m_pVRGun->CastShadow = false;
	m_pVRGun->SetupAttachment(m_pMotionControllerR);
	m_pVRGun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	m_pVRMuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	m_pVRMuzzleLocation->SetupAttachment(m_pVRGun);
	m_pVRMuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	m_pVRMuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		//Counteract the rotation of the VR gun model.
}

/*Virtual*/ void AMDS_NetworkingCharacter::BeginPlay()
{
	//Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	m_pFPGun->AttachToComponent(m_pMesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	//Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (m_bUsingMotionControllers)
	{
		m_pVRGun->SetHiddenInGame(false, true);
		m_pMesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		m_pVRGun->SetHiddenInGame(true, true);
		m_pMesh1P->SetHiddenInGame(false, true);
	}
}

void AMDS_NetworkingCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDS_NetworkingCharacter, m_fCurrentHealth);
}

void AMDS_NetworkingCharacter::SetupPlayerInputComponent(class UInputComponent* _pPlayerInputComponent)
{
	//Set up gameplay key bindings
	check(_pPlayerInputComponent);

	//Bind jump events
	_pPlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMDS_NetworkingCharacter::Jump);
	_pPlayerInputComponent->BindAction("Jump", IE_Released, this, &AMDS_NetworkingCharacter::StopJumping);

	//Bind fire event
	_pPlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMDS_NetworkingCharacter::OnFire);

	//Enable touchscreen input
	EnableTouchscreenMovement(_pPlayerInputComponent);

	_pPlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMDS_NetworkingCharacter::OnResetVR);

	//Bind movement events
	_pPlayerInputComponent->BindAxis("MoveForward", this, &AMDS_NetworkingCharacter::MoveForward);
	_pPlayerInputComponent->BindAxis("MoveRight", this, &AMDS_NetworkingCharacter::MoveRight);

	//We have 2 versions of the rotation bindings to handle different kinds of devices differently
	//"turn" handles devices that provide an absolute delta, such as a mouse.
	//"turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	_pPlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	_pPlayerInputComponent->BindAxis("TurnRate", this, &AMDS_NetworkingCharacter::TurnAtRate);
	_pPlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	_pPlayerInputComponent->BindAxis("LookUpRate", this, &AMDS_NetworkingCharacter::LookUpAtRate);
}

//void AMDS_NetworkingCharacter::Server_SendMove_Implementation(FGoKartMove _Move)
//{
//	SimulateMove(_Move);
//
//	ServerState.LastMove = _Move;
//	ServerState.Transform = GetActorTransform();
//	ServerState.Velocity = velocity;
//}

void AMDS_NetworkingCharacter::MoveForward(float _fValue)
{
	if (_fValue == 0.0f || m_fCurrentHealth <= 0) return;
	
	AddMovementInput(GetActorForwardVector(), _fValue);
}

void AMDS_NetworkingCharacter::MoveRight(float _fValue)
{
	if (_fValue == 0.0f || m_fCurrentHealth <= 0) return;

	AddMovementInput(GetActorRightVector(), _fValue);
}

void AMDS_NetworkingCharacter::Jump()
{
	if (m_fCurrentHealth <= 0) return;

	bPressedJump = true;
	JumpKeyHoldTime = 0.0f;
}

void AMDS_NetworkingCharacter::StopJumping()
{
	if (m_fCurrentHealth <= 0) return;

	bPressedJump = false;
	ResetJumpState();
}

void AMDS_NetworkingCharacter::TurnAtRate(float _fRate)
{
	if (m_fCurrentHealth <= 0) return;

	//Calculate delta for this frame from the rate information
	AddControllerYawInput(_fRate * m_fBaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMDS_NetworkingCharacter::LookUpAtRate(float _fRate)
{
	//Calculate delta for this frame from the rate information
	AddControllerPitchInput(_fRate * m_fBaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMDS_NetworkingCharacter::OnFire()
{
	if (m_fCurrentHealth <= 0) return;

	Server_OnFire();

	//Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	//Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		//Get the animation object for the arms mesh
		UAnimInstance* pAnimInstance = m_pMesh1P->GetAnimInstance();
		if (pAnimInstance != nullptr)
		{
			pAnimInstance->Montage_Play(FireAnimation, 1.0f);
		}
	}
}

void AMDS_NetworkingCharacter::Server_OnFire_Implementation()
{
	if (ProjectileClass == nullptr) return;

	UWorld* const pWorld = GetWorld();
	if (pWorld == nullptr) return;

	if (m_bUsingMotionControllers)
	{
		const FRotator SpawnRotation = m_pVRMuzzleLocation->GetComponentRotation();
		const FVector SpawnLocation = m_pVRMuzzleLocation->GetComponentLocation();
		pWorld->SpawnActor<AMDS_NetworkingProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
	}
	else
	{
		const FRotator SpawnRotation = GetControlRotation();
		//MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = ((m_pFPMuzzleLocation != nullptr) ? m_pFPMuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		//spawn the projectile at the muzzle
		pWorld->SpawnActor<AMDS_NetworkingProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
	}
}

bool AMDS_NetworkingCharacter::Server_OnFire_Validate()
{
	if (m_fCurrentHealth > 100.0f) return false;

	return true;
}

void AMDS_NetworkingCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMDS_NetworkingCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true) return;

	if (FingerIndex == TouchItem.FingerIndex && (TouchItem.bMoved == false))
	{
		OnFire();
	}

	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AMDS_NetworkingCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false) return;

	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AMDS_NetworkingCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

bool AMDS_NetworkingCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (m_fCurrentHealth <= 0) return false;

	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMDS_NetworkingCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AMDS_NetworkingCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AMDS_NetworkingCharacter::TouchUpdate);
		return true;
	}

	return false;
}

void AMDS_NetworkingCharacter::OnRep_CurrentHealth()
{

}