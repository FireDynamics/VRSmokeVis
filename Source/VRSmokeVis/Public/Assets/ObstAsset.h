#pragma once

#include "FdsDataAsset.h"

#include "ObstAsset.Generated.h"


/**
 * Struct containing obstruction textures for multiple timesteps for a fixed quantity and orientation.
 */
USTRUCT()
struct FFixedFaceObstTextures
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FAssetData> Textures;
};

/**
 * Struct containing obstruction textures for multiple timesteps for a fixed quantity and variable orientation.
 */
USTRUCT()
struct FFixedQuantityObstTextures
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<int, FFixedFaceObstTextures> ForOrientation;
};

/**
 * DataAsset containing obstruction/boundary data.
 */
UCLASS()
class VRSMOKEVIS_API UObstAsset : public UFdsDataAsset
{
	GENERATED_BODY()

public:
	/** Maps a face orientation to the boundary data textures for a specific quantity */
	UPROPERTY()
	TMap<FString, FFixedQuantityObstTextures> ObstTextures;

	/** The bounding box of the cuboid defined by the obst */
	UPROPERTY(EditAnywhere)
	TArray<float> BoundingBox;
};
