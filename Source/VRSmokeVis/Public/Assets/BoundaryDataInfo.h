#pragma once

#include "BoundaryDataInfo.generated.h"

// Contains information about the data loaded from the binary data and yaml header file
USTRUCT(BlueprintType)
struct VRSMOKEVIS_API FBoundaryDataInfo
{
	GENERATED_BODY()

	// Name of the obst file that was loaded
	UPROPERTY(VisibleAnywhere)
	TMap<FString, FString> DataFileNames;

	// Path to Textures for each quantity for which data has been loaded
	UPROPERTY(VisibleAnywhere)
	TMap<FString, FString> TextureDirs;

	// Size of each face in cells (w equals time in seconds) 
	UPROPERTY(VisibleAnywhere)
	TMap<int, FVector4> Dimensions;

	// Size of a cell in mm (w equals time in seconds)
	UPROPERTY(VisibleAnywhere)
	TMap<int, FVector4> Spacings;

	// Size of the each face in world units (equals Dimensions * Spacing)
	UPROPERTY(VisibleAnywhere)
	TMap<int, FVector> WorldDimensions;

	// Lowest value possible for this type of data for each quantity
	UPROPERTY(VisibleAnywhere)
	TMap<FString, float> MinValues;

	// Highest value possible for this type of data for each quantity
	UPROPERTY(VisibleAnywhere)
	TMap<FString, float> MaxValues;

	// The factors with which the input values have been scaled. A factor of 2 means a read value of 100 corresponds to
	// an original value of 200.
	UPROPERTY(VisibleAnywhere)
	TMap<FString, float> ScaleFactors;

	// Returns the number of bytes needed to store a specific face
	int64 GetByteSize(const int Face) const;
};
