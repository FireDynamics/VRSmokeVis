// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#pragma once

#include "CoreMinimal.h"
#include "VolumeInfo.generated.h"

/// Voxel format of a loaded volume.
UENUM(BlueprintType)
enum class EVolumeVoxelFormat : uint8
{
	// 1 byte
	UnsignedChar = 0,
	SignedChar = 1,
	// 2 bytes
	UnsignedShort = 2,
	SignedShort = 3,
	// 4 bytes
	UnsignedInt = 4,
	SignedInt = 5,
	// 4 bytes float
	Float = 6
	// #TODO maybe double? Unreal materials don't support them anyways...
};

/// Contains information about the volume loaded from the Various volumetric data file formats supported.
USTRUCT(BlueprintType)
struct VRSMOKEVIS_API FVolumeInfo
{
	GENERATED_BODY()

	/// If true, parsing succeeded. If false, this Volume Info is unusable.
	bool bParseWasSuccessful = false;

	/// Name of the volume file that was loaded, including extension.
	UPROPERTY(VisibleAnywhere)
	FString DataFileName;

	/// Format of voxels loaded from the volume. Does NOT have to match the actual PixelFormat that the VolumeTexture is stored in!
	/// e.g. this could be UChar and VolumeTexture is saved as a EPixelFormat::Float - so don't use for calculating sizes!
	UPROPERTY(VisibleAnywhere)
	EVolumeVoxelFormat OriginalFormat = EVolumeVoxelFormat::SignedShort;

	/// The format we're using after load has been finished. Takes into account being normalized or converted to float.
	UPROPERTY(VisibleAnywhere)
	EVolumeVoxelFormat ActualFormat = EVolumeVoxelFormat::SignedShort;

	// Size of volume in voxels.
	UPROPERTY(VisibleAnywhere)
	FVector4 Dimensions;

	// Size of a voxel in mm.
	UPROPERTY(VisibleAnywhere)
	FVector4 Spacing;

	// Size of the whole volume in mm (equals VoxelDimensions * Spacing)
	UPROPERTY(VisibleAnywhere)
	FVector WorldDimensions;

	// If true, the values in the texture have been normalized from [MinValue, MaxValue] to [0, 1] range.
	UPROPERTY(VisibleAnywhere)
	bool bIsNormalized = false;

	// Lowest value voxel in the volume in the original volume (before normalization).
	UPROPERTY(VisibleAnywhere)
	float MinValue = -1000;

	// Highest value voxel in the volume in the original volume (before normalization).
	UPROPERTY(VisibleAnywhere)
	float MaxValue = 3000;

	// Returns the number of bytes needed to store this Volume.
	int64 GetByteSize() const;

	// Returns the number of voxels in this volume.
	int64 GetTotalVoxels() const;

	// Properties not visible to blueprints (used only when loading)
	// Will reflect the ActualFormat rather than OriginalFormat.
	bool bIsSigned;

	// Will reflect the ActualFormat rather than OriginalFormat.
	size_t BytesPerVoxel;

	// Normalizes an input value from the range [MinValue, MaxValue] to [0,1]. Note that values can be outside of the range,
	// e.g. MinValue - (MaxValue - MinValue) will be normalized to -1.
	float NormalizeValue(float InValue);

	/// Converts a [0,1] normalized value to [Min, Max] range.
	float DenormalizeValue(float InValue);

	/// Normalizes a range to 0-1 depending on the size of the original data.
	float NormalizeRange(float InRange);

	/// Converts a [0,1] normalized range to the range of the original data (e.g. 1 will get converted to (MaxValue - MinValue))
	float DenormalizeRange(float InRange);

	static int32 VoxelFormatByteSize(EVolumeVoxelFormat InFormat);

	static bool IsVoxelFormatSigned(EVolumeVoxelFormat InFormat);

	static EPixelFormat VoxelFormatToPixelFormat(EVolumeVoxelFormat InFormat);

	FString ToString() const;
};
