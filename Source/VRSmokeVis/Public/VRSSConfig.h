#pragma once

#include "VRSSConfig.generated.h"

UCLASS(Config=SimulationProperties)
class VRSMOKEVIS_API UVRSSConfig : public UObject
{
	GENERATED_BODY()
	
public:
	UTexture2D* GetColorMap(const FString Quantity) const;
	
	/** The values below which a specific quantity should become fully transparent (slices only) */
	UPROPERTY(Config)
	TMap<FString, float> SliceCutOffValues;

	/** The values below which a specific quantity should become fully transparent (obstructions only) */
	UPROPERTY(Config)
	TMap<FString, float> ObstCutOffValues;

	UPROPERTY(Config)
	FString ColorMapsPath;

protected:
	/** The ColorMap used for a specific quantity */
	UPROPERTY(Config)
	TMap<FString, FString> ColorMaps;

	struct FStreamableManager* StreamableManager;
};
