// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#include "VolumeAsset/Loaders/VolumeLoader.h"

#include "HAL/FileManagerGeneric.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"
#include "Util/TextureUtilities.h"

DEFINE_LOG_CATEGORY(LogVolumeLoader)

uint8* IVolumeLoader::LoadRawDataFileFromInfo(const FString& FilePath, const FVolumeInfo& Info)
{
	return FVolumeTextureToolkit::LoadRawFileIntoArray(FilePath + "/" + Info.DataFileName, Info.GetByteSize());
}

FString IVolumeLoader::ReadFileAsString(const FString& FileName)
{
	FString FileContent;
	// First, try to read FileName as absolute path
	if (FFileHelper::LoadFileToString(FileContent, *FileName))
	{
		return FileContent;
	}

	// If that didn't work, try it as a relative path
	FString RelativePath = FPaths::ProjectContentDir();
	FString FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelativePath) + FileName;
	if (FFileHelper::LoadFileToString(FileContent, *FullPath))
	{
		return FileContent;
	}
	UE_LOG(LogVolumeLoader, Error, TEXT("Cannot read file path %s either as absolute or as relative path."), *FileName);
	return "";
}


TArray<FString> IVolumeLoader::GetFilesInFolder(FString& Directory, FString& Extension)
{
	TArray<FString> OutPut;
	OutPut.Empty();
	if (FPaths::DirectoryExists(Directory))
	{
		FFileManagerGeneric::Get().FindFiles(OutPut, *Directory, *Extension);
	}
	return OutPut;
}

void IVolumeLoader::GetValidPackageNameFromFileName(const FString& FullPath, FString& OutFilePath, FString& OutPackageName)
{
	FString ExtensionPart;

	FPaths::Split(FullPath, OutFilePath, OutPackageName, ExtensionPart);
	OutPackageName = FPaths::MakeValidFileName(OutPackageName);
	// Periods are not cool in package names -> replace with underscores
	OutPackageName.ReplaceCharInline('.', '_');
}

uint8* IVolumeLoader::LoadAndConvertData(const FString& FilePath, FVolumeInfo& VolumeInfo)
{
	// Load raw data
	uint8* LoadedArray = FVolumeTextureToolkit::LoadRawFileIntoArray( FPaths::Combine(FilePath, VolumeInfo.DataFileName), VolumeInfo.GetByteSize());
	LoadedArray = ConvertData(LoadedArray, VolumeInfo);
	return LoadedArray;
}

uint8* IVolumeLoader::ConvertData(uint8* LoadedArray, FVolumeInfo& VolumeInfo)
{
	// We want to normalize and cap at G16, perform that normalization
	uint8* ConvertedArray = FVolumeTextureToolkit::NormalizeArrayByFormat(
		VolumeInfo.OriginalFormat, LoadedArray, VolumeInfo.GetByteSize(), VolumeInfo.MinValue, VolumeInfo.MaxValue);
	delete[] LoadedArray;
	LoadedArray = ConvertedArray;

	if (VolumeInfo.BytesPerVoxel > 1)
	{
		VolumeInfo.BytesPerVoxel = 2;
		VolumeInfo.ActualFormat = EVolumeVoxelFormat::UnsignedShort;
	}
	else
	{
		VolumeInfo.ActualFormat = EVolumeVoxelFormat::UnsignedChar;
	}
	return LoadedArray;
}
