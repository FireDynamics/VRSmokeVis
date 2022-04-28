﻿#include "Util/AssetCreationUtilities.h"
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


UObject* FAssetCreationUtils::CreateSimulation(const FString& InFileName, const FString& OutDirectory)
{
	constexpr bool LazyLoad = true;
	FString SimulationIntermediateFile;
	if (InFileName.Contains(".smv"))
	{
		FString SmokeViewDir, Temp;
		FImportUtils::SplitPath(InFileName, SmokeViewDir, Temp);
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
			SimulationIntermediateFile = PythonPreprocessor->RunFdsreader(InFileName, OutputDir);
			if (SimulationIntermediateFile.IsEmpty()) return nullptr;
		}
	} else
	{
		SimulationIntermediateFile = InFileName;
	}

	// Todo: Check if simulation exists already
	// Get valid package name and filepath
	FString Directory, SimName, OriginalDataDirectory, Temp;
	FImportUtils::SplitPath(SimulationIntermediateFile, Directory, SimName);
	
	UPackage* SimPackage = CreatePackage(*FPaths::Combine(OutDirectory, SimName));
	USimulationAsset* SimAsset = NewObject<USimulationAsset>(SimPackage, USimulationAsset::StaticClass(),
	                                                         FName("SIM_" + SimName), RF_Standalone | RF_Public);

	SimAsset->SimInfo = NewObject<USimulationInfo>();
	FImportUtils::ParseSimulationInfoFromFile(SimulationIntermediateFile, SimAsset->SimInfo);

	if (SimAsset->SimInfo->ObstPaths.Num() != 0)
	{
		const FString ObstsPackagePath = FPaths::Combine(OutDirectory, TEXT("Obsts"));
		// Chop these 6 characters: "/Game/"
		const FString ObstsPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), ObstsPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(ObstsPackageAbsolutePath);
		SimAsset->AssetDirectories.Add("Obst", ObstsPackagePath);
		FImportUtils::SplitPath(SimAsset->SimInfo->ObstPaths[0], OriginalDataDirectory, Temp);
		SimAsset->SimInfo->OriginalDataFilesPath.Add("Obst", OriginalDataDirectory);
		for (FString& ObstPath : SimAsset->SimInfo->ObstPaths)
		{
			FString ObstFullPath = FPaths::Combine(Directory, ObstPath);
			CreateObstruction(ObstsPackagePath, ObstFullPath, LazyLoad);
		}
	}

	if (SimAsset->SimInfo->SlicePaths.Num() != 0)
	{
		const FString SlicesPackagePath = FPaths::Combine(OutDirectory, TEXT("Slices"));
		const FString SlicesPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), SlicesPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(SlicesPackageAbsolutePath);
		SimAsset->AssetDirectories.Add("Slice", SlicesPackagePath);
		FImportUtils::SplitPath(SimAsset->SimInfo->SlicePaths[0], OriginalDataDirectory, Temp);
		SimAsset->SimInfo->OriginalDataFilesPath.Add("Slice", OriginalDataDirectory);
		for (FString& SlicePath : SimAsset->SimInfo->SlicePaths)
		{
			FString SliceFullPath = FPaths::Combine(Directory, SlicePath);
			CreateSlice(SlicesPackagePath, SliceFullPath, LazyLoad);
		}
	}

	if (SimAsset->SimInfo->VolumePaths.Num() != 0)
	{
		const FString VolumesPackagePath = FPaths::Combine(OutDirectory, TEXT("Volumes"));
		const FString VolumesPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), VolumesPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(VolumesPackagePath);
		SimAsset->AssetDirectories.Add("Volume", VolumesPackagePath);
		FImportUtils::SplitPath(SimAsset->SimInfo->VolumePaths[0], OriginalDataDirectory, Temp);
		SimAsset->SimInfo->OriginalDataFilesPath.Add("Volume", OriginalDataDirectory);
		for (FString& VolumePath : SimAsset->SimInfo->VolumePaths)
		{
			FString VolumeFullPath = FPaths::Combine(Directory, VolumePath);
			CreateVolume(VolumesPackagePath, VolumeFullPath, LazyLoad);
		}
	}

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	// AdditionalImportedObjects.Add(SimAsset);
	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		SimAsset->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::Save(SimAsset->GetPackage(), SimAsset, *PackageFileName, SavePackageArgs);

	return SimAsset;
}

