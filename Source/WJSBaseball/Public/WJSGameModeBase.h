// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WJSGameModeBase.generated.h"

class AWJSPlayerController;

/**
 * 
 */
UCLASS()
class WJSBASEBALL_API AWJSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWJSGameModeBase();

	virtual void OnPostLogin(AController* NewPlayer) override;

	virtual void BeginPlay() override;

	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(
		const FString& InSecretNumberString,
		const FString& InGuessNumberString);

	void PrintChatMessageString(
		AWJSPlayerController* InChattingPlayerController,
		const FString& InChatMessageString);

	void IncreaseGuessCount(AWJSPlayerController* InChattingPlayerController);

	void ResetGame();

	bool JudgeGame(
		AWJSPlayerController* InChattingPlayerController,
		int32 InStrikeCount);

	void StartTurn();

	void AdvanceTurn();

	void HandleTurnTimer();

	void HandleRematchRequest(
		AWJSPlayerController* InRequestingPlayerController,
		const FString& InChatMessageString);

	void BroadcastChatMessage(const FString& InMessageString);

	int32 FindNextAvailablePlayerIndex(int32 InStartIndex) const;

protected:
	FString SecretNumberString;

	UPROPERTY()
	TArray<TObjectPtr<AWJSPlayerController>> AllPlayerControllers;

	UPROPERTY(EditDefaultsOnly)
	int32 TurnDuration;

	int32 CurrentTurnPlayerIndex;

	FTimerHandle TurnTimerHandle;

	bool bIsWaitingForRematch;

	UPROPERTY()
	TSet<TObjectPtr<AWJSPlayerController>> RematchReadyPlayers;
};
