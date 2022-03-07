#pragma once

#include "Engine/StreamableManager.h"
#include "VRSSConfig.generated.h"

UCLASS(Config=SimulationProperties)
class VRSMOKEVIS_API UVRSSConfig : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	UTexture2D* GetColorMap(const FString Quantity);
	
	UFUNCTION(BlueprintCallable)
	float GetSliceCutOffValue(const FString Quantity) const;
	
	UFUNCTION(BlueprintCallable)
	float GetObstCutOffValue(const FString Quantity) const;
	
	UFUNCTION(BlueprintCallable)
	FString GetColorMapsPath() const;
	
protected:
	/** The values below which a specific quantity should become fully transparent (slices only) */
	UPROPERTY(Config)
	TMap<FString, float> SliceCutOffValues;

	/** The values below which a specific quantity should become fully transparent (obstructions only) */
	UPROPERTY(Config)
	TMap<FString, float> ObstCutOffValues;

	/** Path to all ColorMap Textures */
	UPROPERTY(Config)
	FString ColorMapsPath;

	/** The ColorMap used for a specific quantity */
	UPROPERTY(Config)
	TMap<FString, FString> ColorMaps;

	FStreamableManager StreamableManager;
};
