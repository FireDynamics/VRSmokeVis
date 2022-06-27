#include "Util/AssetCreationUtilities.h"
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
			UPreprocessor* PythonPreprocessor = NewObject<UPreprocessor>();
			SimulationIntermediateFile = PythonPreprocessor->RunFdsreader(InFileName, OutputDir);
			if (SimulationIntermediateFile.IsEmpty()) return nullptr;
		}
	}
	else
	{
		SimulationIntermediateFile = InFileName;
	}

	// Get valid package name and filepath
	FString Directory, SimName, Temp;
	FImportUtils::SplitPath(SimulationIntermediateFile, Directory, SimName);
	SimName.LeftChopInline(4); // Chop "-smv"

	FString NewHash = FImportUtils::GetSimulationHashFromFile(SimulationIntermediateFile);

	TArray<FAssetData> SimulationInfos;
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(USimulationInfo::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(OutDirectory);
	ObjectLibrary->GetAssetDataList(SimulationInfos);
	if (SimulationInfos.Num() > 0)
	{
		const FString OldHash = Cast<USimulationInfo>(
			FStreamableManager().LoadSynchronous(SimulationInfos[0].ToSoftObjectPath()))->Hash;

		// If the old hash equals the new hash, we don't have to continue here and simply use the already loaded simulation
		if (OldHash.Equals(NewHash))
		{
			TArray<FAssetData> SimulationAssets;
			ObjectLibrary = UObjectLibrary::CreateLibrary(USimulationAsset::StaticClass(), false, GIsEditor);
			ObjectLibrary->AddToRoot();
			ObjectLibrary->LoadAssetDataFromPath(OutDirectory);
			ObjectLibrary->GetAssetDataList(SimulationAssets);
			if (SimulationInfos.Num() > 0)
				return FStreamableManager().LoadSynchronous(SimulationAssets[0].ToSoftObjectPath());
		}
	}

	UPackage* SimulationInfoPackage = CreatePackage(*FPaths::Combine(OutDirectory, "SI_" + SimName));
	USimulationInfo* SimInfo = NewObject<USimulationInfo>(SimulationInfoPackage, USimulationInfo::StaticClass(),
	                                                      FName("SI_" + SimName), RF_Standalone | RF_Public);
	FImportUtils::ParseSimulationInfoFromFile(SimulationIntermediateFile, SimInfo);

	UPackage* SimPackage = CreatePackage(*FPaths::Combine(OutDirectory, SimName));
	USimulationAsset* SimAsset = NewObject<USimulationAsset>(SimPackage, USimulationAsset::StaticClass(),
	                                                         FName(SimName), RF_Standalone | RF_Public);

	if (SimInfo->ObstPaths.Num() != 0)
	{
		const FString ObstsPackagePath = FPaths::Combine(OutDirectory, TEXT("Obsts"));
		// Chop these 6 characters: "/Game/"
		const FString ObstsPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), ObstsPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(ObstsPackageAbsolutePath);
		SimAsset->AssetDirectories.Add("Obst", ObstsPackagePath);
		for (FString& ObstPath : SimInfo->ObstPaths)
		{
			FString ObstFullPath = FPaths::Combine(Directory, ObstPath);
			LoadAndCreateObstruction(ObstsPackagePath, ObstFullPath, LazyLoad);
		}
	}

	if (SimInfo->SlicePaths.Num() != 0)
	{
		const FString SlicesPackagePath = FPaths::Combine(OutDirectory, TEXT("Slices"));
		const FString SlicesPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), SlicesPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(SlicesPackageAbsolutePath);
		SimAsset->AssetDirectories.Add("Slice", SlicesPackagePath);
		for (FString& SlicePath : SimInfo->SlicePaths)
		{
			FString SliceFullPath = FPaths::Combine(Directory, SlicePath);
			LoadSlice(SlicesPackagePath, SliceFullPath, LazyLoad);
		}
	}

	if (SimInfo->VolumePaths.Num() != 0)
	{
		const FString VolumesPackagePath = FPaths::Combine(OutDirectory, TEXT("Volumes"));
		const FString VolumesPackageAbsolutePath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), VolumesPackagePath.RightChop(6)));
		FImportUtils::VerifyOrCreateDirectory(VolumesPackagePath);
		SimAsset->AssetDirectories.Add("Volume", VolumesPackagePath);
		for (FString& VolumePath : SimInfo->VolumePaths)
		{
			FString VolumeFullPath = FPaths::Combine(Directory, VolumePath);
			LoadVolumes(VolumesPackagePath, VolumeFullPath, LazyLoad);
		}
	}

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;

	// Save DataInfo to disk
	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		SimInfo->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::Save(SimInfo->GetPackage(), SimInfo, *PackageFileName, SavePackageArgs);
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get().AssetCreated(SimInfo);
	SimAsset->SimInfo = SimInfo;

	PackageFileName = FPackageName::LongPackageNameToFilename(
		SimAsset->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::Save(SimAsset->GetPackage(), SimAsset, *PackageFileName, SavePackageArgs);

	return SimAsset;
}

