#include "WJSPlayerState.h"

#include "Net/UnrealNetwork.h"

AWJSPlayerState::AWJSPlayerState()
	: PlayerNameString(TEXT("None"))
	, CurrentGuessCount(0)
	, MaxGuessCount(3)
	, bHasGuessedThisTurn(false)
{
	bReplicates = true;
}

void AWJSPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerNameString);
	DOREPLIFETIME(ThisClass, CurrentGuessCount);
	DOREPLIFETIME(ThisClass, MaxGuessCount);
}

FString AWJSPlayerState::GetPlayerInfoString() const
{
	return PlayerNameString
		+ TEXT("(")
		+ FString::FromInt(CurrentGuessCount)
		+ TEXT("/")
		+ FString::FromInt(MaxGuessCount)
		+ TEXT(")");
}
