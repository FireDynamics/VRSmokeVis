#pragma once

#include "DataInfo.h"
#include "Engine/DataAsset.h"
#include "Engine/StreamableManager.h"

#include "FdsDataAsset.Generated.h"

UCLASS()
class VRSMOKEVIS_API UFdsDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/** Holds the general info about the Yaml Data read from disk */
	UPROPERTY(EditAnywhere)
	UDataInfo* DataInfo;
};
