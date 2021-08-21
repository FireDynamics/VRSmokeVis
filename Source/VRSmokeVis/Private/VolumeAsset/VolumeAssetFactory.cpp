

#include "VolumeAsset//VolumeAssetFactory.h"
#include "Containers/UnrealString.h"
#include "Engine/VolumeTexture.h"
#include "VolumeAsset/VolumeLoader.h"
#include "VolumeAsset/VolumeAsset.h"

DEFINE_LOG_CATEGORY(LogAssetFactory)


UVolumeAssetFactory::UVolumeAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("yaml;")) + NSLOCTEXT("UYamlVolumeTextureFactory", "FormatYaml", ".yaml File").ToString());

	SupportedClass = UVolumeAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	ImportPriority = 1;
}

#pragma optimize("", off)
UObject* UVolumeAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	TMap<FString, FVolumeInfo> VolumeInfos = UVolumeLoader::ParseVolumeInfoFromHeader(Filename);
	UVolumeAsset* OutVolume = nullptr;
	for (auto It = VolumeInfos.CreateIterator(); It; ++It)
	{
		TArray<UVolumeTexture *> VolumeTextures;
		OutVolume = UVolumeLoader::CreateVolumeFromFile(It.Value(), Filename, InParent, It.Key(), VolumeTextures);
	
		OutVolume->VolumeTextures.Reserve(It.Value().Dimensions.W);
		
		// Add VolumeTextures to AdditionalImportedObjects so it also gets saved in-editor.
		for (UVolumeTexture *VolumeTexture : VolumeTextures)
		{
			AdditionalImportedObjects.Add(VolumeTexture);		
		}
		AdditionalImportedObjects.Add(OutVolume);
	}
	bOutOperationCanceled = false;
	
	// Return last volume
	return OutVolume;
}
#pragma optimize("", on)
