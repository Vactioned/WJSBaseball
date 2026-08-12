#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WJSPlayerState.generated.h"

UCLASS()
class WJSBASEBALL_API AWJSPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AWJSPlayerState();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FString GetPlayerInfoString() const;

	UPROPERTY(Replicated)
	FString PlayerNameString;

	UPROPERTY(Replicated)
	int32 CurrentGuessCount;

	UPROPERTY(Replicated)
	int32 MaxGuessCount;

	bool bHasGuessedThisTurn;
};
