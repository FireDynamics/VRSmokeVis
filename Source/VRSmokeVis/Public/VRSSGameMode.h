#pragma once

#include "GameFramework/GameMode.h"
#include "VRSSGameMode.generated.h"

UCLASS()
class VRSMOKEVIS_API AVRSSGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	AVRSSGameMode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
