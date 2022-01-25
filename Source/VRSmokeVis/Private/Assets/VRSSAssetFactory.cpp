#include "Assets/VRSSAssetFactory.h"

#include "Assets/TextureUtilities.h"
#include "Assets/VolumeAsset.h"
#include "Assets/SliceAsset.h"
#include "Assets/ObstAsset.h"
#include "Containers/UnrealString.h"
#include "Engine/VolumeTexture.h"

DEFINE_LOG_CATEGORY(LogAssetFactory)


UVRSSAssetFactory::UVRSSAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("yaml;")) + NSLOCTEXT("UYamlVolumeTextureFactory", "FormatYaml", ".yaml File").ToString());

	SupportedClass = UVolumeAsset::StaticClass(); // Todo: Is this enough, even though we are also using other classes?
	bCreateNew = false;
	bEditorImport = true;
	ImportPriority = 1;
}

UObject* UVRSSAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                              const FString& Filename, const TCHAR* Params, FFeedbackContext* Warn,
                                              bool& bOutOperationCanceled)
{
	UObject* Out = nullptr;
	if (InName.ToString().Contains(TEXT("smoke")) || Filename.Contains(TEXT("slice3D")))
	{
		Out = CreateVolume(InParent, Filename);
	}
	else if (InName.ToString().Contains(TEXT("slice2D")))
	{
		Out = CreateSlice(InParent, Filename);
	}
	else if (InName.ToString().Contains(TEXT("obst")))
	{
		Out = CreateObstruction(InParent, Filename);
	}

	bOutOperationCanceled = false;

	return Out;
}

UObject* UVRSSAssetFactory::CreateVolume(UObject* InParent, const FString& Filename)
{
	TMap<FString, FVolumeDataInfo> DataInfos = FTextureUtils::ParseSliceVolumeDataInfoFromHeader(Filename);
	UVolumeAsset* OutVolume = nullptr;
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UVolumeTexture*> VolumeTextures;
		OutVolume = CreateVolumeFromFile(It.Value(), Filename, InParent, It.Key(), VolumeTextures);

		OutVolume->VolumeTextures.Reserve(It.Value().Dimensions.W);

		// Add VolumeTextures to AdditionalImportedObjects so it also gets saved in-editor.
		for (UVolumeTexture* VolumeTexture : VolumeTextures)
		{
			AdditionalImportedObjects.Add(VolumeTexture);
		}
		AdditionalImportedObjects.Add(OutVolume);
	}

	// Return last volume
	return OutVolume;
}

UObject* UVRSSAssetFactory::CreateSlice(UObject* InParent, const FString& Filename)
{
	TMap<FString, FVolumeDataInfo> DataInfos = FTextureUtils::ParseSliceVolumeDataInfoFromHeader(Filename);
	USliceAsset* OutSlice = nullptr;
	for (auto It = DataInfos.CreateIterator(); It; ++It)
	{
		TArray<UTexture2D*> SliceTextures;
		OutSlice = CreateSliceFromFile(It.Value(), Filename, InParent, It.Key(), SliceTextures);

		OutSlice->SliceTextures.Reserve(It.Value().Dimensions.W);

		// Add SliceTextures to AdditionalImportedObjects so it also gets saved in-editor.
		for (UTexture2D* SliceTexture : SliceTextures)
		{
			AdditionalImportedObjects.Add(SliceTexture);
		}
		AdditionalImportedObjects.Add(OutSlice);
	}

	// Return last slice
	return OutSlice;
}

UObject* UVRSSAssetFactory::CreateObstruction(UObject* InParent, const FString& Filename)
{
	TArray<float> BoundingBox = TArray<float>();
	FBoundaryDataInfo DataInfo = FTextureUtils::ParseObstDataInfoFromHeader(Filename, BoundingBox);
	TMap<FString, TMap<int, TArray<UTexture2D*>>> ObstructionTextures;
	UObstAsset* OutObst = CreateObstructionFromFile(DataInfo, Filename, InParent, ObstructionTextures);
	OutObst->BoundingBox = BoundingBox;

	// Add Textures to AdditionalImportedObjects so it also gets saved in-editor.
	for (auto Quantity : ObstructionTextures)
	{
		for (auto Orientation : Quantity.Value)
		{
			for (UTexture2D* FaceTexture : Orientation.Value)
			{
				AdditionalImportedObjects.Add(FaceTexture);
			}
		}
	}
	AdditionalImportedObjects.Add(OutObst);

	// Return obstruction
	return OutObst;
}

