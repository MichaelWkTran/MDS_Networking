#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "MDS_NetworkingCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config = Game)
class AMDS_NetworkingCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* Mesh1P;
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* FP_Gun;
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USceneComponent* FP_MuzzleLocation;
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* VR_Gun;
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USceneComponent* VR_MuzzleLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FirstPersonCameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMotionControllerComponent* R_MotionController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMotionControllerComponent* L_MotionController;

protected:
	virtual void BeginPlay();
	virtual void Tick(float _fDeltatime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	
	//UFUNCTION(Server, Reliable)
		//void Server_SendMove(FGoKartMove _Move);

	void MoveForward(float Val);
	void MoveRight(float Val);
	void Jump();
	void StopJumping();
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void OnFire();
		UFUNCTION(Server, Reliable, WithValidation)
		void Server_OnFire();

	void OnResetVR();

	struct TouchData
	{
		TouchData() { bIsPressed = false; Location = FVector::ZeroVector; }
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData TouchItem;

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	AMDS_NetworkingCharacter();

	//Health
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, EditAnywhere, BlueprintReadWrite)
		float m_fCurrentHealth;
		UFUNCTION()
		void OnRep_CurrentHealth();

	//Objective
		UPROPERTY(BlueprintReadWrite)
			bool m_bHaveObjective;

	//Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	//Gun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector GunOffset;
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AMDS_NetworkingProjectile> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		USoundBase* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UAnimMontage* FireAnimation;

	//Controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		uint8 bUsingMotionControllers : 1;

	//Get Set Components
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};

