#include "Assets/SliceDataInfo.h"

int64 USliceDataInfo::GetByteSize() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z * Dimensions.W;
}

int64 USliceDataInfo::GetTotalCells() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

FString USliceDataInfo::ToString() const
{
	return "Slicename " + ImportName + " details:" + "\nDimensions = " + Dimensions.ToString() +
		"\nSpacing : " + Spacing.ToString() + "\nWorld Size MM : " + Dimensions.ToString();
}
