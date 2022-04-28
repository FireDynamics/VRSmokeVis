#pragma once

#include "FdsMeta.generated.h"

/** Contains information about the data loaded from the binary data and yaml header file. */
USTRUCT(BlueprintType)
struct VRSMOKEVIS_API FFdsMeta
{
	GENERATED_BODY()
	virtual ~FFdsMeta() = default;
	/** Returns the number of bytes needed to store the data for this asset */
	virtual int64 GetByteSize() const PURE_VIRTUAL(UDataInfo::GetByteSize, return 0;); ;

	virtual FString ToString() const PURE_VIRTUAL(UDataInfo::ToString, return FString(););

	/** FDS name of the asset */
	UPROPERTY(VisibleAnywhere)
	FString FdsName;
};
