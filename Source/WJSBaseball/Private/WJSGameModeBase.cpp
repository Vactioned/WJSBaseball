// Fill out your copyright notice in the Description page of Project Settings.


#include "WJSGameModeBase.h"

#include "EngineUtils.h"
#include "WJSGameStateBase.h"
#include "WJSPlayerController.h"
#include "WJSPlayerState.h"
#include "TimerManager.h"

AWJSGameModeBase::AWJSGameModeBase()
	: TurnDuration(10)
	, CurrentTurnPlayerIndex(INDEX_NONE)
	, bIsWaitingForRematch(false)
{
	PlayerStateClass = AWJSPlayerState::StaticClass();
}

void AWJSGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	AWJSPlayerController* WJSPlayerController = Cast<AWJSPlayerController>(NewPlayer);
	if (IsValid(WJSPlayerController) == true)
	{
		WJSPlayerController->NotificationText =
			FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.AddUnique(WJSPlayerController);

		AWJSPlayerState* WJSPlayerState =
			WJSPlayerController->GetPlayerState<AWJSPlayerState>();
		if (IsValid(WJSPlayerState) == true)
		{
			WJSPlayerState->PlayerNameString =
				TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());

			AWJSGameStateBase* WJSGameStateBase = GetGameState<AWJSGameStateBase>();
			if (IsValid(WJSGameStateBase) == true)
			{
				WJSGameStateBase->MulticastRPCBroadcastLoginMessage(
					WJSPlayerState->PlayerNameString);
			}

			if (CurrentTurnPlayerIndex == INDEX_NONE)
			{
				CurrentTurnPlayerIndex = 0;
				StartTurn();
			}
		}
	}
}

void AWJSGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Log, TEXT("Secret Number: %s"), *SecretNumberString);
}

FString AWJSGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 Number = 1; Number <= 9; ++Number)
	{
		Numbers.Add(Number);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());

	FString Result;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const int32 RandomIndex = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[RandomIndex]));
		Numbers.RemoveAt(RandomIndex);
	}

	return Result;
}

bool AWJSGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	if (InNumberString.Len() != 3)
	{
		return false;
	}

	TSet<TCHAR> UniqueDigits;
	for (const TCHAR Character : InNumberString)
	{
		if (FChar::IsDigit(Character) == false || Character == TEXT('0'))
		{
			return false;
		}

		if (UniqueDigits.Contains(Character) == true)
		{
			return false;
		}

		UniqueDigits.Add(Character);
	}

	return true;
}

