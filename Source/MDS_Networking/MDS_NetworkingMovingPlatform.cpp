#include "MDS_NetworkingMovingPlatform.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AMDS_NetworkingMovingPlatform::AMDS_NetworkingMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	pMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = pMeshComp;

	if (HasAuthority())
	{
		SetReplicates(true);
	}

	m_fBounceMagnitude = 200.0f;
	m_fBounceFactor = FMath::FRandRange(0.0f,2.0f);
	m_fBounceRate = 0.5f;
	m_fClientTimeSinceUpdate = 0;
	m_fClientTimeBetweenLastUpdate = 0;
}

void AMDS_NetworkingMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		m_vStartLocation = GetActorLocation();
	}
}

void AMDS_NetworkingMovingPlatform::Tick(float _fDeltaTime)
{
	Super::Tick(_fDeltaTime);

	//On Server
	if (HasAuthority())
	{
		m_fBounceFactor += _fDeltaTime * m_fBounceRate;
		while (m_fBounceFactor > 2.0f) m_fBounceFactor -= 2.0f;

		SetActorLocation(m_vStartLocation + FVector(0.0f, 0.0f, m_fBounceMagnitude * sin(m_fBounceFactor * 2 * PI)));
		m_vServerLocation = GetActorLocation();
	}

	//On Client
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		m_fClientTimeSinceUpdate += _fDeltaTime;

		//Ignore First Update
		if (m_fClientTimeSinceUpdate < KINDA_SMALL_NUMBER) return;

		//Interpolate between the client and server location
		float fLerpRatio = m_fClientTimeSinceUpdate / m_fClientTimeBetweenLastUpdate;
		FVector NewLocation = FMath::LerpStable(m_vClientStartLocation, m_vServerLocation, fLerpRatio);

		SetActorLocation(NewLocation);
	}
}

void AMDS_NetworkingMovingPlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDS_NetworkingMovingPlatform, m_vServerLocation);
}

void AMDS_NetworkingMovingPlatform::OnRep_ServerLocation()
{
	//In the case of simulated proxy
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		m_fClientTimeBetweenLastUpdate = m_fClientTimeSinceUpdate;
		m_fClientTimeSinceUpdate = 0;
		m_vClientStartLocation = GetActorLocation();
	}
}