#include "Assets/TextureUtilities.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManagerGeneric.h"
#include "Misc/DefaultValueHelper.h"

#include <Engine/TextureRenderTargetVolume.h>

DEFINE_LOG_CATEGORY(LogTextureUtils);

FString FTextureUtils::ReadFileAsString(const FString& FileName)
{
	FString FileContent;
	// First, try to read FileName as absolute path
	if (FFileHelper::LoadFileToString(FileContent, *FileName))
	{
		return FileContent;
	}

	// If that didn't work, try it as a relative path
	const FString RelativePath = FPaths::ProjectContentDir();
	if (const FString FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelativePath) +
		FileName; FFileHelper::LoadFileToString(FileContent, *FullPath))
	{
		return FileContent;
	}
	UE_LOG(LogTextureUtils, Error, TEXT("Cannot read file path %s either as absolute or as relative path."), *FileName);
	return "";
}

TArray<FString> FTextureUtils::GetFilesInFolder(const FString& Directory, const FString& Extension)
{
	TArray<FString> OutPut;
	OutPut.Empty();
	if (FPaths::DirectoryExists(Directory))
	{
		FFileManagerGeneric::Get().FindFiles(OutPut, *Directory, *Extension);
	}
	return OutPut;
}

void FTextureUtils::SplitPath(const FString& FullPath, FString& OutFilePath, FString& OutPackageName)
{
	FString ExtensionPart;

	FPaths::Split(FullPath, OutFilePath, OutPackageName, ExtensionPart);
	OutPackageName = FPaths::MakeValidFileName(OutPackageName);
	// Dots are not cool in package names -> replace with underscores
	OutPackageName.ReplaceCharInline('.', '_');
}

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

void FTextureUtils::CreateTextureMip(UTexture* OutTexture, const FVolumeInfo& VolumeInfo,
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
	if (!OutTexture->GetRunningPlatformData()[0])
	{
		OutTexture->GetRunningPlatformData()[0] = new FTexturePlatformData();
	}
	// Add the new MIP to the list of mips.
	OutTexture->GetRunningPlatformData()[0]->Mips.Add(Mip);
}

bool FTextureUtils::CreateTextureEditorData(UTexture* Texture, const FVector4 Dimensions,
                                                  const uint8* BulkData)
{
	Texture->MipGenSettings = TMGS_NoMipmaps;

	// CompressionNone assures the texture is actually saved in the format we want and not DXT1.
	Texture->CompressionNone = true;

	// Otherwise initialize the source struct with our size and bulk data.
	Texture->Source.Init(Dimensions.X, Dimensions.Y, Dimensions.Z, 1, TSF_G8, BulkData);

	return true;
}

uint8* FTextureUtils::LoadDatFileIntoArray(const FString FileName, const int64 BytesToLoad)
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

void FTextureUtils::DensityToTransmission(const FVolumeInfo& VolumeInfo, uint8* Array)
{
	// Uses the Beer-Lambert law to convert densities to the corresponding transmission using the extinction coefficient
	// Adding 0.5 before assigning the float value to the uint8 array causes it to round correctly without having to
	// round manually, as the implicit conversion to an integer simply cuts off the fraction.
	float StepSize = 0.001; // VolumeInfo.Spacing.Size3();

	// Multiply StepSize and extinction coefficient only once before looping over the array
	StepSize *= VolumeInfo.ExtinctionCoefficient * -1;

	ParallelFor(VolumeInfo.GetByteSize(), [&](const int Idx)
	{
		Array[Idx] = FMath::Exp(StepSize * Array[Idx]) * 255.f + .5f;
	});
}

void FTextureUtils::NormalizeArray(const FVolumeInfo& VolumeInfo, uint8* Array)
{
	const float ValueRange = 255.f / (VolumeInfo.MaxValue - VolumeInfo.MinValue);
	ParallelFor(VolumeInfo.GetByteSize(), [&](const int Idx)
	{
		Array[Idx] = (Array[Idx] - VolumeInfo.MinValue) * ValueRange;
	});
}

