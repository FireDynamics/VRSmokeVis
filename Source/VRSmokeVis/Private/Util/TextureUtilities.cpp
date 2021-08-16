// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#include "Util/TextureUtilities.h"

#include "AssetRegistryModule.h"
#include "Util/UtilityShaders.h"

#include <Engine/TextureRenderTargetVolume.h>

DEFINE_LOG_CATEGORY(LogTextureUtils);

FString FVolumeTextureToolkit::MakePackageName(FString AssetName, FString FolderName)
{
	if (FolderName.IsEmpty())
	{
		FolderName = "GeneratedTextures";
	}
	return "/Game" / FolderName / AssetName;
}

void FVolumeTextureToolkit::SetVolumeTextureDetails(UVolumeTexture*& OutTexture, const EPixelFormat PixelFormat, const FVector4 Dimensions)
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
	OutTexture->PlatformData->PixelFormat = PixelFormat;
	// Set sRGB and streaming to false.
	OutTexture->SRGB = false;
	OutTexture->NeverStream = true;
}

void FVolumeTextureToolkit::CreateVolumeTextureMip(
	UVolumeTexture*& OutTexture, const EPixelFormat PixelFormat, const FVector4 Dimensions, uint8* BulkData)
{
	const int PixelByteSize = GPixelFormats[PixelFormat].BlockBytes;
	const long long TotalSize = static_cast<long long>(Dimensions.X) * Dimensions.Y * Dimensions.Z * PixelByteSize;

	// Create the one and only mip in this texture.
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	Mip->SizeX = Dimensions.X;
	Mip->SizeY = Dimensions.Y;
	Mip->SizeZ = Dimensions.Z;

	Mip->BulkData.Lock(LOCK_READ_WRITE);
	// Allocate memory in the mip and copy the actual texture data inside
	uint8* ByteArray = static_cast<uint8*>(Mip->BulkData.Realloc(TotalSize));

	if (BulkData)
	{
		FMemory::Memcpy(ByteArray, BulkData, TotalSize);
	}
	else
	{
		// If no data is provided, memset to zero
		FMemory::Memset(ByteArray, 0, TotalSize);
	}

	Mip->BulkData.Unlock();

	// Newly created Volume textures have this null'd
	if (!OutTexture->PlatformData)
	{
		OutTexture->PlatformData = new FTexturePlatformData();
	}
	// Add the new MIP to the list of mips.
	OutTexture->PlatformData->Mips.Add(Mip);
}

bool FVolumeTextureToolkit::CreateVolumeTextureAsset(UVolumeTexture*& OutTexture, FString AssetName, UPackage* OutPackage,
                                                     const EPixelFormat PixelFormat, const FVector4 Dimensions, uint8* BulkData, bool ShouldUpdateResource)
{
	if (Dimensions.X == 0 || Dimensions.Y == 0 || Dimensions.Z == 0 || Dimensions.W == 0)
	{
		return false;
	}

	UVolumeTexture* VolumeTexture = NewObject<UVolumeTexture>(OutPackage, FName(*AssetName),
	                                                          RF_Public | RF_Standalone | RF_MarkAsRootSet);

	// Prevent garbage collection of the texture
	VolumeTexture->AddToRoot();

	SetVolumeTextureDetails(VolumeTexture, PixelFormat, Dimensions);
	CreateVolumeTextureMip(VolumeTexture, PixelFormat, Dimensions, BulkData);
	CreateVolumeTextureEditorData(VolumeTexture, PixelFormat, Dimensions, BulkData);

	// Update resource, mark that the folder needs to be rescan and notify editor
	// about asset creation.
	if (ShouldUpdateResource)
	{
		VolumeTexture->UpdateResource();
	}

	OutPackage->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(VolumeTexture);
	// Pass out the reference to our brand new texture.
	OutTexture = VolumeTexture;
	return true;
}

bool FVolumeTextureToolkit::UpdateVolumeTextureAsset(UVolumeTexture* VolumeTexture, const EPixelFormat PixelFormat,
	const FVector4 Dimensions, uint8* BulkData, bool ShouldUpdateResource)
{
	if (!VolumeTexture || (Dimensions.X == 0 || Dimensions.Y == 0 || Dimensions.Z == 0))
	{
		return false;
	}

	SetVolumeTextureDetails(VolumeTexture, PixelFormat, Dimensions);
	CreateVolumeTextureMip(VolumeTexture, PixelFormat, Dimensions, BulkData);
	CreateVolumeTextureEditorData(VolumeTexture, PixelFormat, Dimensions, BulkData);

	// Update resource, mark the asset package dirty.
	if (ShouldUpdateResource)
	{
		VolumeTexture->UpdateResource();
	}

	// Notify asset manager that this is dirty now.
	VolumeTexture->MarkPackageDirty();
	return true;
}