void FAssetCreationUtils::CreateObstruction(const FString& RootPackage, const FString& FileName, const bool LazyLoad)
{
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	FString Directory, ObstName;
	FImportUtils::SplitPath(FileName, Directory, ObstName);
	
	FImportUtils::VerifyOrCreateDirectory(FPaths::ConvertRelativePathToFull(FPaths::Combine(RootPackage, "DataInfos")));
	
	UPackage* ObstDataInfoPackage = CreatePackage(*FPaths::Combine(RootPackage, "DataInfos", ObstName));
	TArray<float> BoundingBox = TArray<float>();
	UBoundaryDataInfo* DataInfo = NewObject<UBoundaryDataInfo>(ObstDataInfoPackage, UBoundaryDataInfo::StaticClass(), FName("DI_" + ObstName),
												  RF_Standalone | RF_Public);
	DataInfo->FdsName = ObstName;
	FImportUtils::ParseObstDataInfoFromFile(FileName, DataInfo, BoundingBox);
	TMap<FString, TMap<int, TArray<UTexture2D*>>> ObstructionTextures;

	UPackage* ObstPackage = CreatePackage(*FPaths::Combine(RootPackage, DataInfo->FdsName));
	UObstAsset* Obst = CreateObstructionFromFile(DataInfo, FileName, ObstPackage, LazyLoad);
	Obst->BoundingBox = BoundingBox;

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// Save DataInfo to disk
	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		DataInfo->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::Save(DataInfo->GetPackage(), DataInfo, *PackageFileName, SavePackageArgs);
	AssetRegistryModule.Get().AssetCreated(DataInfo);
	Obst->DataInfo = DataInfo;

	// Save Obstruction to disk
	PackageFileName = FPackageName::LongPackageNameToFilename(
		Obst->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::Save(Obst->GetPackage(), Obst, *PackageFileName, SavePackageArgs);
	AssetRegistryModule.Get().AssetCreated(Obst);
}

void FAssetCreationUtils::CreateSlice(const FString& RootPackage, const FString& FileName, const bool LazyLoad)
{
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	
	FImportUtils::VerifyOrCreateDirectory(FPaths::ConvertRelativePathToFull(FPaths::Combine(RootPackage, "DataInfos")));
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	TMap<FString, USliceDataInfo*> DataInfos;
	FImportUtils::ParseSliceDataInfoFromFile(FileName, DataInfos);
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UTexture2D*> SliceTextures;
		UPackage* SlicePackage = CreatePackage(*FPaths::Combine(RootPackage, It.Value()->FdsName));
		USliceAsset* Slice = CreateSliceFromFile(It.Value(), FileName, SlicePackage, It.Key(), LazyLoad);
		Slice->SliceTextures.Reserve(It.Value()->Dimensions.W);

		// Copy DataInfo into correct package
		UPackage* SliceDataInfoPackage = CreatePackage(*FPaths::Combine(RootPackage, "DataInfos", It.Value()->FdsName));
		USliceDataInfo* DataInfo = NewObject<USliceDataInfo>(SliceDataInfoPackage, USliceDataInfo::StaticClass(), FName("DI_" + It.Value()->FdsName),
													  RF_Standalone | RF_Public, It.Value());
		// Save DataInfo to disk
		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SliceDataInfoPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(SliceDataInfoPackage, DataInfo, *PackageFileName, SavePackageArgs);
		AssetRegistryModule.Get().AssetCreated(DataInfo);
		Slice->DataInfo = DataInfo;
		
		// Save Slice to disk
		PackageFileName = FPackageName::LongPackageNameToFilename(
			Slice->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Slice->GetPackage(), Slice, *PackageFileName, SavePackageArgs);		
		AssetRegistryModule.Get().AssetCreated(Slice);
	}
}

void FAssetCreationUtils::CreateVolume(const FString& RootPackage, const FString& FileName, const bool LazyLoad)
{
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;

	FImportUtils::VerifyOrCreateDirectory(FPaths::ConvertRelativePathToFull(FPaths::Combine(RootPackage, "DataInfos")));
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	TMap<FString, UVolumeDataInfo*> DataInfos;
	FImportUtils::ParseVolumeDataInfoFromFile(FileName, DataInfos);
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UVolumeTexture*> VolumeTextures;
		UPackage* VolumePackage = CreatePackage(*FPaths::Combine(RootPackage, It.Value()->FdsName));
		UVolumeAsset* Volume = CreateVolumeFromFile(It.Value(), FileName, VolumePackage, It.Key(), LazyLoad);

		Volume->VolumeTextures.Reserve(It.Value()->Dimensions.W);
		
		// Copy DataInfo into correct package
		UPackage* VolumeDataInfoPackage = CreatePackage(*FPaths::Combine(RootPackage, "DataInfos", It.Value()->FdsName));
		UVolumeDataInfo* DataInfo = NewObject<UVolumeDataInfo>(VolumeDataInfoPackage, UVolumeDataInfo::StaticClass(), FName("DI_" + It.Value()->FdsName),
													  RF_Standalone | RF_Public, It.Value());
		Volume->DataInfo = DataInfo;
		// Save DataInfo to disk
		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			VolumeDataInfoPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(VolumeDataInfoPackage, DataInfo, *PackageFileName, SavePackageArgs);
		AssetRegistryModule.Get().AssetCreated(DataInfo);
		
		// Save Volume to disk
		PackageFileName = FPackageName::LongPackageNameToFilename(Volume->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Volume->GetPackage(), Volume, *PackageFileName, SavePackageArgs);
		
		AssetRegistryModule.Get().AssetCreated(Volume);
	}
}

