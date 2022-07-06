#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDS_NetworkingMovingPlatform.generated.h"

UCLASS()
class MDS_NETWORKING_API AMDS_NetworkingMovingPlatform : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float _fDeltaTime) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(VisibleAnywhere, Category = "Components");
		class UStaticMeshComponent* pMeshComp;

	FVector m_vStartLocation;
	float m_fBounceMagnitude;
	float m_fBounceRate;
	float m_fBounceFactor;
	
	UPROPERTY(ReplicatedUsing = OnRep_ServerLocation)
		FVector m_vServerLocation;
	UFUNCTION()
		void OnRep_ServerLocation();
	FVector m_vClientStartLocation;
	float m_fClientTimeSinceUpdate;
	float m_fClientTimeBetweenLastUpdate;

public:
	AMDS_NetworkingMovingPlatform();
};
