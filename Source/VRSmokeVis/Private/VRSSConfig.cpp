#include "VRSSConfig.h"

#include "Engine/ObjectLibrary.h"
#include "Engine/StreamableManager.h"


UTexture2D* UVRSSConfig::GetColorMap(const FString Quantity) const
{
	TArray<FAssetData> ColorMapTextures;
	
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UTexture2D::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(ColorMapsPath);
	ObjectLibrary->GetAssetDataList(ColorMapTextures);

	for (FAssetData& ColorMap : ColorMapTextures)
	{
		if(ColorMap.AssetName.ToString().Equals("CM_" + Quantity)){
			return Cast<UTexture2D>(StreamableManager->LoadSynchronous(ColorMap.ToSoftObjectPath()));
		}
	}
	return nullptr;
}
