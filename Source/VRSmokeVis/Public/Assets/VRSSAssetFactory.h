

#pragma once

#include "VolumeDataInfo.h"
#include "BoundaryDataInfo.h"
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

	class UVolumeAsset* CreateVolumeFromFile(FVolumeDataInfo& DataInfo, const FString& FileName, UObject* Package,
												const FString& MeshName, TArray<UVolumeTexture*>& OutVolumeTextures) const;
	UObject* CreateSlice(UObject* InParent, const FString& Filename);
	class USliceAsset* CreateSliceFromFile(FVolumeDataInfo& DataInfo, const FString& FileName, UObject* Package,
												const FString& MeshName, TArray<UTexture2D*>& OutTextures) const;
	UObject* CreateObstruction(UObject* InParent, const FString& Filename);
	class UObstAsset* CreateObstructionFromFile(FBoundaryDataInfo& DataInfo, const FString& FileName, UObject* Package,
												TMap<FString, TMap<int, TArray<UTexture2D*>>>& OutTextures) const;	
};
