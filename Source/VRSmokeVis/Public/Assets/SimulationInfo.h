#pragma once

#include "SimulationInfo.generated.h"


/**
 * Contains information about the data loaded from a postprocessed .smv file.
 */
UCLASS(BlueprintType)
class VRSMOKEVIS_API USimulationInfo : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FString SmokeViewOriginalFilePath;

	/** Maps a type [Obst, Slice, Volume] to the path of the original data files (.dat) */
	UPROPERTY(VisibleAnywhere)
	TMap<FString, FString> OriginalDataFilesPath;

	UPROPERTY()
	TArray<FString> ObstPaths;

	UPROPERTY()
	TArray<FString> SlicePaths;

	UPROPERTY()
	TArray<FString> VolumePaths;
};
