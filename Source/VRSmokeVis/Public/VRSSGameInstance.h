// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StreamableManager.h"
#include "Engine/ObjectLibrary.h"
#include "Actor/RaymarchLight.h"
#include "VRSSGameInstance.generated.h"

DECLARE_EVENT_OneParam(UVRSSGameInstance, FUpdateVolumeEvent, int)

UCLASS()
class VRSMOKEVIS_API UVRSSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Sets default values
	UVRSSGameInstance();

	virtual void Init() override;
	
	FUpdateVolumeEvent& RegisterTextureLoad(const FString& Directory, TArray<FAssetData>* TextureArray);

	UFUNCTION()
	FORCEINLINE void FastForwardSimulation(const float Amount) { CurrentTimeStep += Amount-1; NextTimeStep(); }
	
	UFUNCTION()
	FORCEINLINE void RewindSimulation(const float Amount) { CurrentTimeStep += Amount-1; NextTimeStep(); }

	UFUNCTION()
	void TogglePauseSimulation();

	UFUNCTION(BlueprintSetter)
	void SetUpdateRate(const float NewUpdateRate);
	
protected:
	UFUNCTION()
	void NextTimeStep();

	bool IsPaused = false;
	
public:	
	UPROPERTY(EditAnywhere)
	int CurrentTimeStep;

	UPROPERTY(VisibleAnywhere)
	int MaxTimeStep;

	/** The class of the raymarch lights that are dimmed over time. */
	TSubclassOf<ARaymarchLight> RaymarchLightClass;

	/** Controls the maximum radius of Jitter that is applied (works as a factor). Defaults to 0 (no Jitter). */
	UPROPERTY(EditAnywhere)
	float JitterRadius = 0;

	/** Controls the time between two updates of RaymarchingVolume textures. Defaults to the rate specified by the input from FDS if set to 0. */
	UPROPERTY(EditAnywhere, BlueprintSetter=SetUpdateRate)
	float UpdateRate = 0;
	
private:
	/** Used to asynchronously load assets at runtime. */
	FStreamableManager* StreamableManager;
	
	/** Handle to manage the update timer. **/
	FTimerHandle UpdateTimerHandle;
	
	/** Contains all the arrays for which assets will be loaded. */
	TArray<TArray<FAssetData>*> VolumeTextureArrays;

	FUpdateVolumeEvent UpdateVolumeEvent;
};