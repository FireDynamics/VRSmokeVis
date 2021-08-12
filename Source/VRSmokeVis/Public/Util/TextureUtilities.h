// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

// Contains functions for creating and updating volume texture assets.
// Also contains helper functions for reading RAW files.

#pragma once

#include "CoreMinimal.h"
#include "VolumeAsset/VolumeAsset.h"
#include "Engine/VolumeTexture.h"

class UTextureRenderTargetVolume;

DECLARE_LOG_CATEGORY_EXTERN(LogTextureUtils, All, All);

class VRSMOKEVIS_API FVolumeTextureToolkit
{
public:
	/** Makes a package name from Asset and Folder name. Puts asset into "Generated" folder if folder is left empty.*/
	static FString MakePackageName(FString AssetName, FString FolderName);

	/** Sets basic volume texture platform data. */
	static void SetVolumeTextureDetails(UVolumeTexture*& OutTexture, EPixelFormat PixelFormat, FVector4 Dimensions);

	/** Creates the volume texture 0th mip from the bulkdata provided (or filled with zeros if left null).*/
	static void CreateVolumeTextureMip(
		UVolumeTexture*& OutTexture, EPixelFormat PixelFormat, FVector4 Dimensions, uint8* BulkData = nullptr);

	/** Creates a Volume Texture asset with the given name, pixel format and
	  dimensions and fills it with the bulk data provided.
	  Returns a reference to the created texture in the CreatedTexture param.
	*/
	static bool CreateVolumeTextureAsset(UVolumeTexture*& OutTexture, FString AssetName, UPackage* OutPackage,
	                                     EPixelFormat PixelFormat, FVector4 Dimensions, uint8* BulkData = nullptr,
	                                     bool ShouldUpdateResource = true);

	/** Updates the provided Volume Texture asset to have the provided format,
	 * dimensions and pixel data*/
	static bool UpdateVolumeTextureAsset(UVolumeTexture* VolumeTexture, EPixelFormat PixelFormat,
	                                     FVector4 Dimensions, uint8* BulkData = nullptr,
	                                     bool ShouldUpdateResource = true);

	/** Handles the saving of source data to persistent textures. Only works
	 in-editor, as packaged builds no longer have source data for textures.*/
	static bool CreateVolumeTextureEditorData(UTexture* Texture, const EPixelFormat PixelFormat,
	                                          const FVector4 Dimensions, const uint8* BulkData);

	/** Loads a RAW file into a newly allocated uint8* array. Loads the given number
	 * of bytes. Don't forget to delete[] after storing the data somewhere.*/
	static uint8* LoadRawFileIntoArray(const FString FileName, const int64 ByteSize);

	/** Normalizes an array InArray to maximum G16 type. If the InType is 8bit, normalizes to G8. Creates a new array, user is
	   responsible for deleting that. The type of data going in is determined by a Format name used in .mhd files - e.g.
	   "MET_SHORT".*/
	static uint8* NormalizeArrayByFormat(const EVolumeVoxelFormat VoxelFormat, uint8* InArray,
	                                     const int64 ArrayByteSize, float& OutOriginalMin, float& OutOriginalMax);

	/** Loads a RAW file into a newly created Volume Texture Asset. Will output error log messages
	 * and return if unsuccessful.
	 * @param RawFileName is supposed to be the absolute path of where the raw file can be found.
	 * @param PackageName Output package of the new asset.
	 * @param TextureName Output asset name.
	 * @param Dimensions Size of the volume.
	 * @param BytesPerVoxel Bytes per voxel.
	 * @param OutPixelFormat Pixel format of the generated volume texture.
	 * @param LoadedTexture is supposed to be the absolute path of where the raw file can be found.
	 */
	static void LoadRawIntoNewVolumeTextureAsset(FString RawFileName, UPackage* PackageName, FString TextureName,
	                                             FIntVector4 Dimensions, uint32 BytesPerVoxel,
	                                             EPixelFormat OutPixelFormat, UVolumeTexture* LoadedTexture);

	/** Loads a RAW file into a provided Volume Texture. Will output error log messages
	 * and return if unsuccessful */
	static void LoadRawIntoVolumeTextureAsset(FString RawFileName, UVolumeTexture* inTexture, FIntVector4 Dimensions,
	                                          uint32 BytesPerVoxel, EPixelFormat OutPixelFormat);
	
