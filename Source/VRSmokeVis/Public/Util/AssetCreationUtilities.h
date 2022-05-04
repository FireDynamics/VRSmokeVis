#pragma once

#include "Assets/VolumeDataInfo.h"
#include "Assets/SliceDataInfo.h"
#include "Assets/BoundaryDataInfo.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAssetUtils, All, All);


/**
 * Utility functions to load data from the intermediate file format (.yaml) and create the corresponding data assets.
 */
class VRSMOKEVIS_API FAssetCreationUtils
{
public:
	/** Loads all Textures for an asset with the specified type */
	static void LoadTextures(UDataInfo* DataInfo, const FString& Type, const FString& Directory);
	
	/** Loads all Textures for a specific obst */
	static void LoadObstTextures(UBoundaryDataInfo* DataInfo, const FString& Directory);

	/** Loads all Textures for a specific slice */
	static void LoadSliceTextures(USliceDataInfo* DataInfo, const FString& Directory);

	/** Loads all VolumeTextures for a specific volume */
	static void LoadVolumeTextures(UVolumeDataInfo* DataInfo, const FString& Directory);

	/** Prepares the simulation asset and also loads all assets defined in the simulation */
	static UObject* CreateSimulation(const FString& InFileName, const FString& OutDirectory);
	
protected:
	/** Loads the information about an obstruction and creates the obstruction asset */
	static void LoadAndCreateObstruction(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	
	/** Loads the information about multiple slices read from a .yaml file and creates the slice assets */
	static void LoadSlice(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	/** Creates a single slice containing (meta-)data for one mesh (basically a subslice) */
	static class USliceAsset* CreateSlice(USliceDataInfo* DataInfo, const FString& FileName, UObject* Package,
										   const FString& MeshName, const bool LazyLoad);
	/** Loads the information about multiple volumes read from a .yaml file and creates the volume assets */
	static void LoadVolumes(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	/** Creates a single volume containing smoke (meta-)data for one mesh */
	static class UVolumeAsset* CreateVolume(UVolumeDataInfo* DataInfo, const FString& FileName, UObject* Package,
											 const FString& MeshName, const bool LazyLoad);
};
