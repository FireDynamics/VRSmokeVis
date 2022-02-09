// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/ImportUtilities.h"
#include "HAL/FileManagerGeneric.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/FileHelper.h"


DEFINE_LOG_CATEGORY(LogImportUtils);


FString FImportUtils::ReadFileAsString(const FString& FileName)
{
	FString FileContent;
	// First, try to read FileName as absolute path
	if (FFileHelper::LoadFileToString(FileContent, *FileName))
	{
		return FileContent;
	}

	// If that didn't work, try it as a relative path
	const FString RelativePath = FPaths::ProjectContentDir();
	if (const FString FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelativePath) +
		FileName; FFileHelper::LoadFileToString(FileContent, *FullPath))
	{
		return FileContent;
	}
	UE_LOG(LogImportUtils, Error, TEXT("Cannot read file path %s either as absolute or as relative path."), *FileName);
	return "";
}

TArray<FString> FImportUtils::GetFilesInFolder(const FString& Directory, const FString& Extension)
{
	TArray<FString> OutPut;
	OutPut.Empty();
	if (FPaths::DirectoryExists(Directory))
	{
		FFileManagerGeneric::Get().FindFiles(OutPut, *Directory, *Extension);
	}
	return OutPut;
}

void FImportUtils::SplitPath(const FString& FullPath, FString& OutFilePath, FString& OutPackageName)
{
	FString ExtensionPart;

	FPaths::Split(FullPath, OutFilePath, OutPackageName, ExtensionPart);
	OutPackageName = FPaths::MakeValidFileName(OutPackageName);
	// Dots are not cool in package names -> replace with underscores
	OutPackageName.ReplaceCharInline('.', '_');
}

uint8* FImportUtils::LoadDatFileIntoArray(const FString FileName, const int64 BytesToLoad)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*FileName);
	if (!FileHandle)
	{
		UE_LOG(LogImportUtils, Error, TEXT("Data file %s could not be opened."), *FileName);
		return nullptr;
	}
	if (FileHandle->Size() < BytesToLoad)
	{
		UE_LOG(LogImportUtils, Error,
		       TEXT("Data file %s does not have the expected size of (at least) %d bytes, aborting."), *FileName,
		       BytesToLoad);
		delete FileHandle;
		return nullptr;
	}

	uint8* LoadedArray = new uint8[BytesToLoad];
	FileHandle->Read(LoadedArray, BytesToLoad);
	delete FileHandle;

	return LoadedArray;
}

void FImportUtils::DensityToTransmission(const float ExtinctionCoefficient, const FVolumeDataInfo& DataInfo,
                                          uint8* Array)
{
	// Uses the Beer-Lambert law to convert densities to the corresponding transmission using the extinction coefficient
	// Adding 0.5 before assigning the float value to the uint8 array causes it to round correctly without having to
	// round manually, as the implicit conversion to an integer simply cuts off the fraction.
	float StepSize = 0.001; // DataInfo.Spacing.Size3();

	// Multiply StepSize and extinction coefficient only once before looping over the array
	StepSize *= ExtinctionCoefficient * -1;

	ParallelFor(DataInfo.GetByteSize(), [&](const int Idx)
	{
		Array[Idx] = FMath::Exp(StepSize * Array[Idx]) * 255.f + .5f;
	});
}

void FImportUtils::NormalizeArray(const FVolumeDataInfo& DataInfo, uint8* Array)
{
	const float ValueRange = 255.f / (DataInfo.MaxValue - DataInfo.MinValue);
	ParallelFor(DataInfo.GetByteSize(), [&](const int Idx)
	{
		Array[Idx] = (Array[Idx] - DataInfo.MinValue) * ValueRange;
	});
}

