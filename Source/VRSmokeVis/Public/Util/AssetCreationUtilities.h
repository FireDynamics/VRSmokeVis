#pragma once

#include "Assets/VolumeDataInfo.h"
#include "Assets/SliceDataInfo.h"
#include "Assets/BoundaryDataInfo.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAssetUtils, All, All);


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
	
	static UObject* CreateSimulation(const FString& InFileName, const FString& OutDirectory);
	
protected:
	static void CreateObstruction(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	static class UObstAsset* CreateObstructionFromFile(UBoundaryDataInfo* DataInfo, const FString& FileName, UObject* Package,
												const bool LazyLoad);
	static void CreateSlice(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	static class USliceAsset* CreateSliceFromFile(USliceDataInfo* DataInfo, const FString& FileName, UObject* Package,
										   const FString& MeshName, const bool LazyLoad);
	static void CreateVolume(const FString& RootPackage, const FString& FileName, const bool LazyLoad);
	static class UVolumeAsset* CreateVolumeFromFile(UVolumeDataInfo* DataInfo, const FString& FileName, UObject* Package,
											 const FString& MeshName, const bool LazyLoad);
};
