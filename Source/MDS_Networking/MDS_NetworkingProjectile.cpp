#include "MDS_NetworkingProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "MDS_NetworkingCharacter.h"
#include "Net/UnrealNetwork.h"

AMDS_NetworkingProjectile::AMDS_NetworkingProjectile()
{
	//Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AMDS_NetworkingProjectile::OnHit);

	//Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	//Set as root component
	RootComponent = CollisionComp;

	//Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	if (HasAuthority())
	{
		//Die after 3 seconds by default
		InitialLifeSpan = 3.0f;

		//Replicate Propreties
		SetReplicates(true);
	}
}

void AMDS_NetworkingProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//Only add impulse and destroy projectile if we hit a physics actor
	if (OtherActor == nullptr || OtherActor == this || OtherComp == nullptr) return;

	AMDS_NetworkingCharacter* mCharacter = Cast<AMDS_NetworkingCharacter>(OtherActor);
	if (mCharacter != nullptr && GetLocalRole() == ROLE_Authority)
	{
		mCharacter->m_fCurrentHealth -= 10;
	}

	if (OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}

	Destroy();
}

void AMDS_NetworkingProjectile::Tick(float _fDeltatime)
{
	Super::Tick(_fDeltatime);

	//On Server
	if (HasAuthority())
	{
		m_ServerState.vLocation = GetActorLocation();
		m_ServerState.vVelocity = GetVelocity();
	}
}

void AMDS_NetworkingProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDS_NetworkingProjectile, m_ServerState);
}

void AMDS_NetworkingProjectile::OnRep_ServerState()
{
	//In the case of simulated proxy
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		SetActorLocation(m_ServerState.vLocation);
		RootComponent->ComponentVelocity = m_ServerState.vVelocity;
	}
}