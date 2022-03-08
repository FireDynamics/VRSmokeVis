#include "Assets/VRSSAssetFactory.h"

#include "Util/TextureUtilities.h"
#include "Assets/VolumeAsset.h"
#include "Assets/SliceAsset.h"
#include "Assets/ObstAsset.h"
#include "Assets/SimulationAsset.h"
#include "Containers/UnrealString.h"
#include "Engine/VolumeTexture.h"
#include "Util/ImportUtilities.h"

DEFINE_LOG_CATEGORY(LogAssetFactory)


UVRSSAssetFactory::UVRSSAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("yaml;")) + NSLOCTEXT("UVRSSAssetFactory", "FormatYaml", ".yaml File").ToString());

	SupportedClass = USimulationAsset::StaticClass();
	// Todo: Is this enough, even though we are also using other classes?
	bCreateNew = false;
	bEditorImport = true;
	ImportPriority = 1;
}

UObject* UVRSSAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                              const FString& FileName, const TCHAR* Params, FFeedbackContext* Warn,
                                              bool& bOutOperationCanceled)
{
	UObject* Out = nullptr;
	if (InName.ToString().Contains(TEXT("smoke")) || FileName.Contains(TEXT("slice3D")))
	{
		Out = CreateVolume(InParent, FileName, false);
	}
	else if (InName.ToString().Contains(TEXT("slice2D")))
	{
		Out = CreateSlice(InParent, FileName, false);
	}
	else if (InName.ToString().Contains(TEXT("obst")))
	{
		Out = CreateObstruction(InParent, FileName, false);
	}
	else if (InName.ToString().Contains(TEXT("-smv")))
	{
		Out = CreateSimulation(InParent, FileName);
	}

	bOutOperationCanceled = false;

	return Out;
}

UObject* UVRSSAssetFactory::CreateSimulation(UObject* InParent, const FString& FileName)
{
	// Get valid package name and filepath
	FString Directory, SimName, PackagePath, OriginalDataDirectory, Temp;
	FImportUtils::SplitPath(InParent->GetFullName(), PackagePath, Temp);
	FImportUtils::SplitPath(FileName, Directory, SimName);

	USimulationAsset* SimAsset = NewObject<USimulationAsset>(InParent, USimulationAsset::StaticClass(),
	                                                         FName("SIM_" + SimName), RF_Standalone | RF_Public);

	SimAsset->SimInfo = FImportUtils::ParseSimulationInfoFromFile(FileName);

	// const FString ObstsPackagePath = FPaths::Combine(PackagePath, TEXT("Obsts"));
	// Chop these 14 characters: "Package /Game/"
	// const FString ObstsPackageAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), ObstsPackagePath.RightChop(14)));
	// FImportUtils::VerifyOrCreateDirectory(ObstsPackageAbsolutePath);
	// UPackage* ObstRootPackage = CreatePackage(*ObstsPackagePath);
	SimAsset->ObstructionsDirectory = PackagePath.RightChop(8);
	FImportUtils::SplitPath(SimAsset->SimInfo.ObstPaths[0], OriginalDataDirectory, Temp);
	SimAsset->SimInfo.OriginalObstFilesPath = OriginalDataDirectory;
	for (FString& ObstPath : SimAsset->SimInfo.ObstPaths)
	{
		FString ObstFullPath = FPaths::Combine(Directory, ObstPath);
		UObject* Obst = CreateObstruction(InParent, ObstFullPath, true); // ObstRootPackage

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			InParent->GetName(), FPackageName::GetAssetPackageExtension()); // ObstRootPackage
		UPackage::Save(Cast<UPackage>(InParent), Obst, RF_Standalone | RF_Public, *PackageFileName); // ObstRootPackage
	}

	// const FString SlicesPackagePath = FPaths::Combine(PackagePath, TEXT("Slices"));
	// FImportUtils::VerifyOrCreateDirectory(SlicesPackagePath);
	// UPackage* SliceRootPackage = CreatePackage(*SlicesPackagePath);
	SimAsset->SlicesDirectory = PackagePath.RightChop(8);
	FImportUtils::SplitPath(SimAsset->SimInfo.SlicePaths[0], OriginalDataDirectory, Temp);
	SimAsset->SimInfo.OriginalSliceFilesPath = OriginalDataDirectory;
	for (FString& SlicePath : SimAsset->SimInfo.SlicePaths)
	{
		FString SliceFullPath = FPaths::Combine(Directory, SlicePath);
		UObject* Slice = CreateSlice(InParent, SliceFullPath, true); // SliceRootPackage

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			InParent->GetName(), FPackageName::GetAssetPackageExtension()); // SliceRootPackage
		UPackage::Save(Cast<UPackage>(InParent), Slice, RF_Standalone | RF_Public, *PackageFileName);
		// SliceRootPackage
	}

	// const FString VolumesPackagePath = FPaths::Combine(PackagePath, TEXT("Volumes"));
	// FImportUtils::VerifyOrCreateDirectory(VolumesPackagePath);
	// UPackage* VolumeRootPackage = CreatePackage(*VolumesPackagePath);
	SimAsset->VolumesDirectory = PackagePath.RightChop(8);
	FImportUtils::SplitPath(SimAsset->SimInfo.VolumePaths[0], OriginalDataDirectory, Temp);
	SimAsset->SimInfo.OriginalVolumeFilesPath = OriginalDataDirectory;
	for (FString& VolumePath : SimAsset->SimInfo.VolumePaths)
	{
		FString VolumeFullPath = FPaths::Combine(Directory, VolumePath);
		UObject* Volume = CreateVolume(InParent, VolumeFullPath, true); // VolumeRootPackage

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			InParent->GetName(), FPackageName::GetAssetPackageExtension()); // VolumeRootPackage
		UPackage::Save(Cast<UPackage>(InParent), Volume, RF_Standalone | RF_Public, *PackageFileName);
		// VolumeRootPackage
	}

	AdditionalImportedObjects.Add(SimAsset);

	if (!Cast<UPackage>(InParent)->MarkPackageDirty())
	{
		// Todo: Error - Something went wrong when marking the package as dirty
	}

	return SimAsset;
}

