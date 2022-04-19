#include "Assets/VolumeDataInfo.h"

int64 UVolumeDataInfo::GetByteSize() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z * Dimensions.W;
}

int64 UVolumeDataInfo::GetTotalVoxels() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

FString UVolumeDataInfo::ToString() const
{
	return "Volumename " + FdsName + " details:" + "\nDimensions = " + Dimensions.ToString() +
		"\nSpacing : " + Spacing.ToString() + "\nWorld Size MM : " + Dimensions.ToString();
}
