// Contains functions for creating and updating volume texture assets.
// Also contains helper functions for reading DAT files.

#pragma once

#include "Assets/VolumeAsset.h"
#include "Assets/ObstAsset.h"
#include "Engine/VolumeTexture.h"
#include "AssetRegistryModule.h"

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
	static void SetTextureDetails(UTexture* OutTexture, const FVector4 Dimensions);

	/** Creates the texture's 0th mip from the bulkdata provided.*/
	static void CreateTextureMip(UTexture* OutTexture, const FVector4 Dimensions, uint8* BulkData, const int DataSize);

	/** Handles the saving of source data to persistent textures. Only works in-editor, as packaged builds no longer
	 * have source data for textures.*/
	static bool CreateTextureEditorData(UTexture* Texture, const FVector4 Dimensions, const uint8* BulkData);

	/** Loads a DAT file into a newly allocated uint8* array. Loads the given number of bytes. Don't forget to delete[]
	 * after storing the data somewhere. */
	static uint8* LoadDatFileIntoArray(const FString FileName, const int64 BytesToLoad);

	/** Converts an array of densities to the resulting transmission. **/
	static void DensityToTransmission(const float ExtinctionCoefficient, const FVolumeDataInfo& DataInfo, uint8* Array);

	/** Normalizes an array to the full range of 0-255 (1 Byte). */
	static void NormalizeArray(const FVolumeDataInfo& DataInfo, uint8* Array);

	/** Loads the raw data specified in the DataInfo and converts it so that it is usable with our raymarching materials. */
	static uint8* LoadAndConvertVolumeData(const float ExtinctionCoefficient, const FString& FilePath, const FVolumeDataInfo& DataInfo);

	/** Loads the raw data specified in the DataInfo. */
	static uint8* LoadSliceData(const FString& FilePath, const FVolumeDataInfo& DataInfo);
	
	/** Loads the raw data specified in the DataInfo. */
	static uint8* LoadObstData(const FString& FilePath, const FBoundaryDataInfo& DataInfo);
	
	/** Getting info about slices or volumes before loading them. */
	static TMap<FString, FVolumeDataInfo> ParseSliceVolumeDataInfoFromHeader(const FString& FileName);

	/** Getting info about obstructions before loading them. */
	static FBoundaryDataInfo ParseObstDataInfoFromHeader(const FString& FileName, TArray<float> &BoundingBoxOut);

	/** Creates a Texture asset with the given name, pixel format and dimensions and fills it with the bulk data
	* provided. Returns a reference to the created texture in the CreatedTexture param. */
	template <typename T>
	static bool CreateTextureAssets(T*& OutTexture, const FString AssetName, const FVector4 Dimensions,
	                                UObject* OutPackage, uint8* BulkData, const int DataSize)
	{
		if (Dimensions.X == 0 || Dimensions.Y == 0 || Dimensions.Z == 0 || Dimensions.W == 0)
		{
			return false;
		}

		T* Texture = NewObject<T>(OutPackage, FName(*AssetName), RF_Public | RF_Standalone | RF_MarkAsRootSet);

		// Prevent garbage collection of the texture
		Texture->AddToRoot();

		SetTextureDetails(Texture, Dimensions);
		CreateTextureMip(Texture, Dimensions, BulkData, DataSize);
		CreateTextureEditorData(Texture, Dimensions, BulkData);

		// Update resource, mark that the folder needs to be rescanned and notify editor about asset creation.
		Texture->UpdateResource();

		FAssetRegistryModule::AssetCreated(Texture);
		// Pass out the reference to our brand new texture.
		OutTexture = Texture;
		return true;
	}
};
