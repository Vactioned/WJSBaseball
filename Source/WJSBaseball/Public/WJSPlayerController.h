// WJSPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WJSPlayerController.generated.h"

class UWJSChatInput;
class UUserWidget;

/**
 * 
 */
UCLASS()
class WJSBASEBALL_API AWJSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWJSPlayerController();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetChatMessageString(const FString& InChatMessageString);

	void PrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCClearChatMessages();

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UWJSChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UWJSChatInput> ChatInputWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TurnTimerWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> TurnTimerWidgetInstance;

	FString ChatMessageString;

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;
};
