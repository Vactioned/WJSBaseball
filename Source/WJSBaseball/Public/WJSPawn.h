// WJSPawn.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WJSPawn.generated.h"

UCLASS()
class WJSBASEBALL_API AWJSPawn : public APawn
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
};
