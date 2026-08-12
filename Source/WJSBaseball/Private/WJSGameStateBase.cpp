#include "WJSGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "WJSPlayerController.h"

AWJSGameStateBase::AWJSGameStateBase()
	: CurrentTurnPlayerName(TEXT("Waiting"))
	, RemainingTurnTime(0)
{
	bReplicates = true;
}

void AWJSGameStateBase::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentTurnPlayerName);
	DOREPLIFETIME(ThisClass, RemainingTurnTime);
}

FText AWJSGameStateBase::GetTurnInfoText() const
{
	return FText::FromString(
		CurrentTurnPlayerName
		+ TEXT(" Turn - ")
		+ FString::FromInt(RemainingTurnTime)
		+ TEXT(" seconds"));
}

void AWJSGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(
	const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PlayerController =
			UGameplayStatics::GetPlayerController(GetWorld(), 0);

		if (IsValid(PlayerController) == true)
		{
			AWJSPlayerController* WJSPlayerController =
				Cast<AWJSPlayerController>(PlayerController);

			if (IsValid(WJSPlayerController) == true)
			{
				const FString NotificationString =
					InNameString + TEXT(" has joined the game.");

				WJSPlayerController->PrintChatMessageString(NotificationString);
			}
		}
	}
}
