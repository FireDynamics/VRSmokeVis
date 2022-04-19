

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FdsActor.generated.h"

UCLASS(Abstract)
class VRSMOKEVIS_API AFdsActor : public AActor
{
	GENERATED_BODY()
	
public:
	/** The loaded asset belonging to this actor */
	UPROPERTY(BlueprintReadOnly)
	class UFdsDataAsset* DataAsset;
};
