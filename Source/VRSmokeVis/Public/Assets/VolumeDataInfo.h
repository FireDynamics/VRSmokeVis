

#pragma once

#include "VolumeDataInfo.generated.h"

// Contains information about the data loaded from the binary data and yaml header file.
USTRUCT(BlueprintType)
struct VRSMOKEVIS_API FVolumeDataInfo
{
	GENERATED_BODY()

	// Name of the volume file that was loaded
	FString DataFileName;

	// Name of the quantity for which data has been loaded
	UPROPERTY(VisibleAnywhere)
	FString Quantity;
	
	// Path to VolumeTextures
	UPROPERTY(VisibleAnywhere)
	FString TextureDir;
	
	// Size of volume/slice in voxels/cells (w equals time in seconds) 
	UPROPERTY(VisibleAnywhere)
	FVector4 Dimensions;
	
	// Size of a voxel/cell in mm (w equals time in seconds) 
	UPROPERTY(VisibleAnywhere)
	FVector4 Spacing;

	// Origin of the volume/slice in simulation
	UPROPERTY(VisibleAnywhere)
	FVector MeshPos;

	// Size of the whole volume/slice in world units (equals Dimensions * Spacing)
	UPROPERTY(VisibleAnywhere)
	FVector WorldDimensions;

	// Lowest value possible for this type of data
	UPROPERTY(VisibleAnywhere)
	float MinValue;

	// Highest value possible for this type of data
	UPROPERTY(VisibleAnywhere)
	float MaxValue;

	// The factor with which the input values have been scaled. A factor of 2 means a read value of 100 corresponds to
	// an original value of 200
	UPROPERTY(VisibleAnywhere)
	float ScaleFactor;
	
	// Returns the number of bytes needed to store this volume/slice
	int64 GetByteSize() const;

	// Returns the number of voxels/cells in this volume/slice
	int64 GetTotalVoxels() const;

	FString ToString() const;
};