void FAssetCreationUtils::LoadAndCreateObstruction(const FString& RootPackage, const FString& FileName,
                                                   const bool LazyLoad)
{
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	FString Directory, ObstName, Temp, PackagePath;
	FImportUtils::SplitPath(FileName, Directory, ObstName);

	FImportUtils::VerifyOrCreateDirectory(FPaths::ConvertRelativePathToFull(FPaths::Combine(RootPackage, "DataInfos")));

	UPackage* ObstDataInfoPackage = CreatePackage(*FPaths::Combine(RootPackage, "DataInfos", ObstName));
	TArray<float> BoundingBox = TArray<float>();
	UBoundaryDataInfo* DataInfo = NewObject<UBoundaryDataInfo>(ObstDataInfoPackage, UBoundaryDataInfo::StaticClass(),
	                                                           FName("DI_" + ObstName),
	                                                           RF_Standalone | RF_Public);
	DataInfo->FdsName = ObstName;
	FImportUtils::ParseObstDataInfoFromFile(FileName, DataInfo, BoundingBox);
	TMap<FString, TMap<int, TArray<UTexture2D*>>> ObstructionTextures;

	UPackage* ObstPackage = CreatePackage(*FPaths::Combine(RootPackage, DataInfo->FdsName));
	// Get valid package name and filepath.
	FImportUtils::SplitPath(ObstPackage->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(FileName, Directory, Temp);

	// Create persistent obst asset
	UObstAsset* ObstAsset = NewObject<UObstAsset>(ObstPackage, UObstAsset::StaticClass(),
	                                              FName("OA_" + DataInfo->FdsName),
	                                              RF_Standalone | RF_Public);
	ObstAsset->BoundingBox = BoundingBox;
	// Setup Texture Dirs
	TArray<int> Orientations;
	DataInfo->Dimensions.GetKeys(Orientations);
	// Iterate over all quantities and fill in the paths to the textures in the DataInfo that are either generated now
	// when LazyLoad is set to false or generated at a later point in time
	TArray<FString> Quantities;
	DataInfo->DataFileNames.GetKeys(Quantities);
	for (const FString& Quantity : Quantities)
	{
		DataInfo->DataFileNames[Quantity] = FPaths::Combine(Directory, DataInfo->DataFileNames[Quantity]);
		DataInfo->TextureDirs.Add(Quantity, FQuantityDir());
		for (const int Ori : Orientations)
		{
			const FString DirName = DataInfo->FdsName + "_" + Quantity + "_Face" + FString::FromInt(Ori);
			DataInfo->TextureDirs[Quantity].FaceDirs.Add(Ori, FPaths::Combine(PackagePath.RightChop(8), DirName));
		}
	}

	if (!LazyLoad) LoadObstTextures(DataInfo);

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		"AssetRegistry");

	// Save DataInfo to disk
	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		DataInfo->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::Save(DataInfo->GetPackage(), DataInfo, *PackageFileName, SavePackageArgs);
	AssetRegistryModule.Get().AssetCreated(DataInfo);
	ObstAsset->DataInfo = DataInfo;

	// Save Obstruction to disk
	PackageFileName = FPackageName::LongPackageNameToFilename(
		ObstAsset->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::Save(ObstAsset->GetPackage(), ObstAsset, *PackageFileName, SavePackageArgs);
	AssetRegistryModule.Get().AssetCreated(ObstAsset);
}

void FAssetCreationUtils::LoadSlice(const FString& RootPackage, const FString& FileName, const bool LazyLoad)
{
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;

	FImportUtils::VerifyOrCreateDirectory(FPaths::ConvertRelativePathToFull(FPaths::Combine(RootPackage, "DataInfos")));
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		"AssetRegistry");

	FString Directory, Temp;
	FImportUtils::SplitPath(FileName, Directory, Temp);

	TMap<FString, USliceDataInfo*> DataInfos;
	FImportUtils::ParseSliceDataInfoFromFile(FileName, DataInfos);
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UTexture2D*> SliceTextures;
		UPackage* SlicePackage = CreatePackage(*FPaths::Combine(RootPackage, It.Value()->FdsName));
		USliceAsset* Slice = CreateSlice(It.Value(), FileName, SlicePackage, It.Key(), LazyLoad);
		Slice->SliceTextures.Reserve(It.Value()->Dimensions.W);

		// Copy DataInfo into correct package
		UPackage* SliceDataInfoPackage = CreatePackage(*FPaths::Combine(RootPackage, "DataInfos", It.Value()->FdsName));
		USliceDataInfo* DataInfo = NewObject<USliceDataInfo>(SliceDataInfoPackage, USliceDataInfo::StaticClass(),
		                                                     FName("DI_" + It.Value()->FdsName),
		                                                     RF_Standalone | RF_Public, It.Value());
		DataInfo->DataFileName = FPaths::Combine(Directory, DataInfo->DataFileName);

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

void FAssetCreationUtils::LoadVolumes(const FString& RootPackage, const FString& FileName, const bool LazyLoad)
{
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;

	FImportUtils::VerifyOrCreateDirectory(FPaths::ConvertRelativePathToFull(FPaths::Combine(RootPackage, "DataInfos")));
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		"AssetRegistry");

	FString Directory, Temp;
	FImportUtils::SplitPath(FileName, Directory, Temp);

	TMap<FString, UVolumeDataInfo*> DataInfos;
	FImportUtils::ParseVolumeDataInfoFromFile(FileName, DataInfos);
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UVolumeTexture*> VolumeTextures;
		UPackage* VolumePackage = CreatePackage(*FPaths::Combine(RootPackage, It.Value()->FdsName));
		UVolumeAsset* Volume = CreateVolume(It.Value(), FileName, VolumePackage, It.Key(), LazyLoad);

		Volume->VolumeTextures.Reserve(It.Value()->Dimensions.W);

		// Copy DataInfo into correct package
		UPackage* VolumeDataInfoPackage =
			CreatePackage(*FPaths::Combine(RootPackage, "DataInfos", It.Value()->FdsName));
		UVolumeDataInfo* DataInfo = NewObject<UVolumeDataInfo>(VolumeDataInfoPackage, UVolumeDataInfo::StaticClass(),
		                                                       FName("DI_" + It.Value()->FdsName),
		                                                       RF_Standalone | RF_Public, It.Value());
		DataInfo->DataFileName = FPaths::Combine(Directory, DataInfo->DataFileName);

		Volume->DataInfo = DataInfo;
		// Save DataInfo to disk
		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			VolumeDataInfoPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(VolumeDataInfoPackage, DataInfo, *PackageFileName, SavePackageArgs);
		AssetRegistryModule.Get().AssetCreated(DataInfo);

		// Save Volume to disk
		PackageFileName = FPackageName::LongPackageNameToFilename(Volume->GetPackage()->GetName(),
		                                                          FPackageName::GetAssetPackageExtension());
		UPackage::Save(Volume->GetPackage(), Volume, *PackageFileName, SavePackageArgs);

		AssetRegistryModule.Get().AssetCreated(Volume);
	}
}

