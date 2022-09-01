#pragma once

#include "Assets/DataInfo.h"
#include "SliceDataInfo.generated.h"

/**
 * Contains information about the data loaded from the binary data and yaml header file.
 */
UCLASS(BlueprintType)
class VRSMOKEVIS_API USliceDataInfo : public UDataInfo
{
	GENERATED_BODY()

public:
	/** Returns the number of bytes needed to store this slice */
	virtual int64 GetByteSize() const override;

	/** Returns the number of cells in this slice */
	int64 GetTotalCells() const;

	virtual FString ToString() const override;
	
	/** Name of the slice file that was loaded */
	UPROPERTY(VisibleAnywhere)
	FString DataFileName;

	/** If the slice contains cell-centered or face-centered data */
	UPROPERTY(VisibleAnywhere)
	bool CellCentered;
	
	/** Name of the quantity for which data has been loaded */
	UPROPERTY(VisibleAnywhere)
	FString Quantity;

	/** Path to SliceTextures */
	UPROPERTY(VisibleAnywhere)
	FString TextureDir;

	/** Size of slice in cells (w equals time in seconds)  */
	UPROPERTY(VisibleAnywhere)
	FVector4 Dimensions;

	/** Size of a cell in mm (w equals time in seconds)  */
	UPROPERTY(VisibleAnywhere)
	FVector4 Spacing;

	/** Origin of the slice in simulation */
	UPROPERTY(VisibleAnywhere)
	FVector MeshPos;

	/** Size of the whole slice in world units (equals Dimensions * Spacing) */
	UPROPERTY(VisibleAnywhere)
	FVector WorldDimensions;

	/** Lowest value possible for this type of data */
	UPROPERTY(VisibleAnywhere)
	float MinValue;

	/** Highest value possible for this type of data */
	UPROPERTY(VisibleAnywhere)
	float MaxValue;

	/** The factor with which the input values have been scaled. A factor of 2 means a read value of 100 corresponds to
	* an original value of 200 */
	UPROPERTY(VisibleAnywhere)
	float ScaleFactor;
};
