// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#pragma once

#include "Actor/RaymarchVolume.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "CoreMinimal.h"
#include "Widget/SliderAndValueBox.h"

#include <Components/ComboBoxString.h>

#include "VolumeLoadMenu.generated.h"

class ARaymarchVolume;

DECLARE_LOG_CATEGORY_EXTERN(VolumeLoadMenu, All, All)

/**
 * A menu that lets a user load new MHD files into a RaymarchVolume
 */
UCLASS()
class VRSMOKEVIS_API UVolumeLoadMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	/// Override initialize to bind button functions.
	virtual bool Initialize() override;

	///  Button that will let user load a new MHD file in normalized G16.
	UPROPERTY(meta = (BindWidget))
	UButton* LoadG16Button;

	///  Button that will let user load a new MHD file in F32 format
	UPROPERTY(meta = (BindWidget))
	UButton* LoadF32Button;

	/// Combobox for selecting loaded MHD Assets.
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* AssetSelectionComboBox;

	/// Array of existing MHD Assets that can be set immediately. Will populate the AssetSelection combo box.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<UVolumeAsset*> AssetArray;

	/// Called when LoadG16Button is clicked.
	UFUNCTION()
	void OnLoadNormalizedClicked();

	/// Called when LoadF32Button is clicked.
	UFUNCTION()
	void OnLoadF32Clicked();

	/// Unified function for loading F32 or normalized.
	UFUNCTION()
	void PerformLoad(bool bNormalized);

	/// Called when AssetSelectionComboBox has a new value selected.
	UFUNCTION()
	void OnAssetSelected(FString AssetName, ESelectInfo::Type SelectType);

	/// The volume this menu is affecting.
	/// #TODO do not touch the volume directly and expose delegates instead?
	UPROPERTY(EditAnywhere)
	TArray<ARaymarchVolume*> ListenerVolumes;

	/// Sets a new volume to be affected by this menu.
	UFUNCTION(BlueprintCallable)
	void AddListenerVolume(ARaymarchVolume* NewRaymarchVolume);

	/// Sets a new volume to be affected by this menu.
	UFUNCTION(BlueprintCallable)
	void RemoveListenerVolume(ARaymarchVolume* RemovedRaymarchVolume);
};
