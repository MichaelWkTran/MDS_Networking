#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDS_NetworkingProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

USTRUCT()
struct FProjectileServerState
{
	GENERATED_BODY()

	UPROPERTY()
		FVector vLocation;
	UPROPERTY()
		FVector vVelocity;
};

UCLASS(config=Game)
class AMDS_NetworkingProjectile : public AActor
{
	GENERATED_BODY()

private:
	//Components-----------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
		USphereComponent* CollisionComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		UProjectileMovementComponent* ProjectileMovement;

	

protected:
	virtual void Tick(float _fDeltatime) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	//
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
		FProjectileServerState m_ServerState;
	UFUNCTION()
		void OnRep_ServerState();
public:
	AMDS_NetworkingProjectile();

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	//Get Set Methods---------------------------------------------------------------------------------------------------------------------------------------------
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};

