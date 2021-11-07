

#pragma once

#include "Engine/DataAsset.h"
#include "BoundaryDataInfo.h"

#include "ObstAsset.Generated.h"

/*USTRUCT()
struct FBoundaryAssetData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FAssetData> Faces;
};*/

UCLASS()
class VRSMOKEVIS_API UObstAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Maps a face orientation to the boundary data textures for a specific quantity
	TMap<FString, TMap<int, TArray<FAssetData>>> ObstTextures;
	
	// Holds the general info about the Yaml Data read from disk
	UPROPERTY(EditAnywhere)
	FBoundaryDataInfo ObstInfo;

	// The bounding box of the cuboid defined by the obst
	UPROPERTY(EditAnywhere)
	TArray<float> BoundingBox;
};
