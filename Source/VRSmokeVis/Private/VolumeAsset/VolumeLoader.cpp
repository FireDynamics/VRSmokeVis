#include "VolumeAsset/VolumeLoader.h"

#include "HAL/FileManagerGeneric.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"
#include "VolumeAsset/TextureUtilities.h"
#include "Misc/DefaultValueHelper.h"

DEFINE_LOG_CATEGORY(LogVolumeLoader)

FString UVolumeLoader::ReadFileAsString(const FString& FileName)
{
	FString FileContent;
	// First, try to read FileName as absolute path
	if (FFileHelper::LoadFileToString(FileContent, *FileName))
	{
		return FileContent;
	}

	// If that didn't work, try it as a relative path
	const FString RelativePath = FPaths::ProjectContentDir();
	const FString FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelativePath) + FileName;
	if (FFileHelper::LoadFileToString(FileContent, *FullPath))
	{
		return FileContent;
	}
	UE_LOG(LogVolumeLoader, Error, TEXT("Cannot read file path %s either as absolute or as relative path."), *FileName);
	return "";
}

TArray<FString> UVolumeLoader::GetFilesInFolder(const FString& Directory, const FString& Extension)
{
	TArray<FString> OutPut;
	OutPut.Empty();
	if (FPaths::DirectoryExists(Directory))
	{
		FFileManagerGeneric::Get().FindFiles(OutPut, *Directory, *Extension);
	}
	return OutPut;
}

void UVolumeLoader::SplitPath(const FString& FullPath, FString& OutFilePath, FString& OutPackageName)
{
	FString ExtensionPart;

	FPaths::Split(FullPath, OutFilePath, OutPackageName, ExtensionPart);
	OutPackageName = FPaths::MakeValidFileName(OutPackageName);
	// Dots are not cool in package names -> replace with underscores
	OutPackageName.ReplaceCharInline('.', '_');
}

uint8* UVolumeLoader::LoadAndConvertData(const FString& FilePath, const FVolumeInfo& VolumeInfo)
{
	// Load data
	uint8* LoadedArray = FVolumeTextureToolkit::LoadDatFileIntoArray(FilePath, VolumeInfo.GetByteSize());
	if (LoadedArray) FVolumeTextureToolkit::NormalizeArray(VolumeInfo, LoadedArray);
	return LoadedArray;
}

TMap<FString, FVolumeInfo> UVolumeLoader::ParseVolumeInfoFromHeader(const FString& FileName)
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

	UE_LOG(LogVolumeLoader, Log, TEXT("Loading volumes, nmeshes: %d"), NMeshes);
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
				UE_LOG(LogVolumeLoader, Log, TEXT("Found datafile %s"), *VolumeInfo.DataFileName);
			}
		}

		VolumeInfo.WorldDimensions = VolumeInfo.Spacing * FVector(VolumeInfo.Dimensions);
		VolumeInfos.Add(MeshId, VolumeInfo);
	}

	return VolumeInfos;
}

UVolumeAsset* UVolumeLoader::CreateVolumeFromFile(FVolumeInfo& VolumeInfo, const FString& FileName, UObject* Package,
                                                  const FString& MeshName, TArray<UVolumeTexture*> OutVolumeTextures)
{
	// Get valid package name and filepath.
	FString Directory, VolumeName, PackagePath;
	SplitPath(Package->GetFullName(), PackagePath, VolumeName);
	SplitPath(FileName, Directory, VolumeName);

	// Create persistent volume asset.
	UVolumeAsset* VolumeAsset = NewObject<UVolumeAsset>(Package, UVolumeAsset::StaticClass(),
	                                                    FName("VA_" + VolumeName + "_" + MeshName),
	                                                    RF_Standalone | RF_Public);

	uint8* LoadedArray = LoadAndConvertData(FPaths::Combine(Directory, VolumeInfo.DataFileName), VolumeInfo);

	// Create the persistent volume textures.
	for (int t = 0; t < VolumeInfo.Dimensions.W; ++t)
	{
		const long SingleTextureSize = static_cast<long>(VolumeInfo.Dimensions.X) * VolumeInfo.Dimensions.Y * VolumeInfo
			.Dimensions.Z;
		const FString VolumeTextureName = "VT_" + VolumeName + "_Data_t" + FString::FromInt(t);
		VolumeInfo.VolumeTextureDir = FPaths::Combine(PackagePath.RightChop(8), MeshName);
		UPackage* SubPackage = CreatePackage(*FPaths::Combine(VolumeInfo.VolumeTextureDir, VolumeTextureName));

		// Set pointer to current Volume position at timestep t
		UVolumeTexture* VolumeTexture;
		FVolumeTextureToolkit::CreateVolumeTextureAssets(VolumeTexture, VolumeTextureName, VolumeInfo,
		                                                 SubPackage, LoadedArray + SingleTextureSize * t);


		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Cast<UPackage>(SubPackage), VolumeTexture, RF_Standalone | RF_Public, *PackageFileName);

		OutVolumeTextures.Add(VolumeTexture);
	}
	VolumeAsset->VolumeInfo = VolumeInfo;

	delete[] LoadedArray;

	return VolumeAsset;
}
