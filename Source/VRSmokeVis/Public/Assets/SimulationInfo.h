#pragma once

#include "SimulationInfo.generated.h"


// Contains information about the data loaded from a postprocessed .smv file
UCLASS(BlueprintType)
class VRSMOKEVIS_API USimulationInfo : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString SmokeViewOriginalFilePath;

	UPROPERTY()
	FString OriginalObstFilesPath;

	UPROPERTY()
	FString OriginalSliceFilesPath;

	UPROPERTY()
	FString OriginalVolumeFilesPath;

	UPROPERTY()
	TArray<FString> ObstPaths;

	UPROPERTY()
	TArray<FString> SlicePaths;

	UPROPERTY()
	TArray<FString> VolumePaths;
};
