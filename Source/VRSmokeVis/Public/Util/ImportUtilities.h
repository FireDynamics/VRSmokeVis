#pragma once


DECLARE_LOG_CATEGORY_EXTERN(LogImportUtils, All, All);

/**
 * Utility functions for loading (meta-)data from the file system and preprocessing that data.
 */
class VRSMOKEVIS_API FImportUtils
{
public:
	/** Tries to read the provided FileName as a file either in absolute path or relative to game folder */
	static FString ReadFileAsString(const FString& FileName);

	/** Gets all files in the specified directory. Extension is optional, if provided, will only return files with the extension */
	static TArray<FString> GetFilesInFolder(const FString& Directory, const FString& Extension);

	/** Takes a filename or a path and splits it into the directory and filename and checks if that filename contains
	 * only valid characters */
	static void SplitPath(const FString& FullPath, FString& OutFilePath, FString& OutFilename);

	/** Loads a DAT file into a newly allocated uint8* array. Loads the given number of bytes. Don't forget to delete[]
	 * after storing the data somewhere */
	static uint8* LoadDatFileIntoArray(const FString FileName, const int64 BytesToLoad);

	/** Converts an array of densities to the resulting transmission */
	static void DensityToTransmission(const float ExtinctionCoefficient, const class UVolumeDataInfo* DataInfo, uint8* Array);

	/** Normalizes an array to the full range of 0-255 (1 Byte) */
	static void NormalizeArray(const class UVolumeDataInfo* DataInfo, uint8* Array);

	/** Loads the raw data specified in the DataInfo and converts it so that it is usable with our raymarching materials */
	static uint8* LoadAndConvertVolumeData(const float ExtinctionCoefficient, const FString& FilePath,
	                                       const class UVolumeDataInfo* DataInfo);

	/** Loads the raw data specified in the DataInfo */
	static uint8* LoadSliceData(const FString& FilePath, const class USliceDataInfo* DataInfo);

	/** Loads the raw data specified in the DataInfo */
	static uint8* LoadObstData(const FString& FilePath, const class UBoundaryDataInfo* DataInfo);

	/** Get info about volumes before loading them */
	static void ParseVolumeDataInfoFromFile(const FString& FileName, UPARAM(ref) TMap<FString, class UVolumeDataInfo*>& DataInfos);

	/** Get info about slices before loading them */
	static void ParseSliceDataInfoFromFile(const FString& FileName, UPARAM(ref) TMap<FString, class USliceDataInfo*>& DataInfos);

	/** Get info about obstructions before loading them */
	static void ParseObstDataInfoFromFile(const FString& FilePath, UPARAM(ref) class UBoundaryDataInfo* DataInfo, UPARAM(ref) TArray<float>& BoundingBoxOut);

	/** Get info about the simulation and the data it contains */
	static void ParseSimulationInfoFromFile(const FString& FileName, UPARAM(ref) class USimulationInfo* SimInfo);

	/** Get the hash of a simulation without reading the whole file */
	static FString GetSimulationHashFromFile(const FString& FileName);
	
	/** If this function cannot find or create the directory, returns false */
	static bool VerifyOrCreateDirectory(const FString& TestDir);
};
