#include "MDS_NetworkingObjective.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AMDS_NetworkingObjective::AMDS_NetworkingObjective()
{
 	PrimaryActorTick.bCanEverTick = true;

	pMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	pMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RootComponent = pMeshComp;
	
	pSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	
	pSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	pSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	pSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	pSphereComp->SetupAttachment(pMeshComp);

	if (HasAuthority())
	{
		SetReplicates(true);
		SetReplicateMovement(true);
	}

	m_fBounceMagnitude = 10.0f;
	m_fBounceFactor = 0.0f;
	m_fBounceRate = 1.0f;
}

void AMDS_NetworkingObjective::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		m_vStartLocation = GetActorLocation();
	}
}

void AMDS_NetworkingObjective::Tick(float _fDeltaTime)
{
	Super::Tick(_fDeltaTime);

	if (HasAuthority())
	{
		m_fBounceFactor += _fDeltaTime * m_fBounceRate;
		while (m_fBounceFactor > 2.0f) m_fBounceFactor -= 2.0f;
		SetActorLocation(m_vStartLocation + FVector(0.0f, 0.0f, m_fBounceMagnitude * sin(m_fBounceFactor * 2 * PI)));
	}
}