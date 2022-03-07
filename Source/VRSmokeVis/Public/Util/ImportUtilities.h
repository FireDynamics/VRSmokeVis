#pragma once

#include "Assets/VolumeAsset.h"
#include "Assets/ObstAsset.h"
#include "Assets/SimulationInfo.h"


DECLARE_LOG_CATEGORY_EXTERN(LogImportUtils, All, All);


class VRSMOKEVIS_API FImportUtils
{
public:
	// Tries to read the provided FileName as a file either in absolute path or relative to game folder.
	static FString ReadFileAsString(const FString& FileName);

	// Gets all files in the specified directory. Extension is optional, if provided, will only return files with the extension.
	static TArray<FString> GetFilesInFolder(const FString& Directory, const FString& Extension);

	// Takes a filename or a path and returns a valid package name.
	// Eg. "/user/somebody/img0.0.1.yaml" 
	// OutPackageName = "img0_0_1" and OutFilePath = "/user/somebody/"
	static void SplitPath(const FString& FullPath, FString& OutFilePath, FString& OutPackageName);

	/** Loads a DAT file into a newly allocated uint8* array. Loads the given number of bytes. Don't forget to delete[]
	 * after storing the data somewhere. */
	static uint8* LoadDatFileIntoArray(const FString FileName, const int64 BytesToLoad);

	/** Converts an array of densities to the resulting transmission. */
	static void DensityToTransmission(const float ExtinctionCoefficient, const FVolumeDataInfo& DataInfo, uint8* Array);

	/** Normalizes an array to the full range of 0-255 (1 Byte). */
	static void NormalizeArray(const FVolumeDataInfo& DataInfo, uint8* Array);

	/** Loads the raw data specified in the DataInfo and converts it so that it is usable with our raymarching materials. */
	static uint8* LoadAndConvertVolumeData(const float ExtinctionCoefficient, const FString& FilePath,
	                                       const FVolumeDataInfo& DataInfo);

	/** Loads the raw data specified in the DataInfo. */
	static uint8* LoadSliceData(const FString& FilePath, const FVolumeDataInfo& DataInfo);

	/** Loads the raw data specified in the DataInfo. */
	static uint8* LoadObstData(const FString& FilePath, const FBoundaryDataInfo& DataInfo);

	/** Getting info about slices or volumes before loading them. */
	static TMap<FString, FVolumeDataInfo> ParseSliceVolumeDataInfoFromFile(const FString& FileName);

	/** Getting info about obstructions before loading them. */
	static FBoundaryDataInfo ParseObstDataInfoFromFile(const FString& FilePath, TArray<float>& BoundingBoxOut);

	static FSimulationInfo ParseSimulationInfoFromFile(const FString& FileName);

	/** If this function cannot find or create the directory, returns false. */
	static FORCEINLINE bool VerifyOrCreateDirectory(const FString& TestDir)
	{
		if (IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			!PlatformFile.DirectoryExists(*TestDir))
		{
			PlatformFile.CreateDirectory(*TestDir);

			if (!PlatformFile.DirectoryExists(*TestDir)) return false;
		}
		return true;
	}
};
