// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#pragma once

#include "CoreMinimal.h"
#include "VolumeAsset/VolumeAsset.h"
#include "VolumeAsset/VolumeInfo.h"

#include "VolumeLoader.generated.h"

UINTERFACE(BlueprintType)
class VRSMOKEVIS_API UVolumeLoader : public UInterface
{
	GENERATED_BODY()
};

DECLARE_LOG_CATEGORY_EXTERN(LogVolumeLoader, Log, All);

/// Interface for all volume loaders.
/// Implement this to make classes that can create UVolumeAssets from arbitrary volumetric data formats.
/// See MHDLoader.h/cpp for an implementation example.
class VRSMOKEVIS_API IVolumeLoader
{
	GENERATED_BODY()
public:
	// Returns a FVolumeInfo without actually creating a volume from the file. Useful for getting info about a volume before loading
	// it.
	virtual FVolumeInfo ParseVolumeInfoFromHeader(const FString& FileName) = 0;

	// Creates a full persistent volume asset from the provided data file.
	virtual UVolumeAsset* CreateVolumeFromFile(const FString& FileName, UPackage* OutPackage) = 0;

	// Loads the raw bytes from the file specified in Info. Detects if file is compressed and loads returns a new uint8 array.
	// Don't forget to delete[] after using.
	static uint8* LoadRawDataFileFromInfo(const FString& FilePath, const FVolumeInfo& Info);

	// Tries to read the provided FileName as a file either in absolute path or relative to game folder.
	static FString ReadFileAsString(const FString& FileName);

	// Gets all files in the specified directory. Extension is optional, if provided, will only return files with the extension.
	static TArray<FString> GetFilesInFolder(FString& Directory, FString& Extension);
	
	// Takes a filename or a path and returns a valid package name.
	// Eg. "/user/somebody/img0.0.1.mhd" 
	// OutPackageName = "img0_0_1" and OutFilePath = "/user/somebody/"
	static void GetValidPackageNameFromFileName(const FString& FullPath, FString& OutFilePath, FString& OutPackageName);

	// Loads the raw data specified in the VolumeInfo and converts it so that it's useable with our raymarching materials.
	virtual uint8* LoadAndConvertData(const FString& FilePath, FVolumeInfo& VolumeInfo);
	
	// Converts raw data read from a Volume file so that it's useable by our materials.
	static uint8* ConvertData(uint8* LoadedArray, FVolumeInfo& VolumeInfo);
};