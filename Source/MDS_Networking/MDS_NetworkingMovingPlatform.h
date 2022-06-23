#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDS_NetworkingMovingPlatform.generated.h"

USTRUCT()
struct FMovingPlatformMove
{
	GENERATED_BODY()

	UPROPERTY();
		float fBounceFactor;
};

USTRUCT()
struct FMovingPlatformServerState
{
	GENERATED_BODY()

	UPROPERTY()
		FMovingPlatformMove LastMove;
	UPROPERTY()
		FTransform Transform;
};

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
	float m_fBounceFactor;
	float m_fBounceRate;
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
		FMovingPlatformServerState m_ServerState;
	UFUNCTION()
		void OnRep_ServerState();
	float m_fClientTimeSinceUpdate;
	float m_fClientTimeBetweenLastUpdate;
	FTransform m_ClientStartTransform;

	UFUNCTION()
		FVector SimulateMove(FMovingPlatformMove _Move);

public:
	AMDS_NetworkingMovingPlatform();
};
