// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "VR/Grabbable.h"
#include "VolumeAsset/VolumeAsset.h"
#include "Rendering/RaymarchTypes.h"

#include "RaymarchVolume.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRaymarchVolume, Log, All);

DECLARE_DYNAMIC_DELEGATE(FOnVolumeLoaded);

UCLASS()
class VRSMOKEVIS_API ARaymarchVolume : public AActor, public IGrabbable
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties*/
	ARaymarchVolume();

	/** Called after the actor is loaded from disk in editor or when spawned in game.
		This is the last action that is performed before BeginPlay.*/
	virtual void PostRegisterAllComponents() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	/** MeshComponent that contains the raymarching cube. */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	/** Pointer to the currently used Transfer Function curve.*/
	UPROPERTY()
	UCurveLinearColor* CurrentTFCurve = nullptr;

	/** Delegate that is fired whenever a new volume is loaded. Useful if you have any UI showing info about this volume.*/
	FOnVolumeLoaded OnVolumeLoaded;

	/** Sets a new VolumeAsset and re-initializes the raymarching resources.*/
	UFUNCTION(BlueprintCallable)
	bool SetVolumeAsset(UVolumeAsset* InVolumeAsset);

protected:
	/** Initializes the Raymarch Resources to work with the provided Data Volume Texture.**/
	void InitializeRaymarchResources(UVolumeTexture* LoadedTexture);

	/** Called before initializing new Raymarch resources to free all old resources.*/
	void FreeRaymarchResources();

public:
#if WITH_EDITOR
	/** Fired when curve gradient is updated.*/
	FDelegateHandle CurveGradientUpdateDelegateHandle;

	/** Fired when curve in the associated Volume file is changed.*/
	FDelegateHandle CurveChangedInVolumeDelegateHandle;

	/** Function that is bound to the current VolumeAssets OnCurveChanged delegate (in-editor only). Gets fired when the
	 * asset's curve changes.*/
	void OnVolumeAssetChangedTF(UCurveLinearColor* Curve);

	/** Function that is bound to the current transfer function color curve and gets fired when that gets changed
	 * (e.g. when the user edits the curve in curve editor. */
	void OnTFColorCurveUpdated(UCurveBase* Curve, EPropertyChangeType::Type ChangeType);

	/** Handles in-editor changes to exposed properties.*/
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);

	/** Override ShouldTickIfViewportsOnly to return true, so this also ticks in editor viewports.*/
	virtual bool ShouldTickIfViewportsOnly() const override;

#endif

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	bool UpdateVolume();
	
	/** The loaded Volume asset belonging to this volume*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UVolumeAsset* VolumeAsset = nullptr;

	/** Only kept so that we can compare to it when a user changes the VolumeAsset. See SetVolumeAsset().*/
	UVolumeAsset* OldVolumeAsset = nullptr;

	/** The base material for intensity rendering.*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UMaterial* RaymarchMaterialBase;

	/** Dynamic material instance for intensity rendering**/
	UPROPERTY(BlueprintReadOnly, Transient)
	UMaterialInstanceDynamic* RaymarchMaterial = nullptr;

	/** Cube border mesh - this is just a cube with wireframe borders.**/
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CubeBorderMeshComponent = nullptr;

	/** Raymarch Rendering resources. These contain references to the volume texture currently used as well as buffers
	 * to fasten the light propagation.	**/
	UPROPERTY(EditAnywhere)
	FBasicRaymarchRenderingResources RaymarchResources;

	/** The number of steps to take when raymarching. This is multiplied by the volume thickness in texture space, so
	 * can be multiplied by anything from 0 to sqrt(3), Raymarcher will only take exactly this many steps when the path through the cube is equal to the length of it's side. **/
	UPROPERTY(EditAnywhere)
	float RaymarchingSteps = 150;

	/** Switches to using a new Transfer function curve.**/
	UFUNCTION(BlueprintCallable)
	void SetTFCurve(UCurveLinearColor* InTFCurve);

	/** Saves the current windowing parameters as default in the Volume Asset.*/
	void SaveCurrentParamsToVolumeAsset();

	/** Sets material Windowing Parameters. Called after changing Window Center or Width.**/
	void SetMaterialVolumeParameters();

	/** API function to get the Min and Max values of the current VolumeAsset file.**/
	UFUNCTION(BlueprintPure)
	void GetMinMaxValues(float& Min, float& Max);

	UFUNCTION(BlueprintCallable)
	void SetRaymarchSteps(float InRaymarchingSteps);
};