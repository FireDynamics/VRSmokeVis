#pragma once

#include "Engine/StreamableManager.h"
#include "Simulation.generated.h"

DECLARE_EVENT_OneParam(UVRSSGameInstance, FUpdateDataEvent, int)

DECLARE_LOG_CATEGORY_EXTERN(LogSimulation, Log, All);


UCLASS()
class VRSMOKEVIS_API ASimulation : public AActor
{
	GENERATED_BODY()

public:
	ASimulation();

	UFUNCTION()
	void CheckObstActivations();
	UFUNCTION()
	void CheckSliceActivations();
	UFUNCTION()
	void CheckVolumeActivations();

	UFUNCTION()
	void InitUpdateRate(const FString Type, float UpdateRateSuggestion, const int MaxNumUpdates);

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

	/** Gets called from GameInstanceSubsystem. */
	UFUNCTION()
	void ChangeObstQuantity(FString& NewQuantity);

	UFUNCTION(BlueprintCallable)
	void GetMaxMins(UPARAM(ref) TMap<FString, float>& Mins, UPARAM(ref) TMap<FString, float>& Maxs) const;

	UFUNCTION(BlueprintCallable)
	void GetMaxMinForQuantity(FString Quantity, float& MinOut, float& MaxOut) const;

	UFUNCTION(BlueprintCallable)
	void UpdateColorMaps(const TMap<FString, float>& Mins, const TMap<FString, float>& Maxs);

	/** Returns all unique quantities present in this simulation. */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetQuantities(const bool ActiveOnly) const;

	UFUNCTION(BlueprintCallable)
	bool AnyObstActive() const;

	/** Returns the active quantities of all slices. */
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

	UFUNCTION()
	bool RegisterTextureLoad(const FString Type, const AActor* Asset, const FString& TextureDirectory,
							 UPARAM(ref) TArray<FAssetData>& TextureArray, const int NumTextures);
	
	UFUNCTION()
	void ActivateObst(AObst* Obst);
	UFUNCTION()
	void DeactivateObst(AObst* Obst);
	UFUNCTION()
	void ActivateSlice(ASlice* Slice);
	UFUNCTION()
	void DeactivateSlice(ASlice* Slice);
	UFUNCTION()
	void ActivateVolume(ARaymarchVolume* Volume);
	UFUNCTION()
	void DeactivateVolume(ARaymarchVolume* Volume);

	UFUNCTION()
	void InitObstructions();
	UFUNCTION()
	void InitSlices();
	UFUNCTION()
	void InitVolumes();

/** Spawn meshes for each obstruction with size and location originating from the fds simulation */
	UFUNCTION(CallInEditor, Category="Simulation")
	void SpawnSimulationGeometry();

public:
	/** The loaded slice asset belonging to this slice. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	class USimulationAsset* SimulationAsset;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AObst> ObstClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ASlice> SliceClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ARaymarchVolume> VolumeClass;
	UPROPERTY(EditAnywhere)
	class UStaticMesh* CubeStaticMesh;
	UPROPERTY(EditAnywhere)
	class UMaterial* CubeDefaultMaterial;
	
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

	UPROPERTY(BlueprintReadOnly)
	class USimControllerUserWidget* SimControllerUserWidget;

protected:
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
	FStreamableManager StreamableManager;

	/** Handles to manage the update timer. */
	TMap<FString, FTimerHandle> UpdateTimerHandles;

	TMap<FString, FUpdateDataEvent> UpdateDataEvents;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int> MaxTimeSteps;

	UPROPERTY()
	bool bIsPaused = false;

	/** Maps actor names (obsts, slices and volumes) to DelegateHandles for the texture update event. */
	TMap<FString, TMap<int, FDelegateHandle>> ObstUpdateDataEventDelegateHandles;
	TMap<FString, FDelegateHandle> SliceUpdateDataEventDelegateHandles;
	TMap<FString, FDelegateHandle> VolumeUpdateDataEventDelegateHandles;
};
