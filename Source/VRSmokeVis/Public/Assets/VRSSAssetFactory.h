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
	GENERATED_BODY()

	UVRSSAssetFactory(const FObjectInitializer& ObjectInitializer);

public:
	/** Loads all VolumeTextures for a specific volume */
	void LoadVolumeTextures(FVolumeDataInfo& DataInfo, const FString& Directory);

	/** Loads all Textures for a specific slice */
	void LoadSliceTextures(FVolumeDataInfo& DataInfo, const FString& Directory);

	/** Loads all Textures for a specific obst */
	void LoadObstTextures(FBoundaryDataInfo& DataInfo, const FString& Directory);

protected:
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	                                   const FString& FileName, const TCHAR* Params, FFeedbackContext* Warn,
	                                   bool& bOutOperationCanceled) override;

	UObject* CreateSimulation(UObject* InParent, const FString& FileName);

	UObject* CreateVolume(UObject* InParent, const FString& FileName, const bool LazyLoad);
	class UVolumeAsset* CreateVolumeFromFile(FVolumeDataInfo& DataInfo, const FString& FileName, UObject* Package,
	                                         const FString& MeshName, const bool LazyLoad);
	UObject* CreateSlice(UObject* InParent, const FString& FileName, const bool LazyLoad);
	class USliceAsset* CreateSliceFromFile(FVolumeDataInfo& DataInfo, const FString& FileName, UObject* Package,
	                                       const FString& MeshName, const bool LazyLoad);
	UObject* CreateObstruction(UObject* InParent, const FString& FileName, const bool LazyLoad);
	class UObstAsset* CreateObstructionFromFile(FBoundaryDataInfo& DataInfo, const FString& FileName, UObject* Package,
	                                            const bool LazyLoad);
};
