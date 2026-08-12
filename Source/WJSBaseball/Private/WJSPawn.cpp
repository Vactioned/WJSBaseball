#include "WJSPawn.h"

#include "WJSBaseball.h"

void AWJSPawn::BeginPlay()
{
	Super::BeginPlay();

	const FString NetRoleString = WJSBaseballFunctionLibrary::GetRoleString(this);
	const FString CombinedString = FString::Printf(
		TEXT("WJSPawn::BeginPlay() %s [%s]"),
		*WJSBaseballFunctionLibrary::GetNetModeString(this),
		*NetRoleString);

	WJSBaseballFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}

void AWJSPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	const FString NetRoleString = WJSBaseballFunctionLibrary::GetRoleString(this);
	const FString CombinedString = FString::Printf(
		TEXT("WJSPawn::PossessedBy() %s [%s]"),
		*WJSBaseballFunctionLibrary::GetNetModeString(this),
		*NetRoleString);

	WJSBaseballFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}