bool FVolumeTextureToolkit::CreateVolumeTextureEditorData(
	UTexture* Texture, const EPixelFormat PixelFormat, const FVector4 Dimensions, const uint8* BulkData)
{
	Texture->MipGenSettings = TMGS_NoMipmaps;

	// CompressionNone assures the texture is actually saved in the format we want and not DXT1.
	Texture->CompressionNone = true;

	// If using a format that's not supported as Source format, fail.
	ETextureSourceFormat TextureSourceFormat = PixelFormatToSourceFormat(PixelFormat);
	if (TextureSourceFormat == TSF_Invalid)
	{
		GEngine->AddOnScreenDebugMessage(
			0, 10, FColor::Red, "Trying to create persistent asset with unsupported pixel format!");
		return false;
	}
	// Otherwise initialize the source struct with our size and bulk data.
	Texture->Source.Init(Dimensions.X, Dimensions.Y, Dimensions.Z, 1, TextureSourceFormat, BulkData);
	
	return true;
}

uint8* FVolumeTextureToolkit::LoadRawFileIntoArray(const FString FileName, const int64 BytesToLoad)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	// Try opening as absolute path.
	IFileHandle* FileHandle = PlatformFile.OpenRead(*FileName);

	// If opening as absolute path failed, open as relative to content directory.
	if (!FileHandle)
	{
		FString FullPath = FPaths::ProjectContentDir() + FileName;
		FileHandle = PlatformFile.OpenRead(*FullPath);
	}

	if (!FileHandle)
	{
		UE_LOG(LogTextureUtils, Error, TEXT("Raw file could not be opened."));
		return nullptr;
	}
	else if (FileHandle->Size() < BytesToLoad)
	{
		UE_LOG(LogTextureUtils, Error, TEXT("Raw file is smaller than expected, cannot read volume."));
		delete FileHandle;
		return nullptr;
	}
	else if (FileHandle->Size() > BytesToLoad)
	{
		UE_LOG(LogTextureUtils, Warning,
			TEXT("Raw File is larger than expected,	check your dimensions and pixel format. (nonfatal, but the texture will "
				 "probably be screwed up)"));
	}

	uint8* LoadedArray = new uint8[BytesToLoad];
	FileHandle->Read(LoadedArray, BytesToLoad);
	delete FileHandle;

	return LoadedArray;
}

uint8* FVolumeTextureToolkit::NormalizeArrayByFormat(
	const EVolumeVoxelFormat VoxelFormat, uint8* InArray, const int64 ByteSize, float& OutInMin, float& OutInMax)
{
	switch (VoxelFormat)
	{
		case EVolumeVoxelFormat::UnsignedChar:
			return ConvertArrayToNormalizedArray<uint8, uint8>(InArray, ByteSize, OutInMin, OutInMax);
		case EVolumeVoxelFormat::SignedChar:
			return ConvertArrayToNormalizedArray<int8, uint8>(InArray, ByteSize, OutInMin, OutInMax);
		case EVolumeVoxelFormat::UnsignedShort:
			return ConvertArrayToNormalizedArray<uint16, uint16>(InArray, ByteSize, OutInMin, OutInMax);
		case EVolumeVoxelFormat::SignedShort:
			return ConvertArrayToNormalizedArray<int16, uint16>(InArray, ByteSize, OutInMin, OutInMax);
		case EVolumeVoxelFormat::UnsignedInt:
			return ConvertArrayToNormalizedArray<uint32, uint16>(InArray, ByteSize, OutInMin, OutInMax);
		case EVolumeVoxelFormat::SignedInt:
			return ConvertArrayToNormalizedArray<int32, uint16>(InArray, ByteSize, OutInMin, OutInMax);
		case EVolumeVoxelFormat::Float:
			return ConvertArrayToNormalizedArray<float, uint16>(InArray, ByteSize, OutInMin, OutInMax);
		default:
			ensure(false);
			return nullptr;
	}
}

float* FVolumeTextureToolkit::ConvertArrayToFloat(const EVolumeVoxelFormat VoxelFormat, uint8* InArray, uint64 VoxelCount)
{
	switch (VoxelFormat)
	{
		case EVolumeVoxelFormat::UnsignedChar:
			return ConvertArrayToFloatTemplated<uint8>(InArray, VoxelCount);
		case EVolumeVoxelFormat::SignedChar:
			return ConvertArrayToFloatTemplated<int8>(InArray, VoxelCount);
		case EVolumeVoxelFormat::UnsignedShort:
			return ConvertArrayToFloatTemplated<uint16>(InArray, VoxelCount);
		case EVolumeVoxelFormat::SignedShort:
			return ConvertArrayToFloatTemplated<int16>(InArray, VoxelCount);
		case EVolumeVoxelFormat::UnsignedInt:
			return ConvertArrayToFloatTemplated<uint32>(InArray, VoxelCount);
		case EVolumeVoxelFormat::SignedInt:
			return ConvertArrayToFloatTemplated<int32>(InArray, VoxelCount);
		case EVolumeVoxelFormat::Float:	   // fall through
		default:
			ensure(false);
			return nullptr;
	}
}

