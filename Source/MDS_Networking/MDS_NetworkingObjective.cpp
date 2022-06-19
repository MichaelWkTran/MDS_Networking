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

}

void AMDS_NetworkingObjective::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMDS_NetworkingObjective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

