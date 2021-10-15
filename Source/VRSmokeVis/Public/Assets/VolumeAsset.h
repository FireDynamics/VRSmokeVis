

#pragma once

#include "Engine/DataAsset.h"
#include "DataInfo.h"

#include "VolumeAsset.Generated.h"

UCLASS()
class VRSMOKEVIS_API UVolumeAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	TArray<FAssetData> VolumeTextures;
	
	// Holds the general info about the Yaml Volume read from disk.
	UPROPERTY(EditAnywhere)
	FDataInfo VolumeInfo;
};
