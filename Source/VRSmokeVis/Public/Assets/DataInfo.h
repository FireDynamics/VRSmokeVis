

#pragma once

#include "DataInfo.generated.h"

// Contains information about the data loaded from the binary data and yaml header file.
USTRUCT(BlueprintType)
struct VRSMOKEVIS_API FDataInfo
{
	GENERATED_BODY()

	// Name of the volume file that was loaded, including extension.
	FString DataFileName;

	// Name of the quantity for which data has been loaded
	UPROPERTY(VisibleAnywhere)
	FString Quantity;
	
	// Path to VolumeTextures
	UPROPERTY(VisibleAnywhere)
	FString VolumeTextureDir;
	
	// Size of volume in voxels.
	UPROPERTY(VisibleAnywhere)
	FVector4 Dimensions;
	
	// Size of a voxel in mm.
	UPROPERTY(VisibleAnywhere)
	FVector4 Spacing;

	// Origin of the mesh in simulation (fourth Dimension empty).
	UPROPERTY(VisibleAnywhere)
	FVector MeshPos;

	// Size of the whole volume in world units (equals Dimensions * Spacing)
	UPROPERTY(VisibleAnywhere)
	FVector WorldDimensions;

	// Lowest value possible for this type of data.
	UPROPERTY(VisibleAnywhere)
	float MinValue;

	// Highest value possible for this type of data.
	UPROPERTY(VisibleAnywhere)
	float MaxValue;

	// The factor with which the input values have been scaled. A factor of 2 means a read value of 100 corresponds to
	// an original value of 200.
	UPROPERTY(VisibleAnywhere)
	float ScaleFactor;
	
	// The mass specific extinction coefficient.
	UPROPERTY(EditAnywhere)
	float ExtinctionCoefficient = 1;
	
	// Returns the number of bytes needed to store this Volume.
	int64 GetByteSize() const;

	// Returns the number of voxels in this volume.
	int64 GetTotalVoxels() const;

	FString ToString() const;
};
