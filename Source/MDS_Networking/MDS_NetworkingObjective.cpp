#include "MDS_NetworkingObjective.h"
#include "MDS_NetworkingCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

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
	m_fClientTimeSinceUpdate = 0;
	m_fClientTimeBetweenLastUpdate = 0;
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

	//On Server
	if (HasAuthority())
	{
		m_fBounceFactor += _fDeltaTime * m_fBounceRate;
		while (m_fBounceFactor > 2.0f) m_fBounceFactor -= 2.0f;
		
		FObjectiveMove Move;
		Move.fDeltaTime = _fDeltaTime;
		Move.fBounceFactor = m_fBounceFactor;

		SetActorLocation(m_vStartLocation + FVector(0.0f, 0.0f, m_fBounceMagnitude * sin(m_fBounceFactor * 2 * PI)));

		m_ServerState.LastMove = Move;
		m_ServerState.Transform = GetActorTransform();
	}

	//On client
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		m_fClientTimeSinceUpdate += _fDeltaTime;
		
		//Ignore First Update
		if (m_fClientTimeSinceUpdate < KINDA_SMALL_NUMBER) return;

		//Lerp Position
		FVector StartLocation = m_ClientStartTransform.GetLocation();
		FVector TargetLocation = m_ServerState.Transform.GetLocation();
		float fLerpRatio = m_fClientTimeBetweenLastUpdate / m_fClientTimeSinceUpdate;

		FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, fLerpRatio);

		SetActorLocation(NewLocation);
	}
}

void AMDS_NetworkingObjective::NotifyActorBeginOverlap(AActor* _pOther)
{
	Super::NotifyActorBeginOverlap(_pOther);

	AMDS_NetworkingCharacter* pCharacter = Cast<AMDS_NetworkingCharacter>(_pOther);
	if (pCharacter)
	{
		pCharacter->m_bHaveObjective = true;
	}
}

void AMDS_NetworkingObjective::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDS_NetworkingObjective, m_ServerState);
}

void AMDS_NetworkingObjective::OnRep_ServerState()
{
	//In the case of simulated proxy
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		m_fClientTimeSinceUpdate = m_fClientTimeBetweenLastUpdate;
		m_fClientTimeBetweenLastUpdate = 0;
		m_ClientStartTransform = GetActorTransform();
	}
}

FVector AMDS_NetworkingObjective::SimulateMove(FObjectiveMove _Move)
{
	return FVector(0.0f, 0.0f, m_fBounceMagnitude * sin(m_fBounceFactor * 2 * PI));
}