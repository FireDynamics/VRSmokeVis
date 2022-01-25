﻿#pragma once

#include "VRSSConfig.h"
#include "Engine/StreamableManager.h"
#include "Engine/ObjectLibrary.h"
#include "VRSSGameInstance.generated.h"

DECLARE_EVENT_OneParam(UVRSSGameInstance, FUpdateDataEvent, int)

DECLARE_EVENT_ThreeParams(UVRSSGameInstance, FColorMapUpdateEvent, FString, float, float)


UCLASS()
class VRSMOKEVIS_API UVRSSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UVRSSGameInstance();

	virtual void Init() override;

	FUpdateDataEvent& RegisterTextureLoad(const FString Type, const FString& Directory,
	                                      TArray<FAssetData>* TextureArray);

	UFUNCTION()
	void InitUpdateRate(const FString Type, float UpdateRateSuggestion);

	UFUNCTION(BlueprintSetter)
	void SetUpdateRate(const FString Type, const float NewUpdateRate);

	UFUNCTION()
	void FastForwardSimulation(const float Amount);

	UFUNCTION()
	void RewindSimulation(const float Amount);

	UFUNCTION()
	void TogglePauseSimulation();

	UFUNCTION()
	void ToggleHUDVisibility() const;

	/** Returns the unique quantities of all active slices. */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetActiveSliceQuantities() const;

	UFUNCTION(BlueprintCallable)
	void GetActiveSlicesMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const;

	UFUNCTION(BlueprintCallable)
	void AddSlice(class ASlice* Slice);

	UFUNCTION(BlueprintCallable)
	void RemoveSlice(class ASlice* Slice);

	/** Returns the unique quantities of all active obstructions. */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetActiveObstQuantities() const;

	UFUNCTION(BlueprintCallable)
	void GetActiveObstructionsMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const;

	UFUNCTION(BlueprintCallable)
	void AddObst(class AObst* Obst);

	UFUNCTION(BlueprintCallable)
	void RemoveObst(class AObst* Obst);

	UFUNCTION(BlueprintCallable)
	void ChangeObstQuantity(class AObst* Obst);

	UFUNCTION(BlueprintCallable)
	void GetActiveMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const;

protected:
	UFUNCTION()
	void NextTimeStep(const FString Type);

public:
	/** An instance of the configuration for the project which simply uses its default values set in the editor. */
	UPROPERTY(EditAnywhere)
	UVRSSConfig* Config;

	/** The class of the raymarch lights that are dimmed over time. */
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ARaymarchLight> RaymarchLightClass;

	/** Controls the maximum radius of Jitter that is applied (works as a factor). Defaults to 0 (no Jitter). */
	UPROPERTY(EditAnywhere)
	float JitterRadius = 0;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int> CurrentTimeSteps;

	/** Controls the time between two updates of RaymarchingVolume textures. Defaults to the rate specified by the input from FDS if set to 0. */
	UPROPERTY(VisibleAnywhere)
	TMap<FString, float> UpdateRates;

	FColorMapUpdateEvent ColorMapUpdateEvent;
protected:
	/** Lists of currently active slices. */
	UPROPERTY(VisibleAnywhere)
	TArray<class ASlice*> ActiveSlices;

	/** Lists of currently active obstructions. */
	UPROPERTY(VisibleAnywhere)
	TArray<class AObst*> ActiveObstructions;

	/** Used to asynchronously load assets at runtime. */
	FStreamableManager* StreamableManager;

	/** Handles to manage the update timer. **/
	TMap<FString, FTimerHandle> UpdateTimerHandles;

	TMap<FString, FUpdateDataEvent> UpdateDataEvents;

	/** Contains all the arrays for which assets will be loaded. */
	TMap<FString, TArray<TArray<FAssetData>*>> TextureArrays;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int> MaxTimeSteps;

	bool bIsPaused = false;
};
