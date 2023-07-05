// Copyright Epic Games, Inc. All Rights Reserved.

#include "GasLearnGameMode.h"
#include "GasLearnCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGasLearnGameMode::AGasLearnGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
