

#pragma once

#include "Engine/DataAsset.h"
#include "VolumeDataInfo.h"

#include "VolumeAsset.Generated.h"

UCLASS()
class VRSMOKEVIS_API UVolumeAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FAssetData> VolumeTextures;
	
	// The mass specific extinction coefficient
	UPROPERTY(EditAnywhere)
	float ExtinctionCoefficient = 1;
	
	// Holds the general info about the Yaml Data read from disk.
	UPROPERTY(EditAnywhere)
	FVolumeDataInfo VolumeInfo;
};
