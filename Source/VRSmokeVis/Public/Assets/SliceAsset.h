#pragma once

#include "FdsDataAsset.h"

#include "SliceAsset.Generated.h"


/**
 * DataAsset containing slice data.
 */
UCLASS()
class VRSMOKEVIS_API USliceAsset : public UFdsDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FAssetData> SliceTextures;
};
