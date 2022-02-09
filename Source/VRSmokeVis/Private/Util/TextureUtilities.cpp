#include "Util/TextureUtilities.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "Util/ImportUtilities.h"


DEFINE_LOG_CATEGORY(LogTextureUtils);


void FTextureUtils::SetTextureDetails(UTexture* OutTexture, const FVector4 Dimensions)
{
	// Newly created Volume textures have this null'd
	if (!OutTexture->GetRunningPlatformData()[0])
	{
		OutTexture->GetRunningPlatformData()[0] = new FTexturePlatformData();
	}
	// Set Dimensions and Pixel format.
	OutTexture->GetRunningPlatformData()[0]->SizeX = Dimensions.X;
	OutTexture->GetRunningPlatformData()[0]->SizeY = Dimensions.Y;
	OutTexture->GetRunningPlatformData()[0]->SetNumSlices(Dimensions.Z);
	OutTexture->GetRunningPlatformData()[0]->PixelFormat = PF_G8;
	OutTexture->LODGroup = TEXTUREGROUP_8BitData;

	// Set sRGB and streaming to false.
	OutTexture->SRGB = false;
	OutTexture->NeverStream = true;
}

void FTextureUtils::CreateTextureMip(UTexture* OutTexture, const FVector4 Dimensions,
                                     uint8* BulkData, const int DataSize)
{
	// Create the one and only mip in this texture.
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	Mip->SizeX = Dimensions.X;
	Mip->SizeY = Dimensions.Y;
	Mip->SizeZ = Dimensions.Z == 0 ? 1 : Dimensions.Z;

	Mip->BulkData.Lock(LOCK_READ_WRITE);
	// Allocate memory in the mip and copy the actual texture data inside
	uint8* ByteArray = static_cast<uint8*>(Mip->BulkData.Realloc(DataSize));

	check(BulkData != nullptr)
	FMemory::Memcpy(ByteArray, BulkData, DataSize);

	Mip->BulkData.Unlock();

	FTexturePlatformData** PlatformData = OutTexture->GetRunningPlatformData();
	// Newly created textures have this null'd
	if (!PlatformData[0])
	{
		PlatformData[0] = new FTexturePlatformData();
	}
	// Add the new MIP to the list of mips.
	PlatformData[0]->Mips.Add(Mip);
}

bool FTextureUtils::CreateTextureEditorData(UTexture* Texture, const FVector4 Dimensions,
                                            const uint8* BulkData)
{
	Texture->MipGenSettings = TMGS_NoMipmaps;

	// CompressionNone assures the texture is actually saved in the format we want and not DXT1.
	Texture->CompressionNone = true;

	// Otherwise initialize the source struct with our size and bulk data.
	Texture->Source.Init(Dimensions.X, Dimensions.Y, Dimensions.Z == 0 ? 1 : Dimensions.Z, 1, TSF_G8, BulkData);

	return true;
}

UTexture2D* FTextureUtils::CreateTextureAsset(const FString AssetName, const FVector4 Dimensions, UObject* OutPackage,
                                              uint8* BulkData, const int DataSize)
{
	UTexture2D* Texture = NewObject<UTexture2D>(OutPackage, FName(*AssetName),
	                                            RF_Public | RF_Standalone | RF_MarkAsRootSet);

	// Prevent garbage collection of the texture
	Texture->AddToRoot();

	SetTextureDetails(Texture, Dimensions);
	CreateTextureMip(Texture, Dimensions, BulkData, DataSize);
	CreateTextureEditorData(Texture, Dimensions, BulkData);
	Texture->Filter = TF_Default;

	// Update resource, mark that the folder needs to be rescanned and notify editor about asset creation.
	Texture->UpdateResource();
	FAssetRegistryModule::AssetCreated(Texture);

	return Texture;
}

UTexture2D* FTextureUtils::CreateSliceTextureAsset(const FString AssetName, FVector4 Dimensions, UObject* OutPackage,
                                                   uint8* BulkData, const int DataSize)
{
	// The following function call expects the first two dimensions to be the ones describing the texture
	// dimensions, we therefore might have to swap them now
	const bool SwapX = Dimensions.X == 1;
	const bool SwapY = Dimensions.Y == 1;
	if (SwapX)
	{
		const float Tmp = Dimensions.X;
		Dimensions.X = Dimensions.Y;
		Dimensions.Y = Dimensions.Z;
		Dimensions.Z = Tmp;
	}
	if (SwapY)
	{
		const float Tmp = Dimensions.Y;
		Dimensions.Y = Dimensions.Z;
		Dimensions.Z = Tmp;
	}

	UTexture2D* SliceTexture = CreateTextureAsset(AssetName, Dimensions, OutPackage, BulkData, DataSize);

	return SliceTexture;
}

UVolumeTexture* FTextureUtils::CreateVolumeAsset(const FString AssetName, const FVector4 Dimensions,
                                                 UObject* OutPackage,
                                                 uint8* BulkData, const int DataSize)
{
	UVolumeTexture* Texture = NewObject<UVolumeTexture>(OutPackage, FName(*AssetName),
	                                                    RF_Public | RF_Standalone | RF_MarkAsRootSet);

	// Prevent garbage collection of the texture
	Texture->AddToRoot();

	SetTextureDetails(Texture, Dimensions);
	CreateTextureMip(Texture, Dimensions, BulkData, DataSize);
	CreateTextureEditorData(Texture, Dimensions, BulkData);


	// Update resource, mark that the folder needs to be rescanned and notify editor about asset creation.
	Texture->UpdateResource();
	FAssetRegistryModule::AssetCreated(Texture);

	return Texture;
}