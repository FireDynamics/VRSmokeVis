#include "VolumeAsset/TextureUtilities.h"

#include "AssetRegistryModule.h"

#include <Engine/TextureRenderTargetVolume.h>

DEFINE_LOG_CATEGORY(LogTextureUtils);

void FVolumeTextureToolkit::SetVolumeTextureDetails(UVolumeTexture*& OutTexture, const FVector4 Dimensions)
{
	// Newly created Volume textures have this null'd
	if (!OutTexture->PlatformData)
	{
		OutTexture->PlatformData = new FTexturePlatformData();
	}
	// Set Dimensions and Pixel format.
	OutTexture->PlatformData->SizeX = Dimensions.X;
	OutTexture->PlatformData->SizeY = Dimensions.Y;
	OutTexture->PlatformData->SetNumSlices(Dimensions.Z);
	OutTexture->PlatformData->PixelFormat = PF_G8;
	// Set sRGB and streaming to false.
	OutTexture->SRGB = false;
	OutTexture->NeverStream = true;
}

void FVolumeTextureToolkit::CreateVolumeTextureMip(UVolumeTexture*& OutTexture, const FVolumeInfo& VolumeInfo,
                                                   uint8* BulkData)
{
	const auto TotalSize = VolumeInfo.GetByteSize() / VolumeInfo.Dimensions.W;

	// Create the one and only mip in this texture.
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	Mip->SizeX = VolumeInfo.Dimensions.X;
	Mip->SizeY = VolumeInfo.Dimensions.Y;
	Mip->SizeZ = VolumeInfo.Dimensions.Z;

	Mip->BulkData.Lock(LOCK_READ_WRITE);
	// Allocate memory in the mip and copy the actual texture data inside
	uint8* ByteArray = static_cast<uint8*>(Mip->BulkData.Realloc(TotalSize));

	check(BulkData != nullptr)
	FMemory::Memcpy(ByteArray, BulkData, TotalSize);

	Mip->BulkData.Unlock();

	// Newly created Volume textures have this null'd
	if (!OutTexture->PlatformData)
	{
		OutTexture->PlatformData = new FTexturePlatformData();
	}
	// Add the new MIP to the list of mips.
	OutTexture->PlatformData->Mips.Add(Mip);
}

bool FVolumeTextureToolkit::CreateVolumeTextureAssets(UVolumeTexture*& OutTexture, const FString AssetName,
                                                      const FVolumeInfo& VolumeInfo, UObject* OutPackage, uint8*
                                                      BulkData)
{
	const FVector4 Dimensions = VolumeInfo.Dimensions;
	if (Dimensions.X == 0 || Dimensions.Y == 0 || Dimensions.Z == 0 || Dimensions.W == 0)
	{
		return false;
	}

	UVolumeTexture* VolumeTexture = NewObject<UVolumeTexture>(OutPackage, FName(*AssetName),
	                                                          RF_Public | RF_Standalone | RF_MarkAsRootSet);

	// Prevent garbage collection of the texture
	VolumeTexture->AddToRoot();

	SetVolumeTextureDetails(VolumeTexture, Dimensions);
	CreateVolumeTextureMip(VolumeTexture, VolumeInfo, BulkData);
	CreateVolumeTextureEditorData(VolumeTexture, Dimensions, BulkData);

	// Update resource, mark that the folder needs to be rescan and notify editor about asset creation.
	VolumeTexture->UpdateResource();

	FAssetRegistryModule::AssetCreated(VolumeTexture);
	// Pass out the reference to our brand new texture.
	OutTexture = VolumeTexture;
	return true;
}

bool FVolumeTextureToolkit::CreateVolumeTextureEditorData(UTexture* Texture, const FVector4 Dimensions,
                                                          const uint8* BulkData)
{
	Texture->MipGenSettings = TMGS_NoMipmaps;

	// CompressionNone assures the texture is actually saved in the format we want and not DXT1.
	Texture->CompressionNone = true;

	// Otherwise initialize the source struct with our size and bulk data.
	Texture->Source.Init(Dimensions.X, Dimensions.Y, Dimensions.Z, 1, TSF_G8, BulkData);

	return true;
}

uint8* FVolumeTextureToolkit::LoadDatFileIntoArray(const FString FileName, const int64 BytesToLoad)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*FileName);
	if (!FileHandle)
	{
		UE_LOG(LogTextureUtils, Error, TEXT("Data file %s could not be opened."), *FileName);
		return nullptr;
	}
	if (FileHandle->Size() < BytesToLoad)
	{
		UE_LOG(LogTextureUtils, Error,
		       TEXT("Data file %s does not have the expected size of (at least) %d bytes, aborting."), *FileName,
		       BytesToLoad);
		delete FileHandle;
		return nullptr;
	}

	uint8* LoadedArray = new uint8[BytesToLoad];
	FileHandle->Read(LoadedArray, BytesToLoad);
	delete FileHandle;

	return LoadedArray;
}

void FVolumeTextureToolkit::NormalizeArray(const FVolumeInfo& VolumeInfo, uint8* Array)
{
	const float ValueRange = 255.f / (VolumeInfo.MaxValue - VolumeInfo.MinValue);
	ParallelFor(VolumeInfo.GetByteSize(), [&](const int Idx)
	{
		Array[Idx] = (Array[Idx] - VolumeInfo.MinValue) * ValueRange;
	});
}
