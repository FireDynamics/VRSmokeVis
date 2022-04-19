#pragma once

#include "FdsDataAsset.h"

#include "SliceAsset.Generated.h"

UCLASS()
class VRSMOKEVIS_API USliceAsset : public UFdsDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FAssetData> SliceTextures;
};
