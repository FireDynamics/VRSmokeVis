

#pragma once

#include "VolumeAsset/VolumeAsset.h"
#include "VolumeAsset/VolumeInfo.h"

#include "VolumeLoader.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVolumeLoader, Log, All);

UCLASS()
class VRSMOKEVIS_API UVolumeLoader : public UObject
{
	GENERATED_BODY()
	
public:
	// Getting info about volumes before loading them.
	static TMap<FString, FVolumeInfo> ParseVolumeInfoFromHeader(const FString& FileName);

	// Creates a full persistent volume asset from the provided data file.
	static UVolumeAsset* CreateVolumeFromFile(::FVolumeInfo& VolumeInfo, const ::FString& FileName, ::UObject* Package, const ::FString& MeshName, TArray<
	                                          UVolumeTexture*> OutVolumeTextures);

	// Tries to read the provided FileName as a file either in absolute path or relative to game folder.
	static FString ReadFileAsString(const FString& FileName);

	// Gets all files in the specified directory. Extension is optional, if provided, will only return files with the extension.
	static TArray<FString> GetFilesInFolder(const FString& Directory, const FString& Extension);
	
	// Takes a filename or a path and returns a valid package name.
	// Eg. "/user/somebody/img0.0.1.yaml" 
	// OutPackageName = "img0_0_1" and OutFilePath = "/user/somebody/"
	static void SplitPath(const FString& FullPath, FString& OutFilePath, FString& OutPackageName);

	// Loads the raw data specified in the VolumeInfo and converts it so that it's usable with our raymarching materials.
	static uint8* LoadAndConvertData(const FString& FilePath, const FVolumeInfo& VolumeInfo);
};