// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MDS_NetworkingHUD.generated.h"

UCLASS()
class AMDS_NetworkingHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMDS_NetworkingHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