uint8* FImportUtils::LoadAndConvertVolumeData(const float ExtinctionCoefficient, const FString& FilePath,
                                               const FVolumeDataInfo& DataInfo)
{
	// Load data
	uint8* LoadedArray = LoadDatFileIntoArray(FilePath, DataInfo.GetByteSize());
	if (LoadedArray)
	{
		DensityToTransmission(ExtinctionCoefficient, DataInfo, LoadedArray);
	}
	return LoadedArray;
}

uint8* FImportUtils::LoadSliceData(const FString& FilePath, const FVolumeDataInfo& DataInfo)
{
	return LoadDatFileIntoArray(FilePath, DataInfo.GetByteSize());
}

uint8* FImportUtils::LoadObstData(const FString& FilePath, const FBoundaryDataInfo& DataInfo)
{
	int TotalByteSize = 0;
	TArray<int> Orientations;
	DataInfo.Dimensions.GetKeys(Orientations);
	for (const int Orientation : Orientations)
	{
		TotalByteSize += DataInfo.GetByteSize(Orientation);
	}
	return LoadDatFileIntoArray(FilePath, TotalByteSize);
}

TMap<FString, FVolumeDataInfo> FImportUtils::ParseSliceVolumeDataInfoFromFile(const FString& FileName)
{
	TMap<FString, FVolumeDataInfo> DataInfos;

	const FString FileString = ReadFileAsString(FileName);
	TArray<FString> Lines;
	FileString.ParseIntoArray(Lines, _T("\n"));

	FString Left, Right, MeshId;

	// DataValMax
	Lines[0].Split(TEXT(": "), &Left, &Right);
	float DataMax;
	FDefaultValueHelper::ParseFloat(Right, DataMax);

	// DataValMin
	Lines[1].Split(TEXT(": "), &Left, &Right);
	float DataMin;
	FDefaultValueHelper::ParseFloat(Right, DataMin);

	// MeshNum
	Lines[2].Split(TEXT(": "), &Left, &Right);
	int NMeshes;
	FDefaultValueHelper::ParseInt(Right, NMeshes);

	// Quantity
	FString Quantity;
	Lines[Lines.Num() - 2].Split(TEXT(": "), &Left, &Quantity);
	Quantity.TrimStartAndEndInline();
	Quantity.ToLowerInline();

	//ScaleFactor
	Lines[Lines.Num() - 1].Split(TEXT(": "), &Left, &Right);
	float ScaleFactor;
	FDefaultValueHelper::ParseFloat(Right, ScaleFactor);

	UE_LOG(LogImportUtils, Log, TEXT("Loading volumes, nmeshes: %d"), NMeshes);
	// Meshes
	for (int m = 0; m < NMeshes; ++m)
	{
		FVolumeDataInfo DataInfo;
		DataInfo.MaxValue = DataMax;
		DataInfo.MinValue = DataMin;

		for (int i = 0; i < 5; ++i)
		{
			Lines[4 + m * 5 + i].Split(TEXT(": "), &Left, &Right);
			if (Left.Contains(TEXT("MeshPos")))
			{
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, DataInfo.MeshPos.X);
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, DataInfo.MeshPos.Y);
				FDefaultValueHelper::ParseFloat(Right, DataInfo.MeshPos.Z);
			}
			else if (Left.Contains(TEXT("Mesh")))
			{
				MeshId = Right.TrimStartAndEnd();
			}
			else if (Left.Contains(TEXT("DimSize")))
			{
				int Val;
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseInt(Left, Val);
				DataInfo.Dimensions.W = Val;
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseInt(Left, Val);
				DataInfo.Dimensions.X = Val;
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseInt(Left, Val);
				DataInfo.Dimensions.Y = Val;
				FDefaultValueHelper::ParseInt(Right, Val);
				DataInfo.Dimensions.Z = Val;
			}
			else if (Left.Contains(TEXT("Spacing")))
			{
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, DataInfo.Spacing.W);
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, DataInfo.Spacing.X);
				Right.Split(TEXT(" "), &Left, &Right);
				FDefaultValueHelper::ParseFloat(Left, DataInfo.Spacing.Y);
				FDefaultValueHelper::ParseFloat(Right, DataInfo.Spacing.Z);
			}
			else if (Left.Contains(TEXT("DataFile")))
			{
				DataInfo.DataFileName = Right.TrimStartAndEnd();
				UE_LOG(LogImportUtils, Log, TEXT("Found datafile %s"), *DataInfo.DataFileName);
			}
		}

		DataInfo.WorldDimensions = DataInfo.Spacing * FVector(DataInfo.Dimensions);
		DataInfo.ScaleFactor = ScaleFactor;
		DataInfo.Quantity = Quantity;
		DataInfos.Add(MeshId, DataInfo);
	}

	return DataInfos;
}

