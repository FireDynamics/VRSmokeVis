#pragma once

#include "VRSSConfig.generated.h"

UCLASS(BlueprintType)
class VRSMOKEVIS_API UVRSSConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// The ColorMap used for a specific slice quantity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, UTexture2D*> SliceColorMaps;
	
	// The values below which a specific quantity should become fully transparent 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> SliceCutOffValues;
	
	// The ColorMap used for a specific boundary data quantity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, UTexture2D*> BoundaryColorMaps;
	
	UVRSSConfig()
	{
		SliceColorMaps = TMap<FString, UTexture2D*>();
		SliceCutOffValues = TMap<FString, float>();
		BoundaryColorMaps = TMap<FString, UTexture2D*>();
	}
};
