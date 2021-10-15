#pragma once

#include "VRSSConfig.generated.h"

UCLASS(BlueprintType)
class VRSMOKEVIS_API UVRSSConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// The ColorMap used for a specific quantity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, UTexture2D*> ColorMaps;

	// The values below which a specific quantity should become fully transparent 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> CutOffValues;

	UVRSSConfig()
	{
		ColorMaps = TMap<FString, UTexture2D*>();
		CutOffValues = TMap<FString, float>();
	}
};
