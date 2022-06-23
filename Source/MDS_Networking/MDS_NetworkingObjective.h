#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDS_NetworkingObjective.generated.h"

USTRUCT()
struct FObjectiveMove
{
	GENERATED_BODY()

	UPROPERTY();
		float fDeltaTime;
	UPROPERTY();
		float fBounceFactor;
};

USTRUCT()
struct FObjectiveServerState
{
	GENERATED_BODY()

	UPROPERTY()
		FObjectiveMove LastMove;
	UPROPERTY()
		FTransform Transform;
};

UCLASS()
class MDS_NETWORKING_API AMDS_NetworkingObjective : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float _fDeltaTime) override;
	virtual void NotifyActorBeginOverlap(AActor* _pOther) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(VisibleAnywhere, Category = "Components");
		class UStaticMeshComponent* pMeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Components");
		class USphereComponent* pSphereComp;

	FVector m_vStartLocation;
	float m_fBounceMagnitude;
	float m_fBounceFactor;
	float m_fBounceRate;
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
		FObjectiveServerState m_ServerState;
		UFUNCTION()
		void OnRep_ServerState();
	float m_fClientTimeSinceUpdate;
	float m_fClientTimeBetweenLastUpdate;
	FTransform m_ClientStartTransform;

	UFUNCTION()
		FVector SimulateMove(FObjectiveMove _Move);

public:	
	AMDS_NetworkingObjective();
};
