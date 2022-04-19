#include "Assets/BoundaryDataInfo.h"

int64 UBoundaryDataInfo::GetByteSize(const int Face) const
{
	return Dimensions[Face].X * Dimensions[Face].Y * Dimensions[Face].W;
}

int64 UBoundaryDataInfo::GetByteSize() const
{
	int64 TotalSize = 0;
	TArray<int> Faces;
	Dimensions.GetKeys(Faces);
	for (int Face : Faces)
	{
		TotalSize += GetByteSize(Face);
	}
	return TotalSize;
}

FString UBoundaryDataInfo::ToString() const
{
	return "Name " + FdsName;
}
