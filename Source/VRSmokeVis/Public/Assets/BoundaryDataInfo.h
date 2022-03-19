#pragma once

#include "BoundaryDataInfo.generated.h"

USTRUCT()
struct VRSMOKEVIS_API FQuantityDir
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TMap<int, FString> FaceDirs;

	FQuantityDir()
	{
		FaceDirs = TMap<int, FString>();
	}
};

/** Contains information about the data loaded from the binary data and yaml header file */
USTRUCT(BlueprintType)
struct VRSMOKEVIS_API FBoundaryDataInfo
{
	GENERATED_BODY()

	/** Returns the number of bytes needed to store a specific face */
	int64 GetByteSize(const int Face) const;

	FString ToString() const;

	/** FDS name of the obst */
	UPROPERTY(VisibleAnywhere)
	FString ObstName;

	/** Name of the obst files per quantity that were loaded */
	UPROPERTY(VisibleAnywhere)
	TMap<FString, FString> DataFileNames;

	/** Path to Textures for each quantity for each face for which data has been loaded */
	UPROPERTY(VisibleAnywhere)
	TMap<FString, FQuantityDir> TextureDirs;

	/** Size of each face in cells (w equals time in seconds)  - z empty */
	UPROPERTY(VisibleAnywhere)
	TMap<int, FVector4> Dimensions;

	/** Size of a cell in mm (w equals time in seconds) - z empty */
	UPROPERTY(VisibleAnywhere)
	TMap<int, FVector4> Spacings;

	/** Size of the each face in world units (equals Dimensions * Spacing) - z empty */
	UPROPERTY(VisibleAnywhere)
	TMap<int, FVector> WorldDimensions;

	/** Lowest value possible for this type of data for each quantity */
	UPROPERTY(VisibleAnywhere)
	TMap<FString, float> MinValues;

	/** Highest value possible for this type of data for each quantity */
	UPROPERTY(VisibleAnywhere)
	TMap<FString, float> MaxValues;

	/** The factors with which the input values have been scaled. A factor of 2 means a read value of 100 corresponds
	* to an original value of 200. */
	UPROPERTY(VisibleAnywhere)
	TMap<FString, float> ScaleFactors;
};
