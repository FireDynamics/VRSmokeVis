// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).
#pragma once

#include "VolumeLoader.h"

#include "MHDLoader.generated.h"
/**
 * IVolumeLoader specialized for reading MHD files.
 */
UCLASS()
class VRSMOKEVIS_API UMHDLoader : public UObject, public IVolumeLoader
{
	GENERATED_BODY()

	static EVolumeVoxelFormat MHDFormatToVoxelFormat(const FString& MHDFormat);

public:
	// Getter for a dummy non-static object. Useful to have non-static so all loaders can use the same interface with
	// virtual methods.
	static UMHDLoader* Get();

	// Returns a FVolumeInfo without actually creating a volume from the file. Useful for getting info about a volume
	// before loading it.
	virtual FVolumeInfo ParseVolumeInfoFromHeader(const FString& FileName) override;

	// Creates a full persistent volume asset from the provided data file.
	virtual UVolumeAsset* CreateVolumeFromFile(const FString& FileName, UPackage* OutPackage) override;

};
