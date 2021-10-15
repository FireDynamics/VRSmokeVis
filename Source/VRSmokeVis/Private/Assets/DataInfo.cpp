

#include "Assets/DataInfo.h"

int64 FDataInfo::GetByteSize() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z * Dimensions.W;
}

int64 FDataInfo::GetTotalVoxels() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

FString FDataInfo::ToString() const
{
	FString Text = "File name " + DataFileName + " details:" + "\nDimensions = " + Dimensions.ToString() +
		"\nSpacing : " + Spacing.ToString() + "\nWorld Size MM : " + Dimensions.ToString();
	return Text;
}