FString AWJSGameModeBase::JudgeResult(
	const FString& InSecretNumberString,
	const FString& InGuessNumberString)
{
	int32 StrikeCount = 0;
	int32 BallCount = 0;

	for (int32 Index = 0; Index < 3; ++Index)
	{
		if (InSecretNumberString[Index] == InGuessNumberString[Index])
		{
			++StrikeCount;
		}
		else
		{
			const FString GuessCharacter = FString::Printf(
				TEXT("%c"), InGuessNumberString[Index]);

			if (InSecretNumberString.Contains(GuessCharacter) == true)
			{
				++BallCount;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void AWJSGameModeBase::PrintChatMessageString(
	AWJSPlayerController* InChattingPlayerController,
	const FString& InChatMessageString)
{
	if (IsValid(InChattingPlayerController) == false)
	{
		return;
	}

	AWJSPlayerState* WJSPlayerState =
		InChattingPlayerController->GetPlayerState<AWJSPlayerState>();
	if (IsValid(WJSPlayerState) == false)
	{
		return;
	}

	if (bIsWaitingForRematch == true)
	{
		HandleRematchRequest(InChattingPlayerController, InChatMessageString);
		return;
	}

	if (AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex) == false
		|| AllPlayerControllers[CurrentTurnPlayerIndex] != InChattingPlayerController)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(
			TEXT("현재 본인의 턴이 아닙니다."));
		return;
	}

	AWJSGameStateBase* WJSGameStateBase = GetGameState<AWJSGameStateBase>();
	if (IsValid(WJSGameStateBase) == false
		|| WJSGameStateBase->RemainingTurnTime <= 0)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(
			TEXT("입력 시간이 종료되었습니다."));
		return;
	}

	FString ErrorMessage;
	if (InChatMessageString.Len() != 3)
	{
		ErrorMessage = TEXT("3자리 숫자를 입력해주세요.");
	}
	else
	{
		TSet<TCHAR> UniqueDigits;
		for (const TCHAR Character : InChatMessageString)
		{
			if (FChar::IsDigit(Character) == false || Character == TEXT('0'))
			{
				ErrorMessage = TEXT("1~9 사이의 숫자만 입력해주세요.");
				break;
			}

			if (UniqueDigits.Contains(Character) == true)
			{
				ErrorMessage = TEXT("중복되지 않은 숫자를 입력해주세요.");
				break;
			}

			UniqueDigits.Add(Character);
		}
	}

	if (ErrorMessage.IsEmpty() == false)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(ErrorMessage);
		return;
	}

	if (WJSPlayerState->CurrentGuessCount >= WJSPlayerState->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(
			TEXT("기회를 모두 사용했습니다."));
		return;
	}

	IncreaseGuessCount(InChattingPlayerController);
	WJSPlayerState->bHasGuessedThisTurn = true;

	const FString JudgeResultString = JudgeResult(
		SecretNumberString, InChatMessageString);
	const FString MessageToBroadcast =
		WJSPlayerState->GetPlayerInfoString()
		+ TEXT(": ")
		+ InChatMessageString
		+ TEXT(" -> ")
		+ JudgeResultString;

	for (TActorIterator<AWJSPlayerController> It(GetWorld()); It; ++It)
	{
		AWJSPlayerController* WJSPlayerController = *It;
		if (IsValid(WJSPlayerController) == true)
		{
			WJSPlayerController->ClientRPCPrintChatMessageString(MessageToBroadcast);
		}
	}

	const int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
	if (JudgeGame(InChattingPlayerController, StrikeCount) == false)
	{
		AdvanceTurn();
	}
}

void AWJSGameModeBase::ResetGame()
{
	bIsWaitingForRematch = false;
	RematchReadyPlayers.Empty();

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Log, TEXT("Secret Number: %s"), *SecretNumberString);

	for (const TObjectPtr<AWJSPlayerController>& WJSPlayerController
		: AllPlayerControllers)
	{
		if (IsValid(WJSPlayerController) == false)
		{
			continue;
		}

		AWJSPlayerState* WJSPlayerState =
			WJSPlayerController->GetPlayerState<AWJSPlayerState>();
		if (IsValid(WJSPlayerState) == true)
		{
			WJSPlayerState->CurrentGuessCount = 0;
			WJSPlayerState->bHasGuessedThisTurn = false;
		}

		WJSPlayerController->NotificationText =
			FText::FromString(TEXT("새 게임을 시작합니다."));
		WJSPlayerController->ClientRPCClearChatMessages();
	}

	CurrentTurnPlayerIndex = FindNextAvailablePlayerIndex(0);
	StartTurn();
	BroadcastChatMessage(TEXT("새 게임이 시작되었습니다."));
}

