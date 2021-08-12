// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VolumeInfo.h"

#include "VolumeAsset.Generated.h"

/// Delegate that is broadcast when the color curve is changed.
DECLARE_MULTICAST_DELEGATE_OneParam(FCurveAssetChangedDelegate, UCurveLinearColor*);

/// Delegate that is broadcast when the inner volume info is changed.
DECLARE_MULTICAST_DELEGATE(FVolumeInfoChangedDelegate);

UCLASS()
class VRSMOKEVIS_API UVolumeAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/// Volume texture containing the data loaded from the MHD file.
	UPROPERTY(VisibleAnywhere)
	TArray<UVolumeTexture*> DataTextures;

	/// A color curve that will be used as a transfer function to display this volume.
	UPROPERTY(EditAnywhere)
	UCurveLinearColor* TransferFuncCurve;

	/// Holds the general info about the MHD Volume read from disk.
	UPROPERTY(EditAnywhere)
	FVolumeInfo ImageInfo;

	static UVolumeAsset* CreatePersistent(UPackage* SavePackage, const FString SaveName);

#if WITH_EDITOR
	/// Called when the Transfer function curve is changed (as in, a different asset is selected).
	FCurveAssetChangedDelegate OnCurveChanged;

	/// Called when the inside of the volume info change.
	FVolumeInfoChangedDelegate OnImageInfoChanged;

	/// Called when inner structs get changed. Used to notify active volumes about stuff changing inside the ImageInfo struct.
	void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

	/// Called when a property is changed.
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