void FVolumeTextureToolkit::LoadRawIntoNewVolumeTextureAsset(FString RawFileName, UPackage* PackageName, FString TextureName,
	FIntVector4 Dimensions, uint32 BytesPerVoxel, EPixelFormat OutPixelFormat, UVolumeTexture* LoadedTexture)
{
	const int64 TotalSize = Dimensions.X * Dimensions.Y * Dimensions.Z * BytesPerVoxel;

	uint8* TempArray = LoadRawFileIntoArray(RawFileName, TotalSize);
	if (!TempArray)
	{
		return;
	}

	// Actually create the asset.
	CreateVolumeTextureAsset(LoadedTexture, TextureName, PackageName, OutPixelFormat, Dimensions, TempArray);

	// Delete temp data.
	delete[] TempArray;
}

void FVolumeTextureToolkit::LoadRawIntoVolumeTextureAsset(FString RawFileName, UVolumeTexture* inTexture, FIntVector4 Dimensions,
                                                          const uint32 BytesPerVoxel, const EPixelFormat OutPixelFormat)
{
	const int64 TotalSize = Dimensions.X * Dimensions.Y * Dimensions.Z * BytesPerVoxel;

	uint8* TempArray = LoadRawFileIntoArray(RawFileName, TotalSize);
	if (!TempArray)
	{
		return;
	}

	// Actually update the asset.
	UpdateVolumeTextureAsset(inTexture, OutPixelFormat, Dimensions, TempArray);

	// Delete temp data.
	delete[] TempArray;
}

bool FVolumeTextureToolkit::Create2DTextureTransient(UTexture2D*& OutTexture, EPixelFormat PixelFormat, FIntPoint Dimensions,
	uint8* BulkData, TextureAddress TilingX, TextureAddress TilingY)
{
	int BlockBytes = GPixelFormats[PixelFormat].BlockBytes;
	int TotalBytes = Dimensions.X * Dimensions.Y * BlockBytes;

	UTexture2D* TransientTexture = UTexture2D::CreateTransient(Dimensions.X, Dimensions.Y, PixelFormat);
	TransientTexture->AddressX = TilingX;
	TransientTexture->AddressY = TilingY;

	TransientTexture->SRGB = false;
	TransientTexture->NeverStream = true;

	FTexture2DMipMap& Mip = TransientTexture->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

	if (BulkData)
	{
		FMemory::Memcpy(Data, BulkData, TotalBytes);
	}
	else
	{
		FMemory::Memset(Data, 0, TotalBytes);
	}

	Mip.BulkData.Unlock();

	TransientTexture->UpdateResource();
	OutTexture = TransientTexture;
	return true;
}

ETextureSourceFormat FVolumeTextureToolkit::PixelFormatToSourceFormat(EPixelFormat PixelFormat)
{
	// THIS IS UNTESTED FOR FORMATS OTHER THAN G8, G16 AND R16G16B16A16_SNORM!
	switch (PixelFormat)
	{
		case PF_G8:
		case PF_R8_UINT:
			return TSF_G8;

		case PF_G16:
			return TSF_G16;

		case PF_B8G8R8A8:
			return TSF_BGRA8;

		case PF_R8G8B8A8:
			return TSF_RGBA8;

		case PF_R16G16B16A16_SINT:
		case PF_R16G16B16A16_UINT:
			return TSF_RGBA16;

		case PF_R16G16B16A16_SNORM:
		case PF_R16G16B16A16_UNORM:
			return TSF_RGBA16F;

		default:
			return TSF_Invalid;
	}
}

void FVolumeTextureToolkit::SetupVolumeTexture(
	UVolumeTexture*& OutVolumeTexture, EPixelFormat PixelFormat, FIntVector4 Dimensions, uint8* ConvertedArray)
{
	SetVolumeTextureDetails(OutVolumeTexture, PixelFormat, Dimensions);
	// Actually create the texture MIP.
	CreateVolumeTextureMip(OutVolumeTexture, PixelFormat, Dimensions, ConvertedArray);
	CreateVolumeTextureEditorData(OutVolumeTexture, PixelFormat, Dimensions, ConvertedArray);
	OutVolumeTexture->UpdateResource();
}

void FVolumeTextureToolkit::ClearVolumeTexture(UTextureRenderTargetVolume* RTVolume, float ClearValue)
{
	if (!RTVolume || !RTVolume->Resource || !RTVolume->Resource->TextureRHI)
	{
		return;
	}

	FRHITexture3D* VolumeTextureResource = RTVolume->Resource->TextureRHI->GetTexture3D();

	// Call the actual rendering code on RenderThread.
	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	([VolumeTextureResource, ClearValue](
		 FRHICommandListImmediate& RHICmdList) { ClearVolumeTexture_RenderThread(RHICmdList, VolumeTextureResource, ClearValue); });
}