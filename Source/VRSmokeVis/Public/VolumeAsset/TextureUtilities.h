// Contains functions for creating and updating volume texture assets.
// Also contains helper functions for reading DAT files.

#pragma once

#include "VolumeAsset/VolumeAsset.h"
#include "Engine/VolumeTexture.h"

class UTextureRenderTargetVolume;

DECLARE_LOG_CATEGORY_EXTERN(LogTextureUtils, All, All);

class VRSMOKEVIS_API FVolumeTextureToolkit
{
public:
	/** Sets basic volume texture platform data. */
	static void SetVolumeTextureDetails(UVolumeTexture*& OutTexture, const FVector4 Dimensions);

	/** Creates the volume texture 0th mip from the bulkdata provided (or filled with zeros if left null).*/
	static void CreateVolumeTextureMip(
		UVolumeTexture*& OutTexture, const FVolumeInfo& VolumeInfo, uint8* BulkData = nullptr);

	/** Creates a Volume Texture asset with the given name, pixel format and dimensions and fills it with the bulk data
	 * provided. Returns a reference to the created texture in the CreatedTexture param. */
	static bool CreateVolumeTextureAssets(UVolumeTexture*& OutTexture, const FString AssetName,
	                                      const FVolumeInfo& VolumeInfo, UObject* OutPackage,
	                                      uint8* BulkData = nullptr);

	/** Handles the saving of source data to persistent textures. Only works in-editor, as packaged builds no longer
	 * have source data for textures.*/
	static bool CreateVolumeTextureEditorData(UTexture* Texture,
	                                          const FVector4 Dimensions, const uint8* BulkData);

	/** Loads a DAT file into a newly allocated uint8* array. Loads the given number of bytes. Don't forget to delete[]
	 * after storing the data somewhere. */
	static uint8* LoadDatFileIntoArray(const FString FileName, const int64 BytesToLoad);

	/** Converts an array of densities to the resulting transmission. **/
	static void DensityToTransmission(const FVolumeInfo& VolumeInfo, uint8* Array);

	/** Converts an array to an array normalized to the full range of 0-255. */
	static void NormalizeArray(const FVolumeInfo& VolumeInfo, uint8* Array);
};
