#include "Util/AssetUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Util/TextureUtilities.h"
#include "Assets/VolumeAsset.h"
#include "Assets/SliceAsset.h"
#include "Assets/ObstAsset.h"
#include "Assets/SimulationAsset.h"
#include "Containers/UnrealString.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/VolumeTexture.h"
#include "UObject/SavePackage.h"
#include "Util/ImportUtilities.h"
#include "Util/Preprocessor.h"


DEFINE_LOG_CATEGORY(LogAssetUtils)


UObject* FAssetUtils::CreateSimulation(UObject* InParent, const FString& FileName)
{
	constexpr bool LazyLoad = true;
	FString SimulationIntermediateFile;
	if (FileName.Contains(".smv"))
	{
		FString SmokeViewDir, Temp;
		FImportUtils::SplitPath(FileName, SmokeViewDir, Temp);
		const FString OutputDir = FPaths::Combine(SmokeViewDir, TEXT("SmokeVisIntermediate"));

		// First check if there already is intermediate data from a previous run
		if (FPaths::DirectoryExists(OutputDir))
		{
			TArray<FString> FoundFiles;
			IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
			FileManager.FindFiles(FoundFiles, *OutputDir, TEXT(".yaml"));
			for (const FString& File : FoundFiles)
			{
				if (File.Contains("-smv.yaml"))
				{
					SimulationIntermediateFile = File;
					break;
				}
			}
		}
		if (SimulationIntermediateFile.IsEmpty())
		{
			FImportUtils::VerifyOrCreateDirectory(OutputDir);
			UPreprocessor *PythonPreprocessor = NewObject<UPreprocessor>();
			SimulationIntermediateFile = PythonPreprocessor->RunFdsreader(FileName, OutputDir);
		}
	} else
	{
		SimulationIntermediateFile = FileName;
	}

	// Get valid package name and filepath
	FString Directory, SimName, PackagePath, OriginalDataDirectory, Temp;
	FImportUtils::SplitPath(InParent->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(SimulationIntermediateFile, Directory, SimName);

	USimulationAsset* SimAsset = NewObject<USimulationAsset>(InParent, USimulationAsset::StaticClass(),
	                                                         FName("SIM_" + SimName), RF_Standalone | RF_Public);

	SimAsset->SimInfo = FImportUtils::ParseSimulationInfoFromFile(SimulationIntermediateFile);

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;

	// Chop these 8 characters: "Package "
	PackagePath.RightChopInline(8);
	if (SimAsset->SimInfo.ObstPaths.Num() != 0)
	{
		const FString ObstsPackagePath = FPaths::Combine(PackagePath, TEXT("Obsts"));
		// Chop these 6 characters: "/Game/"
		const FString ObstsPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), ObstsPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(ObstsPackageAbsolutePath);
		SimAsset->ObstructionsDirectory = ObstsPackagePath;
		FImportUtils::SplitPath(SimAsset->SimInfo.ObstPaths[0], OriginalDataDirectory, Temp);
		SimAsset->SimInfo.OriginalObstFilesPath = OriginalDataDirectory;
		for (FString& ObstPath : SimAsset->SimInfo.ObstPaths)
		{
			FString ObstFullPath = FPaths::Combine(Directory, ObstPath);
			UObject* Obst = CreateObstruction(ObstsPackagePath, ObstFullPath, LazyLoad);
			FString PackageFileName = FPackageName::LongPackageNameToFilename(
				Obst->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
			UPackage::Save(Obst->GetPackage(), Obst, *PackageFileName, SavePackageArgs);
		}
	}

	if (SimAsset->SimInfo.SlicePaths.Num() != 0)
	{
		const FString SlicesPackagePath = FPaths::Combine(PackagePath, TEXT("Slices"));
		const FString SlicesPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), SlicesPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(SlicesPackageAbsolutePath);
		SimAsset->SlicesDirectory = SlicesPackagePath;
		FImportUtils::SplitPath(SimAsset->SimInfo.SlicePaths[0], OriginalDataDirectory, Temp);
		SimAsset->SimInfo.OriginalSliceFilesPath = OriginalDataDirectory;
		for (FString& SlicePath : SimAsset->SimInfo.SlicePaths)
		{
			FString SliceFullPath = FPaths::Combine(Directory, SlicePath);
			UObject* Slice = CreateSlice(SlicesPackagePath, SliceFullPath, LazyLoad);
			FString PackageFileName = FPackageName::LongPackageNameToFilename(
				Slice->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
			UPackage::Save(Slice->GetPackage(), Slice, *PackageFileName, SavePackageArgs);
		}
	}

	if (SimAsset->SimInfo.VolumePaths.Num() != 0)
	{
		const FString VolumesPackagePath = FPaths::Combine(PackagePath, TEXT("Volumes"));
		const FString VolumesPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), VolumesPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(VolumesPackagePath);
		SimAsset->VolumesDirectory = VolumesPackagePath;
		FImportUtils::SplitPath(SimAsset->SimInfo.VolumePaths[0], OriginalDataDirectory, Temp);
		SimAsset->SimInfo.OriginalVolumeFilesPath = OriginalDataDirectory;
		for (FString& VolumePath : SimAsset->SimInfo.VolumePaths)
		{
			FString VolumeFullPath = FPaths::Combine(Directory, VolumePath);
			UObject* Volume = CreateVolume(VolumesPackagePath, VolumeFullPath, LazyLoad);
			FString PackageFileName = FPackageName::LongPackageNameToFilename(
				Volume->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
			UPackage::Save(Volume->GetPackage(), Volume, *PackageFileName, SavePackageArgs);
		}
	}

	// AdditionalImportedObjects.Add(SimAsset);
	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		SimAsset->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::Save(SimAsset->GetPackage(), SimAsset, *PackageFileName, SavePackageArgs);

	return SimAsset;
}

UObject* FAssetUtils::CreateVolume(const FString& RootPackage, const FString& FileName, const bool LazyLoad)
{
	TMap<FString, FVolumeDataInfo> DataInfos = FImportUtils::ParseSliceVolumeDataInfoFromFile(FileName);
	UVolumeAsset* OutVolume = nullptr;
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UVolumeTexture*> VolumeTextures;
		UPackage* VolumePackage = CreatePackage(*FPaths::Combine(RootPackage, It.Value().FdsName));
		OutVolume = CreateVolumeFromFile(It.Value(), FileName, VolumePackage, It.Key(), LazyLoad);

		OutVolume->VolumeTextures.Reserve(It.Value().Dimensions.W);

		// AdditionalImportedObjects.Add(OutVolume);
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().AssetCreated(OutVolume);
		// FAssetRegistryModule::AssetCreated(OutVolume);
	}

	// Return last volume
	return OutVolume;
}

