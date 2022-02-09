#pragma once

#include "SimulationInfo.generated.h"


// Contains information about the data loaded from a postprocessed .smv file
USTRUCT(BlueprintType)
struct VRSMOKEVIS_API FSimulationInfo
{
	GENERATED_BODY()

	TArray<FString> ObstPaths;
	TArray<FString> SlicePaths;
	TArray<FString> VolumePaths;
};
