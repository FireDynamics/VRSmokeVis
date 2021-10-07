

#pragma once

#include "VolumeInfo.generated.h"

// Contains information about the volume loaded from the Various volumetric data file formats supported.
USTRUCT(BlueprintType)
struct VRSMOKEVIS_API FVolumeInfo
{
	GENERATED_BODY()

	// Name of the volume file that was loaded, including extension.
	FString DataFileName;

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
	
	// The mass specific extinction coefficient.
	UPROPERTY(EditAnywhere)
	float ExtinctionCoefficient = 1;
	
	// Returns the number of bytes needed to store this Volume.
	int64 GetByteSize() const;

	// Returns the number of voxels in this volume.
	int64 GetTotalVoxels() const;

	FString ToString() const;
};
