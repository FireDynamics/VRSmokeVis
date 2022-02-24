#pragma once

#include "Simulation.generated.h"

DECLARE_EVENT_OneParam(UVRSSGameInstance, FUpdateDataEvent, int)


UCLASS()
class VRSMOKEVIS_API ASimulation : public AActor
{
	GENERATED_BODY()

public:
	ASimulation();
	
	UFUNCTION(BlueprintCallable)
	void ToggleControllerUI();

	UFUNCTION()
	void CheckObstActivations(const TArray<bool> ObstsActive);
	UFUNCTION()
	void CheckSliceActivations(const TArray<bool> SlicesActive);
	UFUNCTION()
	void CheckVolumeActivations(const TArray<bool> VolumesActive);

	TOptional<FUpdateDataEvent*> RegisterTextureLoad(const FString Type, const FString& Directory,
	                                                 TArray<FAssetData>* TextureArray, const int NumTextures);

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
	TArray<class AObst*>& GetAllObstructions();
	
	UFUNCTION()
	TArray<class ASlice*>& GetAllSlices();
	
	UFUNCTION()
	TArray<class ARaymarchVolume*>& GetAllVolumes();

	UFUNCTION(BlueprintCallable)
	void ChangeObstQuantity(class AObst* Obst);
	
	UFUNCTION(BlueprintCallable)
	void GetMaxMins(TMap<FString, float> Mins, TMap<FString, float> Maxs) const;

	UFUNCTION(BlueprintCallable)
	void GetMaxMinForQuantity(FString Quantity, float& MinOut, float& MaxOut) const;
	
	UFUNCTION(BlueprintCallable)
	void UpdateColorMaps(const TMap<FString, float> Mins, const TMap<FString, float> Maxs);
	
	/** Returns all unique quantities present in this simulation. */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetQuantities(const bool ActiveOnly) const;
	
	/** Returns the unique quantities of all obstructions. */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetObstQuantities(const bool ActiveOnly) const;
	
	/** Returns the unique quantities of all slices. */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetSliceQuantities(const bool ActiveOnly) const;
	
	UFUNCTION(BlueprintCallable)
	void GetSlicesMaxMinForQuantity(FString Quantity, float& MinOut, float& MaxOut) const;

	UFUNCTION(BlueprintCallable)
	void GetObstructionsMaxMinForQuantity(FString Quantity, float& MinOut, float& MaxOut) const;
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void NextTimeStep(const FString Type);
	
public:
	/** The loaded slice asset belonging to this slice. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	class USimulationAsset* SimulationAsset;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class USimControllerUserWidget> SimControllerUserWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AObst> ObstClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ASlice> SliceClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ARaymarchVolume> VolumeClass;
	
	/** The class of the raymarch lights that are dimmed over time. */
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ARaymarchLight> RaymarchLightClass;

	/** Controls the maximum radius of Jitter that is applied (works as a factor). Defaults to 0 (no Jitter). */
	UPROPERTY(EditAnywhere)
	float JitterRadius = 0;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int> CurrentTimeSteps;

	/** Controls the time between two updates of textures. Defaults to the rate specified by the input from FDS if set to 0. */
	UPROPERTY(VisibleAnywhere)
	TMap<FString, float> UpdateRates;
	
protected:
	UPROPERTY(BlueprintReadOnly)
	class USimControllerUserWidget* SimControllerUserWidget;
	
	/** Lists of currently inactive obstructions. */
	UPROPERTY(VisibleAnywhere)
	TArray<class AObst*> Obstructions;
	/** Lists of currently inactive slices. */
	UPROPERTY(VisibleAnywhere)
	TArray<class ASlice*> Slices;
	/** Lists of currently inactive volumes. */
	UPROPERTY(VisibleAnywhere)
	TArray<class ARaymarchVolume*> Volumes;

	/** Used to asynchronously load assets at runtime. */
	struct FStreamableManager* StreamableManager;

	/** Handles to manage the update timer. */
	TMap<FString, FTimerHandle> UpdateTimerHandles;

	TMap<FString, FUpdateDataEvent> UpdateDataEvents;

	/** Contains all the arrays for which assets will be loaded. */
	TMap<FString, TArray<TArray<FAssetData>*>> TextureArrays;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int> MaxTimeSteps;

	bool bIsPaused = false;
};
