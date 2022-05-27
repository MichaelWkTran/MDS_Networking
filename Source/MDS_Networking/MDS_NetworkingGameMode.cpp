// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDS_NetworkingGameMode.h"
#include "MDS_NetworkingHUD.h"
#include "MDS_NetworkingCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMDS_NetworkingGameMode::AMDS_NetworkingGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMDS_NetworkingHUD::StaticClass();
}