USliceAsset* FAssetCreationUtils::CreateSlice(USliceDataInfo* DataInfo, const FString& FileName,
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
	if (!LazyLoad) LoadSliceTextures(DataInfo);

	return SliceAsset;
}

UVolumeAsset* FAssetCreationUtils::CreateVolume(UVolumeDataInfo* DataInfo, const FString& FileName,
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
	if (!LazyLoad) LoadVolumeTextures(DataInfo);

	return VolumeAsset;
}

void FAssetCreationUtils::LoadTextures(UDataInfo* DataInfo, const FString& Type)
{
	if (Type == "Obst")
	{
		LoadObstTextures(Cast<UBoundaryDataInfo>(DataInfo));
	}
	else if (Type == "Slice")
	{
		LoadSliceTextures(Cast<USliceDataInfo>(DataInfo));
	}
	else
	{
		LoadVolumeTextures(Cast<UVolumeDataInfo>(DataInfo));
	}
}

void FAssetCreationUtils::LoadObstTextures(UBoundaryDataInfo* DataInfo)
{
	TArray<int> Orientations;
	DataInfo->Dimensions.GetKeys(Orientations);

	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone | RF_Public;
	// Iterate over all quantities
	for (const auto DataFileName : DataInfo->DataFileNames)
	{
		FString Quantity = DataFileName.Key;
		uint8* LoadedArray = FImportUtils::LoadObstData(DataFileName.Value, DataInfo);
		int Offset = 0;

		// Set pointer to current data position at timestep t for each orientation
		for (const int Ori : Orientations)
		{
			const FString DirName = DataInfo->FdsName + "_" + Quantity + "_Face" + FString::FromInt(Ori);
			for (int t = 0; t < DataInfo->Dimensions[Ori].W; ++t)
			{
				const long SingleTextureSize = static_cast<long>(DataInfo->Dimensions[Ori].X) * DataInfo->Dimensions[
						Ori].
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

void FAssetCreationUtils::LoadSliceTextures(USliceDataInfo* DataInfo)
{
	uint8* LoadedArray = FImportUtils::LoadSliceData(DataInfo->DataFileName, DataInfo);

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

void FAssetCreationUtils::LoadVolumeTextures(UVolumeDataInfo* DataInfo)
{
	uint8* LoadedArray = FImportUtils::LoadAndConvertVolumeData(1, DataInfo->DataFileName, DataInfo);

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
