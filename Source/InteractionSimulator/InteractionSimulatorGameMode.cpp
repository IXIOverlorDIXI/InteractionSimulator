// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionSimulatorGameMode.h"
#include "InteractionSimulatorCharacter.h"
#include "UObject/ConstructorHelpers.h"

AInteractionSimulatorGameMode::AInteractionSimulatorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
