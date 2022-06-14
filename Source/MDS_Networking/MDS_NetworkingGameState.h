#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MDS_NetworkingGameState.generated.h"

UCLASS()
class MDS_NETWORKING_API AMDS_NetworkingGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(NetMulticast, Reliable) void MultiCastWin();

};
