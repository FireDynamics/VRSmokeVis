#pragma once

#include "FdsDataAsset.h"

#include "ObstAsset.Generated.h"


UCLASS()
class VRSMOKEVIS_API UObstAsset : public UFdsDataAsset
{
	GENERATED_BODY()

public:
	// Maps a face orientation to the boundary data textures for a specific quantity
	// Todo: Check for UPROPERTY necessity
	TMap<FString, TMap<int, TArray<FAssetData>>> ObstTextures;

	// The bounding box of the cuboid defined by the obst
	UPROPERTY(EditAnywhere)
	TArray<float> BoundingBox;
};