bool AWJSGameModeBase::JudgeGame(
	AWJSPlayerController* InChattingPlayerController,
	int32 InStrikeCount)
{
	FString ResultMessage;

	if (InStrikeCount == 3 && IsValid(InChattingPlayerController) == true)
	{
		AWJSPlayerState* WinnerPlayerState =
			InChattingPlayerController->GetPlayerState<AWJSPlayerState>();
		if (IsValid(WinnerPlayerState) == true)
		{
			ResultMessage = WinnerPlayerState->PlayerNameString
				+ TEXT(" has won the game.");
		}
	}
	else
	{
		bool bIsDraw = AllPlayerControllers.Num() > 0;
		for (const TObjectPtr<AWJSPlayerController>& WJSPlayerController
			: AllPlayerControllers)
		{
			if (IsValid(WJSPlayerController) == false)
			{
				continue;
			}

			AWJSPlayerState* WJSPlayerState =
				WJSPlayerController->GetPlayerState<AWJSPlayerState>();
			if (IsValid(WJSPlayerState) == true
				&& WJSPlayerState->CurrentGuessCount < WJSPlayerState->MaxGuessCount)
			{
				bIsDraw = false;
				break;
			}
		}

		if (bIsDraw == true)
		{
			ResultMessage = TEXT("Draw...");
		}
	}

	if (ResultMessage.IsEmpty() == true)
	{
		return false;
	}

	for (const TObjectPtr<AWJSPlayerController>& WJSPlayerController
		: AllPlayerControllers)
	{
		if (IsValid(WJSPlayerController) == true)
		{
			WJSPlayerController->NotificationText =
				FText::FromString(
					ResultMessage
					+ TEXT("\n아무 숫자나 입력하여 재시작"));
		}
	}

	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	bIsWaitingForRematch = true;
	RematchReadyPlayers.Empty();
	CurrentTurnPlayerIndex = INDEX_NONE;

	AWJSGameStateBase* WJSGameStateBase = GetGameState<AWJSGameStateBase>();
	if (IsValid(WJSGameStateBase) == true)
	{
		WJSGameStateBase->CurrentTurnPlayerName = TEXT("Game Over");
		WJSGameStateBase->RemainingTurnTime = 0;
	}

	return true;
}

void AWJSGameModeBase::HandleRematchRequest(
	AWJSPlayerController* InRequestingPlayerController,
	const FString& InChatMessageString)
{
	if (IsValid(InRequestingPlayerController) == false)
	{
		return;
	}

	if (InChatMessageString.IsEmpty() == true
		|| InChatMessageString.IsNumeric() == false)
	{
		InRequestingPlayerController->ClientRPCPrintChatMessageString(
			TEXT("재시작하려면 아무 숫자나 입력해주세요."));
		return;
	}

	AWJSPlayerState* WJSPlayerState =
		InRequestingPlayerController->GetPlayerState<AWJSPlayerState>();
	if (IsValid(WJSPlayerState) == false)
	{
		return;
	}

	if (RematchReadyPlayers.Contains(InRequestingPlayerController) == true)
	{
		InRequestingPlayerController->ClientRPCPrintChatMessageString(
			TEXT("이미 재시합에 동의했습니다."));
		return;
	}

	RematchReadyPlayers.Add(InRequestingPlayerController);
	BroadcastChatMessage(
		WJSPlayerState->PlayerNameString
		+ TEXT("이(가) 재시합에 동의하였습니다."));

	int32 ConnectedPlayerCount = 0;
	for (const TObjectPtr<AWJSPlayerController>& WJSPlayerController
		: AllPlayerControllers)
	{
		if (IsValid(WJSPlayerController) == true)
		{
			++ConnectedPlayerCount;
		}
	}

	if (ConnectedPlayerCount > 0
		&& RematchReadyPlayers.Num() >= ConnectedPlayerCount)
	{
		BroadcastChatMessage(TEXT("모든 플레이어가 동의하여 재시합을 시작합니다."));
		ResetGame();
	}
}

void AWJSGameModeBase::BroadcastChatMessage(const FString& InMessageString)
{
	for (const TObjectPtr<AWJSPlayerController>& WJSPlayerController
		: AllPlayerControllers)
	{
		if (IsValid(WJSPlayerController) == true)
		{
			WJSPlayerController->ClientRPCPrintChatMessageString(InMessageString);
		}
	}
}

