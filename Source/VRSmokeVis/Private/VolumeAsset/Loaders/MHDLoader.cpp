// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#include "VolumeAsset/Loaders/MHDLoader.h"

#include "Misc/DefaultValueHelper.h"
#include "Util/TextureUtilities.h"

UMHDLoader* UMHDLoader::Get()
{
	// Maybe get a singleton going on here?
	return NewObject<UMHDLoader>();
}

EVolumeVoxelFormat UMHDLoader::MHDFormatToVoxelFormat(const FString& MHDFormat)
{
	if (MHDFormat.Contains(TEXT("MET_UCHAR")))
	{
		return EVolumeVoxelFormat::UnsignedChar;
	}
	else if (MHDFormat.Contains(TEXT("MET_CHAR")))
	{
		return EVolumeVoxelFormat::SignedChar;
	}

	else if (MHDFormat.Contains(TEXT("MET_USHORT")))
	{
		return EVolumeVoxelFormat::UnsignedShort;
	}
	else if (MHDFormat.Contains(TEXT("MET_SHORT")))
	{
		return EVolumeVoxelFormat::SignedShort;
	}
	else if (MHDFormat.Contains(TEXT("MET_UINT")))
	{
		return EVolumeVoxelFormat::UnsignedInt;
	}
	else if (MHDFormat.Contains(TEXT("MET_INT")))
	{
		return EVolumeVoxelFormat::SignedInt;
	}
	else if (MHDFormat.Contains(TEXT("MET_FLOAT")))
	{
		return EVolumeVoxelFormat::Float;
	}
	return EVolumeVoxelFormat::Float;
}

FVolumeInfo UMHDLoader::ParseVolumeInfoFromHeader(const FString& FileName)
{
	FString FileString = ReadFileAsString(FileName);
	TArray<FString> Lines;
	int32 LineCount = FileString.ParseIntoArray(Lines, _T("\n"));

	FVolumeInfo OutVolumeInfo;

	FString Left, Right;
	for (int i = 0; i < LineCount; ++i)
	{
		Lines[i].Split(TEXT(" = "), &Left, &Right);
		if (Left.Contains(TEXT("DimSize")))
		{
			int Val;
			Right.Split(TEXT(" "), &Left, &Right);
			FDefaultValueHelper::ParseInt(Left, Val);
			OutVolumeInfo.Dimensions.W = Val;
			Right.Split(TEXT(" "), &Left, &Right);
			FDefaultValueHelper::ParseInt(Left, Val);
			OutVolumeInfo.Dimensions.X = Val;
			Right.Split(TEXT(" "), &Left, &Right);
			FDefaultValueHelper::ParseInt(Left, Val);
			OutVolumeInfo.Dimensions.Y = Val;
			FDefaultValueHelper::ParseInt(Right, Val);
			OutVolumeInfo.Dimensions.Z = Val;
		}
		else if (Left.Contains(TEXT("ElementSpacing")))
		{
			Right.Split(TEXT(" "), &Left, &Right);
			FDefaultValueHelper::ParseFloat(Left, OutVolumeInfo.Spacing.W);
			Right.Split(TEXT(" "), &Left, &Right);
			FDefaultValueHelper::ParseFloat(Left, OutVolumeInfo.Spacing.X);
			Right.Split(TEXT(" "), &Left, &Right);
			FDefaultValueHelper::ParseFloat(Left, OutVolumeInfo.Spacing.Y);
			FDefaultValueHelper::ParseFloat(Right, OutVolumeInfo.Spacing.Z);
		}
		else if (Left.Contains(TEXT("ElementType")))
		{
			OutVolumeInfo.OriginalFormat = MHDFormatToVoxelFormat(Right);
		}
		else if (Left.Contains(TEXT("ElementDataFile")))
		{
			OutVolumeInfo.DataFileName = Right;
		}
	}

	OutVolumeInfo.WorldDimensions = OutVolumeInfo.Spacing * FVector(OutVolumeInfo.Dimensions);

	OutVolumeInfo.BytesPerVoxel = FVolumeInfo::VoxelFormatByteSize(OutVolumeInfo.OriginalFormat);
	OutVolumeInfo.bIsSigned = FVolumeInfo::IsVoxelFormatSigned(OutVolumeInfo.OriginalFormat);

	OutVolumeInfo.bParseWasSuccessful = true;
	return OutVolumeInfo;
}

UVolumeAsset* UMHDLoader::CreateVolumeFromFile(const FString& FileName, UPackage* OutPackage)
{
	FVolumeInfo VolumeInfo = ParseVolumeInfoFromHeader(FileName);
	if (!VolumeInfo.bParseWasSuccessful)
	{
		return nullptr;
	}
	// Get valid package name and filepath.
	FString FilePath, VolumeName;
	GetValidPackageNameFromFileName(FileName, FilePath, VolumeName);

	// Create persistent volume asset.
	UVolumeAsset* OutAsset = UVolumeAsset::CreatePersistent(OutPackage, VolumeName);
	if (!OutAsset)
	{
		return nullptr;
	}

	uint8* LoadedArray = LoadAndConvertData(FilePath, VolumeInfo);
	EPixelFormat PixelFormat = FVolumeInfo::VoxelFormatToPixelFormat(VolumeInfo.ActualFormat);

	for (int t = 0; t < VolumeInfo.Dimensions.W; ++t)
	{
		// Create the persistent volume texture.
		FString VolumeTextureName = "VA_" + VolumeName + "_Data_t" + FString::FromInt(t);
		const long SingleTextureSize = static_cast<long>(VolumeInfo.Dimensions.X) * VolumeInfo.Dimensions.Y * VolumeInfo
			.Dimensions.Z * VolumeInfo.BytesPerVoxel;
		// Set pointer to current Volume position at timestep t
		UVolumeTexture* VolumeTexture;
		FVolumeTextureToolkit::CreateVolumeTextureAsset(
			VolumeTexture, VolumeTextureName, OutPackage, PixelFormat, VolumeInfo.Dimensions,
			LoadedArray + SingleTextureSize * t, true);
		OutAsset->DataTextures.Add(VolumeTexture);
	}

	OutAsset->ImageInfo = VolumeInfo;

	delete[] LoadedArray;

	return OutAsset;
}
