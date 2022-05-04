// Contains functions for creating and updating texture assets

#pragma once

#include "Engine/VolumeTexture.h"


DECLARE_LOG_CATEGORY_EXTERN(LogTextureUtils, All, All);


/**
 * Utility functions for creation of 2D (slices, obsts) and 3D textures (smoke volumes).
 */
class VRSMOKEVIS_API FTextureUtils
{
public:
	/** Sets basic texture platform data. */
	static void SetTextureDetails(UTexture* OutTexture, const FVector4 Dimensions);

	/** Creates the texture's 0th mip from the bulkdata provided.*/
	static void CreateTextureMip(UTexture* OutTexture, const FVector4 Dimensions, uint8* BulkData, const int DataSize);

	
#if WITH_EDITOR
	/** Handles the saving of source data to persistent textures. Only works in-editor, as packaged builds no longer
	 * have source data for textures.*/
	static bool CreateTextureEditorData(UTexture* Texture, const FVector4 Dimensions, const uint8* BulkData);
#endif
	
	/** Creates a Texture asset with the given name, pixel format and dimensions and fills it with the bulk data
	* provided */
	static UTexture2D* CreateTextureAsset(const FString AssetName, const FVector4 Dimensions, UObject* OutPackage,
	                                      uint8* BulkData, const int DataSize);

	/** Creates a Texture asset for a slice with the given name, pixel format and dimensions and fills it with the bulk
	 * data provided */
	static UTexture2D* CreateSliceTextureAsset(const FString AssetName, const FVector4 Dimensions, UObject* OutPackage,
	                                           uint8* BulkData, const int DataSize);

	/** Creates a VolumeTexture asset with the given name, pixel format and dimensions and fills it with the bulk data
	* provided */
	static UVolumeTexture* CreateVolumeAsset(const FString AssetName, const FVector4 Dimensions, UObject* OutPackage,
	                                         uint8* BulkData, const int DataSize);
};
