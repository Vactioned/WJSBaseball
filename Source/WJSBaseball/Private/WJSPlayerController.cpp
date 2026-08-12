#include "WJSPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "WJSBaseball.h"
#include "WJSChatInput.h"
#include "WJSGameModeBase.h"
#include "WJSPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

AWJSPlayerController::AWJSPlayerController()
{
	bReplicates = true;
}

void AWJSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UWJSChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance =
			CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(TurnTimerWidgetClass) == true)
	{
		TurnTimerWidgetInstance =
			CreateWidget<UUserWidget>(this, TurnTimerWidgetClass);
		if (IsValid(TurnTimerWidgetInstance) == true)
		{
			TurnTimerWidgetInstance->AddToViewport();
		}
	}
}

void AWJSPlayerController::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}

void AWJSPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	if (IsLocalController() == true)
	{
		ServerRPCPrintChatMessageString(InChatMessageString);
	}
}

void AWJSPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	WJSBaseballFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void AWJSPlayerController::ClientRPCPrintChatMessageString_Implementation(
	const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void AWJSPlayerController::ServerRPCPrintChatMessageString_Implementation(
	const FString& InChatMessageString)
{
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (IsValid(GameMode) == true)
	{
		AWJSGameModeBase* WJSGameMode = Cast<AWJSGameModeBase>(GameMode);
		if (IsValid(WJSGameMode) == true)
		{
			WJSGameMode->PrintChatMessageString(this, InChatMessageString);
		}
	}
}
