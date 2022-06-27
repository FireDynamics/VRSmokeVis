#include "VRSSConfig.h"

#include "Engine/ObjectLibrary.h"


UTexture2D* UVRSSConfig::GetColorMap(const FString Quantity)
{
	TArray<FAssetData> ColorMapTextures;

	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UTexture2D::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(GetColorMapsPath());
	ObjectLibrary->GetAssetDataList(ColorMapTextures);

	if (const FString* ColorMapName = ColorMaps.Find(Quantity))
	{
		for (const FAssetData& ColorMap : ColorMapTextures)
		{
			if (ColorMap.AssetName.ToString().Equals("CM_" + *ColorMapName))
			{
				return StreamableManager.LoadSynchronous<UTexture2D>(ColorMap.ToSoftObjectPath());
			}
		}
	}
	return StreamableManager.LoadSynchronous<UTexture2D>(ColorMapTextures[0].ToSoftObjectPath());
}

float UVRSSConfig::GetSliceCutOffValue(const FString Quantity) const
{
	if (const float* CutOff = SliceCutOffValues.Find(Quantity))
		return *CutOff;
	return TNumericLimits<float>::Min();
}

float UVRSSConfig::GetObstCutOffValue(const FString Quantity) const
{
	if (const float* CutOff = ObstCutOffValues.Find(Quantity))
		return *CutOff;
	return TNumericLimits<float>::Min();
}

FString UVRSSConfig::GetColorMapsPath() const
{
	if (ColorMapsPath.IsEmpty())
	{
		// Todo: Set final plugin name
		FPaths::Combine(FPaths::ProjectPluginsDir(), "VRSmokeVis");
	}

	return ColorMapsPath;
}

FString UVRSSConfig::GetActiveObstQuantity() const
{
	return ActiveObstQuantity;
}
	
void UVRSSConfig::SetActiveObstQuantity(const FString& NewQuantity)
{
	ActiveObstQuantity = NewQuantity;

	this->SaveConfig();
}