// WJSGameStateBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "WJSGameStateBase.generated.h"

UCLASS()
class WJSBASEBALL_API AWJSGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AWJSGameStateBase();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBroadcastLoginMessage(
		const FString& InNameString = FString(TEXT("Player")));

	UFUNCTION(BlueprintPure)
	FText GetTurnInfoText() const;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FString CurrentTurnPlayerName;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 RemainingTurnTime;
};
