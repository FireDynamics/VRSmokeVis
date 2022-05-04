

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FdsActor.generated.h"


/**
 * Abstract base class for any actor that displays data from FDS, such as slices, obstructions or smoke volumes.
 */
UCLASS(Abstract)
class VRSMOKEVIS_API AFdsActor : public AActor
{
	GENERATED_BODY()
	
public:
	/** The loaded asset belonging to this actor */
	UPROPERTY(BlueprintReadOnly)
	class UFdsDataAsset* DataAsset;
};
