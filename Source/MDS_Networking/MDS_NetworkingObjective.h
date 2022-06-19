#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDS_NetworkingObjective.generated.h"

UCLASS()
class MDS_NETWORKING_API AMDS_NetworkingObjective : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Components");
		class UStaticMeshComponent* pMeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Components");
		class USphereComponent* pSphereComp;

protected:
	virtual void BeginPlay() override;

public:	
	AMDS_NetworkingObjective();
	
	virtual void Tick(float DeltaTime) override;

};