FBoundaryDataInfo FImportUtils::ParseObstDataInfoFromFile(const FString& FileName, TArray<float>& BoundingBoxOut)
{
	FBoundaryDataInfo DataInfo;

	const FString FileString = ReadFileAsString(FileName);
	TArray<FString> Lines;
	FileString.ParseIntoArray(Lines, _T("\n"));

	FString Left, Right;

	// TimeSteps
	int TimeSteps;
	Lines[Lines.Num() - 1].Split(TEXT(": "), &Left, &Right);
	FDefaultValueHelper::ParseInt(Right, TimeSteps);

	// BoundingBox
	float Value;
	Lines[0].Split(TEXT(": "), &Left, &Right);
	Right.Split(TEXT(" "), &Left, &Right);
	FDefaultValueHelper::ParseFloat(Left, Value);
	BoundingBoxOut.Add(Value);
	Right.Split(TEXT(" "), &Left, &Right);
	FDefaultValueHelper::ParseFloat(Left, Value);
	BoundingBoxOut.Add(Value);
	Right.Split(TEXT(" "), &Left, &Right);
	FDefaultValueHelper::ParseFloat(Left, Value);
	BoundingBoxOut.Add(Value);
	Right.Split(TEXT(" "), &Left, &Right);
	FDefaultValueHelper::ParseFloat(Left, Value);
	BoundingBoxOut.Add(Value);
	Right.Split(TEXT(" "), &Left, &Right);
	FDefaultValueHelper::ParseFloat(Left, Value);
	BoundingBoxOut.Add(Value);
	FDefaultValueHelper::ParseFloat(Right, Value);
	BoundingBoxOut.Add(Value);

	// NumOrientations
	Lines[1].Split(TEXT(": "), &Left, &Right);
	int NumOrientations;
	FDefaultValueHelper::ParseInt(Right, NumOrientations);

	// NumQuantities
	Lines[2].Split(TEXT(": "), &Left, &Right);
	int NumQuantities;
	FDefaultValueHelper::ParseInt(Right, NumQuantities);

	DataInfo.TextureDirs.Reserve(NumQuantities);
	DataInfo.DataFileNames.Reserve(NumQuantities);
	DataInfo.Dimensions.Reserve(NumOrientations);
	DataInfo.Spacings.Reserve(NumOrientations);
	DataInfo.WorldDimensions.Reserve(NumOrientations);
	DataInfo.MinValues.Reserve(NumQuantities);
	DataInfo.MaxValues.Reserve(NumQuantities);
	DataInfo.ScaleFactors.Reserve(NumQuantities);

	// Orientations
	for (int o = 0; o < NumOrientations; ++o)
	{
		// BoundaryOrientation
		int Orientation;
		Lines[4 + o * 3].Split(TEXT(": "), &Left, &Right);
		FDefaultValueHelper::ParseInt(Right, Orientation);

		// DimSize
		int DimX, DimY;
		Lines[4 + o * 3 + 1].Split(TEXT(": "), &Left, &Right);
		Right.Split(TEXT(" "), &Left, &Right);
		FDefaultValueHelper::ParseInt(Left, DimX);
		FDefaultValueHelper::ParseInt(Right, DimY);
		DataInfo.Dimensions.Add(Orientation, FVector4(DimX, DimY, 0, TimeSteps));

		// Spacing
		float X, Y, W;
		Lines[4 + o * 3 + 2].Split(TEXT(": "), &Left, &Right);
		Right.Split(TEXT(" "), &Left, &Right);
		FDefaultValueHelper::ParseFloat(Left, W);
		Right.Split(TEXT(" "), &Left, &Right);
		FDefaultValueHelper::ParseFloat(Left, X);
		FDefaultValueHelper::ParseFloat(Right, Y);
		DataInfo.Spacings.Add(Orientation, FVector4(X, Y, 0, W));

		DataInfo.WorldDimensions.Add(Orientation,
		                             DataInfo.Spacings[Orientation] * FVector(DataInfo.Dimensions[Orientation]));
	}

	const int QuantityOffset = 5 + NumOrientations * 3;

	UE_LOG(LogImportUtils, Log, TEXT("Loading obstruction, quantities: %d"), NumQuantities);
	// Quantities
	for (int m = 0; m < NumQuantities; ++m)
	{
		float Val;
		Lines[QuantityOffset + m * 5].Split(TEXT(": "), &Left, &Right);
		FString Quantity = Right.TrimStartAndEnd().ToLower();
		Lines[QuantityOffset + m * 5 + 1].Split(TEXT(": "), &Left, &Right);
		DataInfo.DataFileNames.Add(Quantity, Right.TrimStartAndEnd());
		Lines[QuantityOffset + m * 5 + 2].Split(TEXT(": "), &Left, &Right);
		FDefaultValueHelper::ParseFloat(Right, Val);
		DataInfo.MaxValues.Add(Quantity, Val);
		Lines[QuantityOffset + m * 5 + 3].Split(TEXT(": "), &Left, &Right);
		FDefaultValueHelper::ParseFloat(Right, Val);
		DataInfo.MinValues.Add(Quantity, Val);
		Lines[QuantityOffset + m * 5 + 4].Split(TEXT(": "), &Left, &Right);
		FDefaultValueHelper::ParseFloat(Right, Val);
		DataInfo.ScaleFactors.Add(Quantity, Val);
	}

	return DataInfo;
}