UObject* FAssetUtils::CreateSlice(const FString& RootPackage, const FString& FileName, const bool LazyLoad)
{
	TMap<FString, FVolumeDataInfo> DataInfos = FImportUtils::ParseSliceVolumeDataInfoFromFile(FileName);
	USliceAsset* OutSlice = nullptr;
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UTexture2D*> SliceTextures;
		UPackage* SlicePackage = CreatePackage(*FPaths::Combine(RootPackage, It.Value().FdsName));
		OutSlice = CreateSliceFromFile(It.Value(), FileName, SlicePackage, It.Key(), LazyLoad);

		OutSlice->SliceTextures.Reserve(It.Value().Dimensions.W);

		// AdditionalImportedObjects.Add(OutSlice);
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().AssetCreated(OutSlice);
		// FAssetRegistryModule::AssetCreated(OutSlice);
	}

	// Return last slice
	return OutSlice;
}

UObject* FAssetUtils::CreateObstruction(const FString& RootPackage, const FString& FileName, const bool LazyLoad)
{
	TArray<float> BoundingBox = TArray<float>();
	FBoundaryDataInfo DataInfo = FImportUtils::ParseObstDataInfoFromFile(FileName, BoundingBox);
	TMap<FString, TMap<int, TArray<UTexture2D*>>> ObstructionTextures;

	UPackage* ObstPackage = CreatePackage(*FPaths::Combine(RootPackage, DataInfo.ObstName));
	UObstAsset* OutObst = CreateObstructionFromFile(DataInfo, FileName, ObstPackage, LazyLoad);
	OutObst->BoundingBox = BoundingBox;

	// AdditionalImportedObjects.Add(OutObst);
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().AssetCreated(OutObst);
	// FAssetRegistryModule::AssetCreated(OutObst);

	// Return obstruction
	return OutObst;
}

