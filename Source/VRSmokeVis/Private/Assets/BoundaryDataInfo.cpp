

#include "Assets/BoundaryDataInfo.h"

int64 FBoundaryDataInfo::GetByteSize(const int Face) const
{
	return Dimensions[Face].X * Dimensions[Face].Y * Dimensions[Face].W;
}
