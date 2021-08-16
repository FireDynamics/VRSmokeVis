// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#include "VolumeAsset/VolumeAsset.h"

#include "Misc/FileHelper.h"

#include <AssetRegistryModule.h>


UVolumeAsset* UVolumeAsset::CreatePersistent(UPackage* SavePackage, const FString SaveName)
{
	UVolumeAsset* VolumeAsset =
		NewObject<UVolumeAsset>(SavePackage, StaticClass(), FName("VA_" + SaveName), RF_Standalone | RF_Public);
	if (VolumeAsset)
	{
		FAssetRegistryModule::AssetCreated(VolumeAsset);
	}
	SavePackage->MarkPackageDirty();
	return VolumeAsset;
}

#if WITH_EDITOR
void UVolumeAsset::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	const FName MemberPropertyName =
		PropertyChangedEvent.MemberProperty != nullptr ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	// If the curve property changed, broadcast the delegate.
	if (MemberPropertyName != GET_MEMBER_NAME_CHECKED(UVolumeAsset, TransferFuncCurve))
	{
		OnImageInfoChanged.Broadcast();
	}
}

void UVolumeAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName MemberPropertyName =
		PropertyChangedEvent.MemberProperty != nullptr ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	// If the curve property changed, broadcast the delegate.
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UVolumeAsset, TransferFuncCurve))
	{
		OnCurveChanged.Broadcast(TransferFuncCurve);
	}
}
#endif