FSimulationInfo FImportUtils::ParseSimulationInfoFromFile(const FString& FileName)
{
	FSimulationInfo SimInfo;

	const FString FileString = ReadFileAsString(FileName);
	TArray<FString> Lines;
	FileString.ParseIntoArray(Lines, _T("\n"));

	FString Left, Right;

	int NumObstructions, NumSlices, NumVolumes, i;
	
	// NumObstructions
	Lines[0].Split(TEXT(": "), &Left, &Right);
	FDefaultValueHelper::ParseInt(Right, NumObstructions);
	Lines[1].Split(TEXT(": "), &Left, &Right);
	FDefaultValueHelper::ParseInt(Right, NumSlices);
	Lines[2].Split(TEXT(": "), &Left, &Right);
	FDefaultValueHelper::ParseInt(Right, NumVolumes);
	
	SimInfo.ObstPaths.Reserve(NumObstructions);
	SimInfo.SlicePaths.Reserve(NumSlices);
	SimInfo.VolumePaths.Reserve(NumVolumes);
	
	for (i = 0; i < NumObstructions; ++i) SimInfo.ObstPaths.Add(Lines[4 + i].RightChop(2).TrimEnd());
	for (i = 0; i < NumSlices; ++i) SimInfo.SlicePaths.Add(Lines[5 + NumObstructions + i].RightChop(2).TrimEnd());
	for (i = 0; i < NumVolumes; ++i) SimInfo.VolumePaths.Add(Lines[6 + NumObstructions + NumSlices + i].RightChop(2).TrimEnd());

	return SimInfo;
}