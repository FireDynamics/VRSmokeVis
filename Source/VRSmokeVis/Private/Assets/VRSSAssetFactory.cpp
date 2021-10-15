#include "Assets/VRSSAssetFactory.h"

#include "Assets/TextureUtilities.h"
#include "Assets/VolumeAsset.h"
#include "Assets/SliceAsset.h"
#include "Containers/UnrealString.h"
#include "Engine/VolumeTexture.h"

DEFINE_LOG_CATEGORY(LogAssetFactory)


UVRSSAssetFactory::UVRSSAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("yaml;")) + NSLOCTEXT("UYamlVolumeTextureFactory", "FormatYaml", ".yaml File").ToString());

	SupportedClass = UVolumeAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	ImportPriority = 1;
}

#pragma optimize("", off)
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

	bOutOperationCanceled = false;

	return Out;
}

UObject* UVRSSAssetFactory::CreateVolume(UObject* InParent, const FString& Filename)
{
	TMap<FString, FDataInfo> VolumeInfos = FTextureUtils::ParseVolumeInfoFromHeader(Filename);
	UVolumeAsset* OutVolume = nullptr;
	for (auto It = VolumeInfos.CreateIterator(); It; ++It)
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
	TMap<FString, FDataInfo> VolumeInfos = FTextureUtils::ParseVolumeInfoFromHeader(Filename);
	USliceAsset* OutSlice = nullptr;
	for (auto It = VolumeInfos.CreateIterator(); It; ++It)
	{
		TArray<UTexture2D*> SliceTextures;
		OutSlice = CreateSliceFromFile(It.Value(), Filename, InParent, It.Key(), SliceTextures);

		OutSlice->SliceTextures.Reserve(It.Value().Dimensions.W);

		// Add VolumeTextures to AdditionalImportedObjects so it also gets saved in-editor.
		for (UTexture2D* SliceTexture : SliceTextures)
		{
			AdditionalImportedObjects.Add(SliceTexture);
		}
		AdditionalImportedObjects.Add(OutSlice);
	}
	
	// Return last slice
	return OutSlice;
}

UVolumeAsset* UVRSSAssetFactory::CreateVolumeFromFile(FDataInfo& VolumeInfo, const FString& FileName, UObject* Package,
                                                  const FString& MeshName, TArray<UVolumeTexture*> OutVolumeTextures) const
{
	// Get valid package name and filepath.
	FString Directory, VolumeName, PackagePath;
	FTextureUtils::SplitPath(Package->GetFullName(), PackagePath, VolumeName);
	FTextureUtils::SplitPath(FileName, Directory, VolumeName);

	// Create persistent volume asset.
	UVolumeAsset* VolumeAsset = NewObject<UVolumeAsset>(Package, UVolumeAsset::StaticClass(),
	                                                    FName("VA_" + VolumeName + "_" + MeshName),
	                                                    RF_Standalone | RF_Public);

	uint8* LoadedArray = FTextureUtils::LoadAndConvertVolumeData(FPaths::Combine(Directory, VolumeInfo.DataFileName), VolumeInfo);

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
		FTextureUtils::CreateTextureAssets<UVolumeTexture>(VolumeTexture, VolumeTextureName, VolumeInfo,
		                                                 SubPackage, LoadedArray + SingleTextureSize * t);
		VolumeTexture->Filter = TF_Bilinear;

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Cast<UPackage>(SubPackage), VolumeTexture, RF_Standalone | RF_Public, *PackageFileName);

		OutVolumeTextures.Add(VolumeTexture);
	}
	VolumeAsset->VolumeInfo = VolumeInfo;

	delete[] LoadedArray;

	return VolumeAsset;
}

USliceAsset* UVRSSAssetFactory::CreateSliceFromFile(FDataInfo& VolumeInfo, const FString& FileName, UObject* Package,
                                                  const FString& MeshName, TArray<UTexture2D*> OutTextures) const
{
	// Get valid package name and filepath.
	FString Directory, SliceName, PackagePath;
	FTextureUtils::SplitPath(Package->GetFullName(), PackagePath, SliceName);
	FTextureUtils::SplitPath(FileName, Directory, SliceName);

	// Create persistent volume asset.
	USliceAsset* SliceAsset = NewObject<USliceAsset>(Package, USliceAsset::StaticClass(),
	                                                    FName("SA_" + SliceName + "_" + MeshName),
	                                                    RF_Standalone | RF_Public);

	uint8* LoadedArray = FTextureUtils::LoadSliceData(FPaths::Combine(Directory, VolumeInfo.DataFileName), VolumeInfo);

	// Create the persistent volume textures.
	for (int t = 0; t < VolumeInfo.Dimensions.W; ++t)
	{
		const long SingleTextureSize = static_cast<long>(VolumeInfo.Dimensions.X) * VolumeInfo.Dimensions.Y * VolumeInfo
			.Dimensions.Z;
		const FString SliceTextureName = "ST_" + SliceName + "_Data_t" + FString::FromInt(t);
		VolumeInfo.VolumeTextureDir = FPaths::Combine(PackagePath.RightChop(8), SliceName + "_" + MeshName);
		UPackage* SubPackage = CreatePackage(*FPaths::Combine(VolumeInfo.VolumeTextureDir, SliceTextureName));

		// The following function call expects the first two dimensions to be the ones describing the texture
		// dimensions, we therefore might have to swap them now
		const bool SwapX = VolumeInfo.Dimensions.X == 1;
		const bool SwapY = VolumeInfo.Dimensions.Y == 1;
		if (SwapX)
		{
			const float Tmp = VolumeInfo.Dimensions.X;
			VolumeInfo.Dimensions.X = VolumeInfo.Dimensions.Y;
			VolumeInfo.Dimensions.Y = VolumeInfo.Dimensions.Z;
			VolumeInfo.Dimensions.Z = Tmp;
		}
		if (SwapY)
		{
			const float Tmp = VolumeInfo.Dimensions.Y;
			VolumeInfo.Dimensions.Y = VolumeInfo.Dimensions.Z;
			VolumeInfo.Dimensions.Z = Tmp;
		}
		
		// Set pointer to current Volume position at timestep t
		UTexture2D* SliceTexture;
		FTextureUtils::CreateTextureAssets<UTexture2D>(SliceTexture, SliceTextureName, VolumeInfo,
														SubPackage, LoadedArray + SingleTextureSize * t);
		SliceTexture->Filter = TF_Default;
		
		// Correct the dimensions again
		if (SwapX)
		{
			const float Tmp = VolumeInfo.Dimensions.X;
			VolumeInfo.Dimensions.X = VolumeInfo.Dimensions.Z;
			VolumeInfo.Dimensions.Z = VolumeInfo.Dimensions.Y;
			VolumeInfo.Dimensions.Y = Tmp;
		}
		if (SwapY)
		{
			const float Tmp = VolumeInfo.Dimensions.Z;
			VolumeInfo.Dimensions.Z = VolumeInfo.Dimensions.Y;
			VolumeInfo.Dimensions.Y = Tmp;
		}
		

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			SubPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Cast<UPackage>(SubPackage), SliceTexture, RF_Standalone | RF_Public, *PackageFileName);

		OutTextures.Add(SliceTexture);
	}
	SliceAsset->SliceInfo = VolumeInfo;

	delete[] LoadedArray;

	return SliceAsset;
}

#pragma optimize("", on)



