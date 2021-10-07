

#pragma once

#include "VolumeInfo.h"
#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "VRSSAssetFactory.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogAssetFactory, Log, All);

/**
 * Implements a factory for creating (volume) texture assets by drag'n'dropping .yaml files into the content browser.
 */
UCLASS(hidecategories = Object)
class UVRSSAssetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	//~ UFactory Interface
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Params, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

protected:
	UObject* CreateVolume(UObject* InParent, const FString& Filename);

	class UVolumeAsset* CreateVolumeFromFile(FVolumeInfo& VolumeInfo, const FString& FileName, UObject* Package,
												const FString& MeshName, TArray<UVolumeTexture*> OutVolumeTextures);
	class USliceAsset* CreateSliceFromFile(FVolumeInfo& VolumeInfo, const FString& FileName, UObject* Package,
												const FString& MeshName, TArray<UTexture2D*> OutTextures);
	UObject* CreateSlice(UObject* InParent, const FString& Filename);
};