void AWJSGameModeBase::StartTurn()
{
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);

	if (AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex) == false)
	{
		return;
	}

	AWJSPlayerController* CurrentPlayerController =
		AllPlayerControllers[CurrentTurnPlayerIndex];
	if (IsValid(CurrentPlayerController) == false)
	{
		AdvanceTurn();
		return;
	}

	AWJSPlayerState* WJSPlayerState =
		CurrentPlayerController->GetPlayerState<AWJSPlayerState>();
	AWJSGameStateBase* WJSGameStateBase = GetGameState<AWJSGameStateBase>();
	if (IsValid(WJSPlayerState) == false || IsValid(WJSGameStateBase) == false)
	{
		return;
	}

	WJSPlayerState->bHasGuessedThisTurn = false;
	WJSGameStateBase->CurrentTurnPlayerName = WJSPlayerState->PlayerNameString;
	WJSGameStateBase->RemainingTurnTime = TurnDuration;

	GetWorldTimerManager().SetTimer(
		TurnTimerHandle,
		this,
		&ThisClass::HandleTurnTimer,
		1.0f,
		true);
}

void AWJSGameModeBase::AdvanceTurn()
{
	if (AllPlayerControllers.IsEmpty() == true)
	{
		return;
	}

	const int32 StartIndex =
		(CurrentTurnPlayerIndex + 1) % AllPlayerControllers.Num();
	CurrentTurnPlayerIndex = FindNextAvailablePlayerIndex(StartIndex);
	StartTurn();
}

void AWJSGameModeBase::HandleTurnTimer()
{
	AWJSGameStateBase* WJSGameStateBase = GetGameState<AWJSGameStateBase>();
	if (IsValid(WJSGameStateBase) == false)
	{
		return;
	}

	WJSGameStateBase->RemainingTurnTime =
		FMath::Max(0, WJSGameStateBase->RemainingTurnTime - 1);
	if (WJSGameStateBase->RemainingTurnTime > 0)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	if (AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex) == false)
	{
		return;
	}

	AWJSPlayerController* CurrentPlayerController =
		AllPlayerControllers[CurrentTurnPlayerIndex];
	AWJSPlayerState* WJSPlayerState = IsValid(CurrentPlayerController)
		? CurrentPlayerController->GetPlayerState<AWJSPlayerState>()
		: nullptr;

	if (IsValid(WJSPlayerState) == true
		&& WJSPlayerState->bHasGuessedThisTurn == false
		&& WJSPlayerState->CurrentGuessCount < WJSPlayerState->MaxGuessCount)
	{
		++WJSPlayerState->CurrentGuessCount;
		for (const TObjectPtr<AWJSPlayerController>& WJSPlayerController
			: AllPlayerControllers)
		{
			if (IsValid(WJSPlayerController) == true)
			{
				WJSPlayerController->ClientRPCPrintChatMessageString(
					WJSPlayerState->GetPlayerInfoString()
					+ TEXT(": 시간 초과"));
			}
		}
	}

	if (JudgeGame(CurrentPlayerController, 0) == false)
	{
		AdvanceTurn();
	}
}

int32 AWJSGameModeBase::FindNextAvailablePlayerIndex(int32 InStartIndex) const
{
	if (AllPlayerControllers.IsEmpty() == true)
	{
		return INDEX_NONE;
	}

	for (int32 Offset = 0; Offset < AllPlayerControllers.Num(); ++Offset)
	{
		const int32 Index = (InStartIndex + Offset) % AllPlayerControllers.Num();
		AWJSPlayerController* WJSPlayerController = AllPlayerControllers[Index];
		if (IsValid(WJSPlayerController) == false)
		{
			continue;
		}

		const AWJSPlayerState* WJSPlayerState =
			WJSPlayerController->GetPlayerState<AWJSPlayerState>();
		if (IsValid(WJSPlayerState) == true
			&& WJSPlayerState->CurrentGuessCount < WJSPlayerState->MaxGuessCount)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

void AWJSGameModeBase::IncreaseGuessCount(
	AWJSPlayerController* InChattingPlayerController)
{
	if (IsValid(InChattingPlayerController) == false)
	{
		return;
	}

	AWJSPlayerState* WJSPlayerState =
		InChattingPlayerController->GetPlayerState<AWJSPlayerState>();
	if (IsValid(WJSPlayerState) == true)
	{
		++WJSPlayerState->CurrentGuessCount;
	}
}
