

#include "VolumeAsset/VolumeInfo.h"

int64 FVolumeInfo::GetByteSize() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z * Dimensions.W;
}

int64 FVolumeInfo::GetTotalVoxels() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

FString FVolumeInfo::ToString() const
{
	FString text = "File name " + DataFileName + " details:" + "\nDimensions = " + Dimensions.ToString() +
		"\nSpacing : " + Spacing.ToString() + "\nWorld Size MM : " + Dimensions.ToString();
	return text;
}