UObstAsset* FAssetCreationUtils::CreateObstructionFromFile(UBoundaryDataInfo* DataInfo, const FString& FileName,
                                                         UObject* Package, const bool LazyLoad)
{
	// Get valid package name and filepath.
	FString Directory, Temp, PackagePath;
	FImportUtils::SplitPath(Package->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(FileName, Directory, Temp);

	// Create persistent obst asset
	UObstAsset* ObstAsset = NewObject<UObstAsset>(Package, UObstAsset::StaticClass(), FName("OA_" + DataInfo->FdsName),
	                                              RF_Standalone | RF_Public);

	// Setup Texture Dirs
	TArray<int> Orientations;
	DataInfo->Dimensions.GetKeys(Orientations);
	// Iterate over all quantities
	for (const auto DataFileName : DataInfo->DataFileNames)
	{
		FString Quantity = DataFileName.Key;
		DataInfo->TextureDirs.Add(Quantity, FQuantityDir());
		for (const int Ori : Orientations)
		{
			const FString DirName = DataInfo->FdsName + "_" + Quantity + "_Face" + FString::FromInt(Ori);
			DataInfo->TextureDirs[Quantity].FaceDirs.Add(Ori, FPaths::Combine(PackagePath.RightChop(8), DirName));
		}
	}

	if (!LazyLoad) LoadObstTextures(DataInfo, Directory);

	return ObstAsset;
}

USliceAsset* FAssetCreationUtils::CreateSliceFromFile(USliceDataInfo* DataInfo, const FString& FileName,
													UObject* Package, const FString& MeshName,
													const bool LazyLoad)
{
	// Get valid package name and filepath
	FString Directory, Temp, PackagePath;
	FImportUtils::SplitPath(Package->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(FileName, Directory, Temp);

	// Create persistent slice asset
	USliceAsset* SliceAsset = NewObject<USliceAsset>(Package, USliceAsset::StaticClass(),
													 FName("SA_" + DataInfo->FdsName + "_" + MeshName),
													 RF_Standalone | RF_Public);

	// Setup Texture Dirs
	DataInfo->TextureDir = FPaths::Combine(PackagePath.RightChop(8), DataInfo->FdsName + "_" + MeshName);
	if (!LazyLoad) LoadSliceTextures(DataInfo, Directory);

	return SliceAsset;
}

UVolumeAsset* FAssetCreationUtils::CreateVolumeFromFile(UVolumeDataInfo* DataInfo, const FString& FileName,
													  UObject* Package, const FString& MeshName,
													  const bool LazyLoad)
{
	// Get valid package name and filepath
	FString Directory, Temp, PackagePath;
	FImportUtils::SplitPath(Package->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(FileName, Directory, Temp);

	// Create persistent volume asset
	UVolumeAsset* VolumeAsset = NewObject<UVolumeAsset>(Package, UVolumeAsset::StaticClass(),
														FName("VA_" + DataInfo->FdsName + "_" + MeshName),
														RF_Standalone | RF_Public);

	// Setup Texture Dirs
	DataInfo->TextureDir = FPaths::Combine(PackagePath.RightChop(8), MeshName);
	if (!LazyLoad) LoadVolumeTextures(DataInfo, Directory);

	return VolumeAsset;
}

void FAssetCreationUtils::LoadTextures(UDataInfo* DataInfo, const FString& Type, const FString& Directory)
{
	if (Type == "Obst")
	{
		LoadObstTextures(Cast<UBoundaryDataInfo>(DataInfo), Directory);
	} else if (Type == "Slice")
	{
		LoadSliceTextures(Cast<USliceDataInfo>(DataInfo), Directory);	
	} else
	{
		LoadVolumeTextures(Cast<UVolumeDataInfo>(DataInfo), Directory);
	}
}

void FAssetCreationUtils::LoadObstTextures(UBoundaryDataInfo* DataInfo, const FString& Directory)
{
	TArray<int> Orientations;
	DataInfo->Dimensions.GetKeys(Orientations);

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	// Iterate over all quantities
	for (const auto DataFileName : DataInfo->DataFileNames)
	{
		FString Quantity = DataFileName.Key;
		uint8* LoadedArray = FImportUtils::LoadObstData(FPaths::Combine(Directory, DataFileName.Value), DataInfo);
		int Offset = 0;

		// Set pointer to current data position at timestep t for each orientation
		for (const int Ori : Orientations)
		{
			const FString DirName = DataInfo->FdsName + "_" + Quantity + "_Face" + FString::FromInt(Ori);
			for (int t = 0; t < DataInfo->Dimensions[Ori].W; ++t)
			{
				const long SingleTextureSize = static_cast<long>(DataInfo->Dimensions[Ori].X) * DataInfo->Dimensions[Ori].
					Y;
				const FString FaceTextureName = "OT_" + DirName + "_Data_t" + FString::FromInt(t);
				UPackage* SubPackage = CreatePackage(
					*FPaths::Combine(DataInfo->TextureDirs[Quantity].FaceDirs[Ori], FaceTextureName));
				UTexture2D* ObstTexture = FTextureUtils::CreateTextureAsset(FaceTextureName, DataInfo->Dimensions[Ori],
																			SubPackage, LoadedArray + Offset,
																			SingleTextureSize);
				ObstTexture->Filter = TF_Default;
				FString PackageFileName = FPackageName::LongPackageNameToFilename(
					SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
				UPackage::Save(SubPackage, ObstTexture, *PackageFileName, SavePackageArgs);

				Offset += SingleTextureSize;
			}
		}
		FMemory::Free(LoadedArray);
	}
}

void FAssetCreationUtils::LoadSliceTextures(USliceDataInfo* DataInfo, const FString& Directory)
{
	uint8* LoadedArray = FImportUtils::LoadSliceData(FPaths::Combine(Directory, DataInfo->DataFileName), DataInfo);

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	// Create the persistent slice textures.
	for (int t = 0; t < DataInfo->Dimensions.W; ++t)
	{
		const long SingleTextureSize = static_cast<long>(DataInfo->Dimensions.X) * DataInfo->Dimensions.Y * DataInfo
			->Dimensions.Z;
		const FString SliceTextureName = "ST_" + DataInfo->FdsName + "_Data_t" + FString::FromInt(t);
		UPackage* SubPackage = CreatePackage(*FPaths::Combine(DataInfo->TextureDir, SliceTextureName));

		// Set pointer to current slice position at timestep t
		UTexture2D* SliceTexture = FTextureUtils::CreateSliceTextureAsset(
			SliceTextureName, DataInfo->Dimensions, SubPackage,
			LoadedArray + SingleTextureSize * t, SingleTextureSize);

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(SubPackage, SliceTexture, *PackageFileName, SavePackageArgs);
	}

	delete[] LoadedArray;
}

void FAssetCreationUtils::LoadVolumeTextures(UVolumeDataInfo* DataInfo, const FString& Directory)
{
	uint8* LoadedArray = FImportUtils::LoadAndConvertVolumeData(1, FPaths::Combine(Directory, DataInfo->DataFileName),
																DataInfo);

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	const long SingleTextureSize = static_cast<long>(DataInfo->Dimensions.X) * DataInfo->Dimensions.Y * DataInfo->
		Dimensions.Z;
	// Create the persistent volume textures
	for (int t = 0; t < DataInfo->Dimensions.W; ++t)
	{
		const FString VolumeTextureName = "VT_" + DataInfo->FdsName + "_Data_t" + FString::FromInt(t);
		UPackage* SubPackage = CreatePackage(*FPaths::Combine(DataInfo->TextureDir, VolumeTextureName));

		// Set pointer to current Volume position at timestep t
		UVolumeTexture* VolumeTexture = FTextureUtils::CreateVolumeAsset(
			VolumeTextureName, DataInfo->Dimensions, SubPackage,
			LoadedArray + SingleTextureSize * t,
			SingleTextureSize);
		VolumeTexture->Filter = TF_Bilinear;

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Cast<UPackage>(SubPackage), VolumeTexture, *PackageFileName, SavePackageArgs);
	}

	delete[] LoadedArray;
}