UVolumeAsset* FAssetUtils::CreateVolumeFromFile(FVolumeDataInfo& DataInfo, const FString& FileName,
                                                      UObject* Package, const FString& MeshName,
                                                      const bool LazyLoad)
{
	// Get valid package name and filepath
	FString Directory, Temp, PackagePath;
	FImportUtils::SplitPath(Package->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(FileName, Directory, Temp);

	// Create persistent volume asset
	UVolumeAsset* VolumeAsset = NewObject<UVolumeAsset>(Package, UVolumeAsset::StaticClass(),
	                                                    FName("VA_" + DataInfo.FdsName + "_" + MeshName),
	                                                    RF_Standalone | RF_Public);

	// Setup Texture Dirs
	DataInfo.TextureDir = FPaths::Combine(PackagePath.RightChop(8), MeshName);
	if (!LazyLoad) LoadVolumeTextures(DataInfo, Directory);

	VolumeAsset->VolumeInfo = DataInfo;

	return VolumeAsset;
}

USliceAsset* FAssetUtils::CreateSliceFromFile(FVolumeDataInfo& DataInfo, const FString& FileName,
                                                    UObject* Package, const FString& MeshName,
                                                    const bool LazyLoad)
{
	// Get valid package name and filepath.
	FString Directory, Temp, PackagePath;
	FImportUtils::SplitPath(Package->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(FileName, Directory, Temp);

	// Create persistent slice asset.
	USliceAsset* SliceAsset = NewObject<USliceAsset>(Package, USliceAsset::StaticClass(),
	                                                 FName("SA_" + DataInfo.FdsName + "_" + MeshName),
	                                                 RF_Standalone | RF_Public);

	// Setup Texture Dirs
	DataInfo.TextureDir = FPaths::Combine(PackagePath.RightChop(8), DataInfo.FdsName + "_" + MeshName);
	if (!LazyLoad) LoadSliceTextures(DataInfo, Directory);

	SliceAsset->SliceInfo = DataInfo;

	return SliceAsset;
}

UObstAsset* FAssetUtils::CreateObstructionFromFile(FBoundaryDataInfo& DataInfo, const FString& FileName,
                                                         UObject* Package, const bool LazyLoad)
{
	// Get valid package name and filepath.
	FString Directory, Temp, PackagePath;
	FImportUtils::SplitPath(Package->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(FileName, Directory, Temp);

	// Create persistent obst asset
	UObstAsset* ObstAsset = NewObject<UObstAsset>(Package, UObstAsset::StaticClass(), FName("OA_" + DataInfo.ObstName),
	                                              RF_Standalone | RF_Public);

	// Setup Texture Dirs
	TArray<int> Orientations;
	DataInfo.Dimensions.GetKeys(Orientations);
	// Iterate over all quantities
	for (const auto DataFileName : DataInfo.DataFileNames)
	{
		FString Quantity = DataFileName.Key;
		DataInfo.TextureDirs.Add(Quantity, FQuantityDir());
		for (const int Ori : Orientations)
		{
			const FString DirName = DataInfo.ObstName + "_" + Quantity + "_Face" + FString::FromInt(Ori);
			DataInfo.TextureDirs[Quantity].FaceDirs.Add(Ori, FPaths::Combine(PackagePath.RightChop(8), DirName));
		}
	}

	if (!LazyLoad) LoadObstTextures(DataInfo, Directory);

	ObstAsset->ObstInfo = DataInfo;

	return ObstAsset;
}

void FAssetUtils::LoadVolumeTextures(FVolumeDataInfo& DataInfo, const FString& Directory)
{
	uint8* LoadedArray = FImportUtils::LoadAndConvertVolumeData(1, FPaths::Combine(Directory, DataInfo.DataFileName),
	                                                            DataInfo);

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	const long SingleTextureSize = static_cast<long>(DataInfo.Dimensions.X) * DataInfo.Dimensions.Y * DataInfo.
		Dimensions.Z;
	// Create the persistent volume textures
	for (int t = 0; t < DataInfo.Dimensions.W; ++t)
	{
		const FString VolumeTextureName = "VT_" + DataInfo.FdsName + "_Data_t" + FString::FromInt(t);
		UPackage* SubPackage = CreatePackage(*FPaths::Combine(DataInfo.TextureDir, VolumeTextureName));

		// Set pointer to current Volume position at timestep t
		UVolumeTexture* VolumeTexture = FTextureUtils::CreateVolumeAsset(
			VolumeTextureName, DataInfo.Dimensions, SubPackage,
			LoadedArray + SingleTextureSize * t,
			SingleTextureSize);
		VolumeTexture->Filter = TF_Bilinear;

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Cast<UPackage>(SubPackage), VolumeTexture, *PackageFileName, SavePackageArgs);

		// Add VolumeTextures to AdditionalImportedObjects so it also gets saved in-editor.
		// AdditionalImportedObjects.Add(VolumeTexture);
	}

	delete[] LoadedArray;
}

void FAssetUtils::LoadSliceTextures(FVolumeDataInfo& DataInfo, const FString& Directory)
{
	uint8* LoadedArray = FImportUtils::LoadSliceData(FPaths::Combine(Directory, DataInfo.DataFileName), DataInfo);

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	// Create the persistent slice textures.
	for (int t = 0; t < DataInfo.Dimensions.W; ++t)
	{
		const long SingleTextureSize = static_cast<long>(DataInfo.Dimensions.X) * DataInfo.Dimensions.Y * DataInfo
			.Dimensions.Z;
		const FString SliceTextureName = "ST_" + DataInfo.FdsName + "_Data_t" + FString::FromInt(t);
		UPackage* SubPackage = CreatePackage(*FPaths::Combine(DataInfo.TextureDir, SliceTextureName));

		// Set pointer to current slice position at timestep t
		UTexture2D* SliceTexture = FTextureUtils::CreateSliceTextureAsset(
			SliceTextureName, DataInfo.Dimensions, SubPackage,
			LoadedArray + SingleTextureSize * t, SingleTextureSize);

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(SubPackage, SliceTexture, *PackageFileName, SavePackageArgs);

		// Add SliceTexture to AdditionalImportedObjects so it also gets saved in-editor
		// AdditionalImportedObjects.Add(SliceTexture);
	}

	delete[] LoadedArray;
}

void FAssetUtils::LoadObstTextures(FBoundaryDataInfo& DataInfo, const FString& Directory)
{
	TArray<int> Orientations;
	DataInfo.Dimensions.GetKeys(Orientations);

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	// Iterate over all quantities
	for (const auto DataFileName : DataInfo.DataFileNames)
	{
		FString Quantity = DataFileName.Key;
		uint8* LoadedArray = FImportUtils::LoadObstData(FPaths::Combine(Directory, DataFileName.Value), DataInfo);
		int Offset = 0;

		// Set pointer to current data position at timestep t for each orientation
		for (const int Ori : Orientations)
		{
			const FString DirName = DataInfo.ObstName + "_" + Quantity + "_Face" + FString::FromInt(Ori);
			for (int t = 0; t < DataInfo.Dimensions[Ori].W; ++t)
			{
				const long SingleTextureSize = static_cast<long>(DataInfo.Dimensions[Ori].X) * DataInfo.Dimensions[Ori].
					Y;
				const FString FaceTextureName = "OT_" + DirName + "_Data_t" + FString::FromInt(t);
				UPackage* SubPackage = CreatePackage(
					*FPaths::Combine(DataInfo.TextureDirs[Quantity].FaceDirs[Ori], FaceTextureName));
				UTexture2D* ObstTexture = FTextureUtils::CreateTextureAsset(FaceTextureName, DataInfo.Dimensions[Ori],
				                                                            SubPackage, LoadedArray + Offset,
				                                                            SingleTextureSize);
				ObstTexture->Filter = TF_Default;
				FString PackageFileName = FPackageName::LongPackageNameToFilename(
					SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
				UPackage::Save(SubPackage, ObstTexture, *PackageFileName, SavePackageArgs);

				// AdditionalImportedObjects.Add(ObstTexture);

				Offset += SingleTextureSize;
			}
		}
		FMemory::Free(LoadedArray);
	}
}