UVolumeAsset* UVRSSAssetFactory::CreateVolumeFromFile(FVolumeDataInfo& DataInfo, const FString& FileName,
                                                      UObject* Package, const FString& MeshName,
                                                      TArray<UVolumeTexture*>& OutVolumeTextures) const
{
	// Get valid package name and filepath.
	FString Directory, VolumeName, PackagePath;
	FTextureUtils::SplitPath(Package->GetFullName(), PackagePath, VolumeName);
	FTextureUtils::SplitPath(FileName, Directory, VolumeName);

	// Create persistent volume asset.
	UVolumeAsset* VolumeAsset = NewObject<UVolumeAsset>(Package, UVolumeAsset::StaticClass(),
	                                                    FName("VA_" + VolumeName + "_" + MeshName),
	                                                    RF_Standalone | RF_Public);

	uint8* LoadedArray = FTextureUtils::LoadAndConvertVolumeData(1, FPaths::Combine(Directory, DataInfo.DataFileName),
	                                                             DataInfo);

	DataInfo.TextureDir = FPaths::Combine(PackagePath.RightChop(8), MeshName);
	// Create the persistent volume textures.
	for (int t = 0; t < DataInfo.Dimensions.W; ++t)
	{
		const long SingleTextureSize = static_cast<long>(DataInfo.Dimensions.X) * DataInfo.Dimensions.Y * DataInfo
			.Dimensions.Z;
		const FString VolumeTextureName = "VT_" + VolumeName + "_Data_t" + FString::FromInt(t);
		UPackage* SubPackage = CreatePackage(*FPaths::Combine(DataInfo.TextureDir, VolumeTextureName));

		// Set pointer to current Volume position at timestep t
		UVolumeTexture* VolumeTexture = FTextureUtils::CreateVolumeAsset(VolumeTextureName, DataInfo.Dimensions,
		                                                                 SubPackage,
		                                                                 LoadedArray + SingleTextureSize * t,
		                                                                 SingleTextureSize);
		VolumeTexture->Filter = TF_Bilinear;

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Cast<UPackage>(SubPackage), VolumeTexture, RF_Standalone | RF_Public, *PackageFileName);

		OutVolumeTextures.Add(VolumeTexture);
	}
	VolumeAsset->DataInfo = DataInfo;

	delete[] LoadedArray;

	return VolumeAsset;
}

USliceAsset* UVRSSAssetFactory::CreateSliceFromFile(FVolumeDataInfo& DataInfo, const FString& FileName,
                                                    UObject* Package,
                                                    const FString& MeshName, TArray<UTexture2D*>& OutTextures) const
{
	// Get valid package name and filepath.
	FString Directory, SliceName, PackagePath;
	FTextureUtils::SplitPath(Package->GetFullName(), PackagePath, SliceName);
	FTextureUtils::SplitPath(FileName, Directory, SliceName);

	// Create persistent slice asset.
	USliceAsset* SliceAsset = NewObject<USliceAsset>(Package, USliceAsset::StaticClass(),
	                                                 FName("SA_" + SliceName + "_" + MeshName),
	                                                 RF_Standalone | RF_Public);

	uint8* LoadedArray = FTextureUtils::LoadSliceData(FPaths::Combine(Directory, DataInfo.DataFileName), DataInfo);

	DataInfo.TextureDir = FPaths::Combine(PackagePath.RightChop(8), SliceName + "_" + MeshName);
	// Create the persistent slice textures.
	for (int t = 0; t < DataInfo.Dimensions.W; ++t)
	{
		const long SingleTextureSize = static_cast<long>(DataInfo.Dimensions.X) * DataInfo.Dimensions.Y * DataInfo
			.Dimensions.Z;
		const FString SliceTextureName = "ST_" + SliceName + "_Data_t" + FString::FromInt(t);
		UPackage* SubPackage = CreatePackage(*FPaths::Combine(DataInfo.TextureDir, SliceTextureName));

		// Set pointer to current slice position at timestep t
		UTexture2D* SliceTexture = FTextureUtils::CreateSliceTextureAsset(
			SliceTextureName, DataInfo.Dimensions, SubPackage, LoadedArray + SingleTextureSize * t, SingleTextureSize);

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(SubPackage, SliceTexture, RF_Standalone | RF_Public, *PackageFileName);

		OutTextures.Add(SliceTexture);
	}
	SliceAsset->SliceInfo = DataInfo;

	delete[] LoadedArray;

	return SliceAsset;
}