uint8* FTextureUtils::LoadAndConvertVolumeData(const FString& FilePath, const FVolumeInfo& VolumeInfo)
{
	// Load data
	uint8* LoadedArray = LoadDatFileIntoArray(FilePath, VolumeInfo.GetByteSize());
	if (LoadedArray)
	{
		DensityToTransmission(VolumeInfo, LoadedArray);
	}
	return LoadedArray;
}

uint8* FTextureUtils::LoadSliceData(const FString& FilePath, const FVolumeInfo& VolumeInfo)
{
	uint8* LoadedArray =  LoadDatFileIntoArray(FilePath, VolumeInfo.GetByteSize());
	NormalizeArray(VolumeInfo, LoadedArray);
	return LoadedArray;
}

TMap<FString, FVolumeInfo> FTextureUtils::ParseVolumeInfoFromHeader(const FString& FileName)
{
	TMap<FString, FVolumeInfo> VolumeInfos;

	const FString FileString = ReadFileAsString(FileName);
	TArray<FString> Lines;
	FileString.ParseIntoArray(Lines, _T("\n"));

	FString Left, Right, MeshId;

	// DataValMax
	Lines[0].Split(TEXT(": "), &Left, &Right);
	float DataMax;
	FDefaultValueHelper::ParseFloat(Right, DataMax);

	// DataValMin
	Lines[1].Split(TEXT(": "), &Left, &Right);
	float DataMin;
	FDefaultValueHelper::ParseFloat(Right, DataMin);

	// MeshNum
	Lines[2].Split(TEXT(": "), &Left, &Right);
	int NMeshes;
	FDefaultValueHelper::ParseInt(Right, NMeshes);

	UE_LOG(LogTextureUtils, Log, TEXT("Loading volumes, nmeshes: %d"), NMeshes);
	// Meshes
	for (int m = 0; m < NMeshes; ++m)
	{
		FVolumeInfo VolumeInfo;
		VolumeInfo.MaxValue = DataMax;
		VolumeInfo.MinValue = DataMin;

		for (int i = 0; i < 5; ++i)
		{
			Lines[4 + m * 5 + i].Split(TEXT(": "), &Left, &Right);
			if (Left.Contains(TEXT("MeshPos")))
			{
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, VolumeInfo.MeshPos.X);
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, VolumeInfo.MeshPos.Y);
				FDefaultValueHelper::ParseFloat(Right, VolumeInfo.MeshPos.Z);
			}
			else if (Left.Contains(TEXT("Mesh")))
			{
				MeshId = Right.TrimStartAndEnd();
			}
			else if (Left.Contains(TEXT("DimSize")))
			{
				int Val;
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseInt(Left, Val);
				VolumeInfo.Dimensions.W = Val;
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseInt(Left, Val);
				VolumeInfo.Dimensions.X = Val;
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseInt(Left, Val);
				VolumeInfo.Dimensions.Y = Val;
				FDefaultValueHelper::ParseInt(Right, Val);
				VolumeInfo.Dimensions.Z = Val;
			}
			else if (Left.Contains(TEXT("Spacing")))
			{
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, VolumeInfo.Spacing.W);
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, VolumeInfo.Spacing.X);
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, VolumeInfo.Spacing.Y);
				FDefaultValueHelper::ParseFloat(Right, VolumeInfo.Spacing.Z);
			}
			else if (Left.Contains(TEXT("DataFile")))
			{
				VolumeInfo.DataFileName = Right.TrimStartAndEnd();
				UE_LOG(LogTextureUtils, Log, TEXT("Found datafile %s"), *VolumeInfo.DataFileName);
			}
		}

		VolumeInfo.WorldDimensions = VolumeInfo.Spacing * FVector(VolumeInfo.Dimensions);
		VolumeInfos.Add(MeshId, VolumeInfo);
	}

	return VolumeInfos;
}
