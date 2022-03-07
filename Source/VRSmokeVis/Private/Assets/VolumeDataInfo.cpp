

#include "Assets/VolumeDataInfo.h"

int64 FVolumeDataInfo::GetByteSize() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z * Dimensions.W;
}

int64 FVolumeDataInfo::GetTotalVoxels() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

FString FVolumeDataInfo::ToString() const
{
	return "Name " + FdsName + " details:" + "\nDimensions = " + Dimensions.ToString() +
		"\nSpacing : " + Spacing.ToString() + "\nWorld Size MM : " + Dimensions.ToString();
}
