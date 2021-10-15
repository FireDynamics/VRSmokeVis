// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VRSSConfig.h"
#include "Engine/StreamableManager.h"
#include "Engine/ObjectLibrary.h"
#include "VRSSGameInstance.generated.h"

DECLARE_EVENT_OneParam(UVRSSGameInstance, FUpdateVolumeEvent, int)
DECLARE_EVENT_ThreeParams(UVRSSGameInstance, FSliceUpdateEvent, FString, float, float)

UCLASS()
class VRSMOKEVIS_API UVRSSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UVRSSGameInstance();

	virtual void Init() override;

	FUpdateVolumeEvent& RegisterTextureLoad(const FString& Directory, TArray<FAssetData>* TextureArray);

	UFUNCTION()
	void InitUpdateRate(float UpdateRateSuggestion);

	UFUNCTION(BlueprintSetter)
	void SetUpdateRate(const float NewUpdateRate);

	UFUNCTION()
	void FastForwardSimulation(const float Amount);

	UFUNCTION()
	void RewindSimulation(const float Amount);

	UFUNCTION()
	void TogglePauseSimulation();

	UFUNCTION()
	void ToggleHUDVisibility() const;

protected:
	UFUNCTION()
	void NextTimeStep();

public:
	/** An instance of the configuration for the project which simply uses its default values set in the editor. */
	UPROPERTY(EditAnywhere)
	UVRSSConfig* Config;

	/** The class of the raymarch lights that are dimmed over time. */
	TSubclassOf<class ARaymarchLight> RaymarchLightClass;

	/** Controls the maximum radius of Jitter that is applied (works as a factor). Defaults to 0 (no Jitter). */
	UPROPERTY(EditAnywhere)
	float JitterRadius = 0;

	/** Controls the time between two updates of RaymarchingVolume textures. Defaults to the rate specified by the input from FDS if set to 0. */
	UPROPERTY(EditAnywhere, BlueprintSetter=SetUpdateRate)
	float UpdateRate = 0;

	/** Returns the unique quantities of all active slices. */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetActiveSliceQuantities() const;

	UFUNCTION(BlueprintCallable)
	void GetActiveSlicesMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const;

	UFUNCTION(BlueprintCallable)
	void AddSlice(class ASlice* Slice);

	UFUNCTION(BlueprintCallable)
	void RemoveSlice(class ASlice* Slice);
	
	FSliceUpdateEvent SliceUpdateEvent;

protected:
	/** Lists of currently active slices. */
	UPROPERTY(VisibleAnywhere)
	TArray<class ASlice*> ActiveSlices;
	
	/** Used to asynchronously load assets at runtime. */
	FStreamableManager* StreamableManager;

	/** Handle to manage the update timer. **/
	FTimerHandle UpdateTimerHandle;

	/** Contains all the arrays for which assets will be loaded. */
	TArray<TArray<FAssetData>*> VolumeTextureArrays;

	FUpdateVolumeEvent UpdateVolumeEvent;

	UPROPERTY(BlueprintReadOnly)
	int CurrentTimeStep = 0;

	UPROPERTY(BlueprintReadOnly)
	int MaxTimeStep = 0;

	UPROPERTY(BlueprintReadOnly)
	float SimTimeStepLength = -1;

	bool bIsPaused = false;
};
