// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#include "VolumeAsset//VolumeAssetFactory.h"
#include "Containers/UnrealString.h"
#include "Engine/VolumeTexture.h"
#include "VolumeAsset/Loaders/MHDLoader.h"
#include "VolumeAsset/VolumeAsset.h"

/* UMHDVolumeTextureFactory structors
 *****************************************************************************/

UVolumeAssetFactory::UVolumeAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("mhd;")) + NSLOCTEXT("UMHDVolumeTextureFactory", "FormatMhd", ".mhd File").ToString());

	SupportedClass = UVolumeAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	ImportPriority = 1;
}

#pragma optimize("", off)
UObject* UVolumeAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	IVolumeLoader* Loader;

	FString FileNamePart, FolderPart, ExtensionPart;
	FPaths::Split(Filename, FolderPart, FileNamePart, ExtensionPart);
	if (ExtensionPart.Equals(TEXT("mhd")))
	{
		Loader = UMHDLoader::Get();
	}
	else
	{
		return nullptr;
	}

	FVolumeInfo Info = Loader->ParseVolumeInfoFromHeader(Filename);
	if (!Info.bParseWasSuccessful)
	{
		return nullptr;
	}

	UVolumeAsset* OutVolume = Loader->CreateVolumeFromFile(Filename, dynamic_cast<UPackage*>(InParent));
	bOutOperationCanceled = false;

	// Add to AdditionalImportedObjects so it also gets saved in-editor.
	for (int i = 0; i < OutVolume->DataTextures.Num(); i++) {
		AdditionalImportedObjects.Add(OutVolume->DataTextures[i]);
	}

	return OutVolume;
}
#pragma optimize("", on)
