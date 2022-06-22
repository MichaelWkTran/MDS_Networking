#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDS_NetworkingObjective.generated.h"

UCLASS()
class MDS_NETWORKING_API AMDS_NetworkingObjective : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components");
		class UStaticMeshComponent* pMeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Components");
		class USphereComponent* pSphereComp;

	FVector m_vStartLocation;
	float m_fBounceMagnitude;
	float m_fBounceFactor;
	float m_fBounceRate;

public:	
	AMDS_NetworkingObjective();
	
	virtual void Tick(float _fDeltaTime) override;
};
