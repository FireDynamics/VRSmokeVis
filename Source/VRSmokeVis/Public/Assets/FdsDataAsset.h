#pragma once

#include "DataInfo.h"
#include "Engine/DataAsset.h"
#include "Engine/StreamableManager.h"

#include "FdsDataAsset.Generated.h"

/**
 * Abstract base class for DataAssets containing FDS data.
 */
UCLASS(Abstract)
class VRSMOKEVIS_API UFdsDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/** Holds the general info about the Yaml Data read from disk */
	UPROPERTY(EditAnywhere)
	UDataInfo* DataInfo;
};
