#pragma once

#include "Assets/VolumeDataInfo.h"
#include "Assets/BoundaryDataInfo.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAssetUtils, All, All);


class VRSMOKEVIS_API FAssetCreationUtils
{
public:
	/** Loads all Textures for a specific obst */
	static void LoadObstTextures(FBoundaryDataInfo& DataInfo, const FString& Directory);

	/** Loads all Textures for a specific slice */
	static void LoadSliceTextures(FVolumeDataInfo& DataInfo, const FString& Directory);

	/** Loads all VolumeTextures for a specific volume */
	static void LoadVolumeTextures(FVolumeDataInfo& DataInfo, const FString& Directory);

	static UObject* CreateSimulation(UObject* InParent, const FString& FileName);
	
protected:
	static void CreateObstruction(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	static class UObstAsset* CreateObstructionFromFile(FBoundaryDataInfo& DataInfo, const FString& FileName, UObject* Package,
												const bool LazyLoad);
	static void CreateSlice(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	static class USliceAsset* CreateSliceFromFile(FVolumeDataInfo& DataInfo, const FString& FileName, UObject* Package,
										   const FString& MeshName, const bool LazyLoad);
	static void CreateVolume(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	static class UVolumeAsset* CreateVolumeFromFile(FVolumeDataInfo& DataInfo, const FString& FileName, UObject* Package,
											 const FString& MeshName, const bool LazyLoad);
};
