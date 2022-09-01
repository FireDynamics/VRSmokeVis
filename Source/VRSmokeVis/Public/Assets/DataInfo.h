#pragma once

#include "DataInfo.generated.h"

/**
 * Contains information about the data loaded from the binary data and yaml header file.
 */
UCLASS(Abstract, BlueprintType)
class VRSMOKEVIS_API UDataInfo : public UObject
{
	GENERATED_BODY()

public:
	/** Returns the number of bytes needed to store the data for this asset */
	virtual int64 GetByteSize() const PURE_VIRTUAL(UDataInfo::GetByteSize, return 0;); ;

	virtual FString ToString() const PURE_VIRTUAL(UDataInfo::ToString, return FString(););

	/** FDS name of the asset as it was imported */
	UPROPERTY(VisibleAnywhere)
	FString ImportName;

	/** Actual FDS name of the asset (e.g. id) */
	UPROPERTY(VisibleAnywhere)
	FString FdsName;
};
