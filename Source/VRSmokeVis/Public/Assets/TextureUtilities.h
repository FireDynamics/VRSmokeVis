// Contains functions for creating and updating volume texture assets.
// Also contains helper functions for reading DAT files.

#pragma once

#include "Assets/VolumeAsset.h"
#include "Engine/VolumeTexture.h"

class UTextureRenderTargetVolume;

DECLARE_LOG_CATEGORY_EXTERN(LogTextureUtils, All, All);

class VRSMOKEVIS_API FTextureUtils
{
public:
	// Tries to read the provided FileName as a file either in absolute path or relative to game folder.
	static FString ReadFileAsString(const FString& FileName);

	// Gets all files in the specified directory. Extension is optional, if provided, will only return files with the extension.
	static TArray<FString> GetFilesInFolder(const FString& Directory, const FString& Extension);
	
	// Takes a filename or a path and returns a valid package name.
	// Eg. "/user/somebody/img0.0.1.yaml" 
	// OutPackageName = "img0_0_1" and OutFilePath = "/user/somebody/"
	static void SplitPath(const FString& FullPath, FString& OutFilePath, FString& OutPackageName);
	
	/** Sets basic texture platform data. */
	static void SetTextureDetails(UVolumeTexture* OutTexture, const FVector4 Dimensions);

	/** Creates the texture's 0th mip from the bulkdata provided.*/
	static void CreateTextureMip(UVolumeTexture* OutTexture, const FVolumeInfo& VolumeInfo, uint8* BulkData);

	/** Creates a Volume Texture asset with the given name, pixel format and dimensions and fills it with the bulk data
	 * provided. Returns a reference to the created texture in the CreatedTexture param. */
	template<typename T>
	static bool CreateTextureAssets(T* OutTexture, const FString AssetName,
	                                      const FVolumeInfo& VolumeInfo, UObject* OutPackage,
	                                      uint8* BulkData = nullptr);

	/** Handles the saving of source data to persistent textures. Only works in-editor, as packaged builds no longer
	 * have source data for textures.*/
	static bool CreateTextureEditorData(UTexture* Texture,
	                                          const FVector4 Dimensions, const uint8* BulkData);

	/** Loads a DAT file into a newly allocated uint8* array. Loads the given number of bytes. Don't forget to delete[]
	 * after storing the data somewhere. */
	static uint8* LoadDatFileIntoArray(const FString FileName, const int64 BytesToLoad);

	/** Converts an array of densities to the resulting transmission. **/
	static void DensityToTransmission(const FVolumeInfo& VolumeInfo, uint8* Array);

	/** Converts an array to an array normalized to the full range of 0-255. */
	static void NormalizeArray(const FVolumeInfo& VolumeInfo, uint8* Array);

	// Loads the raw data specified in the VolumeInfo and converts it so that it's usable with our raymarching materials.
	static uint8* LoadAndConvertVolumeData(const FString& FilePath, const FVolumeInfo& VolumeInfo);

	// Loads the raw data specified in the VolumeInfo.
	static uint8* LoadSliceData(const FString& FilePath, const FVolumeInfo& VolumeInfo);
		
	// Getting info about volumes before loading them.
	static TMap<FString, FVolumeInfo> ParseVolumeInfoFromHeader(const FString& FileName);
};
