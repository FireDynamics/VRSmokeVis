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
	
	// The values below which a specific quantity should become fully transparent (slices only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> SliceCutOffValues;

	// The values below which a specific quantity should become fully transparent (obstructions only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> ObstCutOffValues;
	
	
	UVRSSConfig()
	{
		ColorMaps = TMap<FString, UTexture2D*>();
		SliceCutOffValues = TMap<FString, float>();
	}
};