UObject* UVRSSAssetFactory::CreateVolume(UObject* InParent, const FString& FileName, const bool LazyLoad)
{
	TMap<FString, FVolumeDataInfo> DataInfos = FImportUtils::ParseSliceVolumeDataInfoFromFile(FileName);
	UVolumeAsset* OutVolume = nullptr;
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UVolumeTexture*> VolumeTextures;
		OutVolume = CreateVolumeFromFile(It.Value(), FileName, InParent, It.Key(), LazyLoad);

		OutVolume->VolumeTextures.Reserve(It.Value().Dimensions.W);

		AdditionalImportedObjects.Add(OutVolume);
	}

	// Return last volume
	return OutVolume;
}

UObject* UVRSSAssetFactory::CreateSlice(UObject* InParent, const FString& FileName, const bool LazyLoad)
{
	TMap<FString, FVolumeDataInfo> DataInfos = FImportUtils::ParseSliceVolumeDataInfoFromFile(FileName);
	USliceAsset* OutSlice = nullptr;
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UTexture2D*> SliceTextures;
		OutSlice = CreateSliceFromFile(It.Value(), FileName, InParent, It.Key(), LazyLoad);

		OutSlice->SliceTextures.Reserve(It.Value().Dimensions.W);

		AdditionalImportedObjects.Add(OutSlice);
	}

	// Return last slice
	return OutSlice;
}

UObject* UVRSSAssetFactory::CreateObstruction(UObject* InParent, const FString& FileName, const bool LazyLoad)
{
	TArray<float> BoundingBox = TArray<float>();
	FBoundaryDataInfo DataInfo = FImportUtils::ParseObstDataInfoFromFile(FileName, BoundingBox);
	TMap<FString, TMap<int, TArray<UTexture2D*>>> ObstructionTextures;
	UObstAsset* OutObst = CreateObstructionFromFile(DataInfo, FileName, InParent, LazyLoad);
	OutObst->BoundingBox = BoundingBox;

	AdditionalImportedObjects.Add(OutObst);

	// Return obstruction
	return OutObst;
}

UVolumeAsset* UVRSSAssetFactory::CreateVolumeFromFile(FVolumeDataInfo& DataInfo, const FString& FileName,
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

USliceAsset* UVRSSAssetFactory::CreateSliceFromFile(FVolumeDataInfo& DataInfo, const FString& FileName,
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

UObstAsset* UVRSSAssetFactory::CreateObstructionFromFile(FBoundaryDataInfo& DataInfo, const FString& FileName,
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
	// Todo: Else copy the data file somewhere so it can be opened

	ObstAsset->ObstInfo = DataInfo;

	return ObstAsset;
}

void UVRSSAssetFactory::LoadVolumeTextures(FVolumeDataInfo& DataInfo, const FString& Directory)
{
	uint8* LoadedArray = FImportUtils::LoadAndConvertVolumeData(1, FPaths::Combine(Directory, DataInfo.DataFileName),
	                                                            DataInfo);

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
		UPackage::Save(Cast<UPackage>(SubPackage), VolumeTexture, RF_Standalone | RF_Public, *PackageFileName);

		// Add VolumeTextures to AdditionalImportedObjects so it also gets saved in-editor.
		AdditionalImportedObjects.Add(VolumeTexture);
	}

	delete[] LoadedArray;
}

void UVRSSAssetFactory::LoadSliceTextures(FVolumeDataInfo& DataInfo, const FString& Directory)
{
	uint8* LoadedArray = FImportUtils::LoadSliceData(FPaths::Combine(Directory, DataInfo.DataFileName), DataInfo);

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
		UPackage::Save(SubPackage, SliceTexture, RF_Standalone | RF_Public, *PackageFileName);

		// Add SliceTexture to AdditionalImportedObjects so it also gets saved in-editor
		AdditionalImportedObjects.Add(SliceTexture);
	}

	delete[] LoadedArray;
}

void UVRSSAssetFactory::LoadObstTextures(FBoundaryDataInfo& DataInfo, const FString& Directory)
{
	TArray<int> Orientations;
	DataInfo.Dimensions.GetKeys(Orientations);

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
				UPackage::Save(SubPackage, ObstTexture, RF_Standalone | RF_Public, *PackageFileName);

				AdditionalImportedObjects.Add(ObstTexture);

				Offset += SingleTextureSize;
			}
		}
		FMemory::Free(LoadedArray);
	}
}