	/** Creates a transient 2D Texture (no asset name, cannot be saved)*/
	static bool Create2DTextureTransient(UTexture2D*& OutTexture, EPixelFormat PixelFormat, FIntPoint Dimensions,
		uint8* BulkData = nullptr, TextureAddress TilingX = TA_Clamp, TextureAddress TilingY = TA_Clamp);

	
	/** Converts an array to an array normalized on the range of the OutType, based on the minimum and maximum values
		found in the InArray, when cast to the type InType.*/
	template <typename InType, typename OutType>
	static uint8* ConvertArrayToNormalizedArray(
		uint8* InArray, unsigned long ByteSize, float& OutOriginalMin, float& OutOriginalMax);

	/// Function to convert from arbitrary type T of data to float.
	/// Used when you want to keep the original values and use FLOAT_32 texture.
	template <class T>
	static float* ConvertArrayToFloatTemplated(uint8* Data, int32 VoxelCount);;

	static float* ConvertArrayToFloat(const EVolumeVoxelFormat VoxelFormat, uint8* InArray, uint64 VoxelCount);

	/** Tells you which source format to use for a texture's source according to the
	 * Pixel format. */
	static ETextureSourceFormat PixelFormatToSourceFormat(EPixelFormat PixelFormat);

	/** Performs full setup of a volume texture depending on the provided parameters. Just a convenience function that calls
		SetVolumeTextureDetails, CreateVolumeTextureMip and CreateVolumeTextureEditorData. In this order. */
	static void SetupVolumeTexture(
		UVolumeTexture*& OutVolumeTexture, EPixelFormat PixelFormat, FIntVector4 Dimensions, uint8* InSourceArray);

	/** Clears a Volume Texture. */
	UFUNCTION(BlueprintCallable, Category = " Volume Texture Utilities")
	static void ClearVolumeTexture(UTextureRenderTargetVolume* RTVolume, float ClearValue);
};

template <typename InType, typename OutType>
uint8* FVolumeTextureToolkit::ConvertArrayToNormalizedArray(uint8* InArray, unsigned long ByteSize,
                                                            float& OutOriginalMin, float& OutOriginalMax)
{
	InType* InCastArray = reinterpret_cast<InType*>(InArray);
	const unsigned long ElementCount = ByteSize / sizeof(InType);

	InType InMin = std::numeric_limits<InType>::max();
	InType InMax = std::numeric_limits<InType>::min();

	for (unsigned long i = 0; i < ElementCount; i++)
	{
		if (InCastArray[i] < InMin)
		{
			InMin = InCastArray[i];
		}
		if (InCastArray[i] > InMax)
		{
			InMax = InCastArray[i];
		}
	}

	OutType* OutArray = new OutType[ElementCount];

	// Normalize all values to the full range of the OutType.
	//
	// e.g. - minimum value was -50, max value was 200
	// OutType is uint16 ->
	// -50 will map to 0
	// 200 will map to 65535

	OutType OutMin = std::numeric_limits<OutType>::min();
	OutType OutMax = std::numeric_limits<OutType>::max();

	// #TODO this could use a ParallelFor
	for (unsigned long i = 0; i < ElementCount; i++)
	{
		float Normalized = (static_cast<float>(InCastArray[i]) - InMin) / (static_cast<float>(InMax) - InMin);
		OutArray[i] = OutMin + (Normalized * (OutMax - OutMin));
	}

	// Output the original min and max.
	OutOriginalMin = static_cast<float>(InMin);
	OutOriginalMax = static_cast<float>(InMax);

	return reinterpret_cast<uint8*>(OutArray);
}

template <class T>
float* FVolumeTextureToolkit::ConvertArrayToFloatTemplated(uint8* Data, int32 VoxelCount)
{
	T* TypedData = reinterpret_cast<T*>(Data);
	float* NewData = new float[VoxelCount];

	const int32 NumWorkerThreads = FTaskGraphInterface::Get().GetNumWorkerThreads();
	int32 NumVoxelsPerThread = VoxelCount / NumWorkerThreads;

	ParallelFor(NumWorkerThreads, [&](int32 ThreadId)
	{
		int32 index = 0;
		for (int32 i = 0; i < NumVoxelsPerThread; i++)
		{
			index = (NumVoxelsPerThread * ThreadId) + i;
			NewData[index] = static_cast<float>(TypedData[index]);
		}
	});

	// Finish the leftovers
	for (int32 index = NumWorkerThreads * NumVoxelsPerThread; index < VoxelCount; index++)
	{
		NewData[index] = static_cast<float>(TypedData[index]);
	}
	return NewData;
}