UObstAsset* UVRSSAssetFactory::CreateObstructionFromFile(FBoundaryDataInfo& DataInfo, const FString& FileName,
                                                         UObject* Package,
                                                         TMap<FString, TMap<int, TArray<UTexture2D*>>>& OutTextures)
const
{
	// Get valid package name and filepath.
	FString Directory, ObstName, PackagePath;
	FTextureUtils::SplitPath(Package->GetFullName(), PackagePath, ObstName);
	FTextureUtils::SplitPath(FileName, Directory, ObstName);

	// Create persistent obst asset
	UObstAsset* ObstAsset = NewObject<UObstAsset>(Package, UObstAsset::StaticClass(), FName("OA_" + ObstName),
	                                              RF_Standalone | RF_Public);

	TArray<int> Orientations;
	DataInfo.Dimensions.GetKeys(Orientations);

	// Iterate over all quantities
	for (const auto DataFileName : DataInfo.DataFileNames)
	{
		FString Quantity = DataFileName.Key;
		OutTextures.Add(Quantity, TMap<int, TArray<UTexture2D*>>());
		DataInfo.TextureDirs.Add(Quantity, FQuantityDir());
		uint8* LoadedArray = FTextureUtils::LoadObstData(FPaths::Combine(Directory, DataFileName.Value), DataInfo);
		int Offset = 0;

		// Set pointer to current data position at timestep t for each orientation
		for (const int Ori : Orientations)
		{
			const FString DirName = ObstName + "_" + Quantity + "_Face" + FString::FromInt(Ori);
			DataInfo.TextureDirs[Quantity].FaceDirs.Add(
				Ori, FPaths::Combine<FString>(PackagePath.RightChop(8), DirName));
			OutTextures[Quantity].Add(Ori, TArray<UTexture2D*>());
			for (int t = 0; t < DataInfo.Dimensions[Ori].W; ++t)
			{
				const long SingleTextureSize = static_cast<long>(DataInfo.Dimensions[Ori].X) * DataInfo.Dimensions[Ori].Y;
				const FString FaceTextureName = "OT_" + DirName + "_Data_t" + FString::FromInt(t);
				UPackage* SubPackage = CreatePackage(*FPaths::Combine(DataInfo.TextureDirs[Quantity].FaceDirs[Ori], FaceTextureName));
				UTexture2D* ObstTexture = FTextureUtils::CreateTextureAsset(FaceTextureName, DataInfo.Dimensions[Ori],
				                                                            SubPackage, LoadedArray + Offset,
				                                                            SingleTextureSize);
				ObstTexture->Filter = TF_Default;
				FString PackageFileName = FPackageName::LongPackageNameToFilename(
					SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
				UPackage::Save(SubPackage, ObstTexture, RF_Standalone | RF_Public, *PackageFileName);
				OutTextures[Quantity][Ori].Add(ObstTexture);

				Offset += SingleTextureSize;
			}
		}
		delete[] LoadedArray;
	}
	ObstAsset->ObstInfo = DataInfo;

	return ObstAsset;
}
