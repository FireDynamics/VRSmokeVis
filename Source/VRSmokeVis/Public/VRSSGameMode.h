#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "VRSSGameMode.generated.h"

UCLASS()
class VRSMOKEVIS_API AVRSSGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	AVRSSGameMode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};