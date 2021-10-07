

#pragma once

#include "Engine/DataAsset.h"
#include "VolumeInfo.h"

#include "SliceAsset.Generated.h"

UCLASS()
class VRSMOKEVIS_API USliceAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	TArray<FAssetData> SliceTextures;
	
	// Holds the general info about the Yaml Volume read from disk.
	UPROPERTY(EditAnywhere)
	FVolumeInfo VolumeInfo;
};
