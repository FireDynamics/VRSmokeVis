#pragma once

#include "FdsDataAsset.h"

#include "VolumeAsset.Generated.h"

UCLASS()
class VRSMOKEVIS_API UVolumeAsset : public UFdsDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FAssetData> VolumeTextures;

	// The mass specific extinction coefficient
	UPROPERTY(EditAnywhere)
	float ExtinctionCoefficient = 1;
};
