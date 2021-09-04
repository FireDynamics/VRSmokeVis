// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StreamableManager.h"
#include "Engine/ObjectLibrary.h"
#include "VRSSGameInstanceSubsystem.generated.h"

DECLARE_EVENT_OneParam(UVRSSGameInstanceSubsystem, FUpdateVolumeEvent, int)

static TAutoConsoleVariable<float> CVarUpdateRate(
	TEXT("VolumeUpdateRate"),
	-1,
	TEXT("Controls the time between two updates of RaymarchingVolume textures.\n")
	TEXT("Defaults to the rate specified by the input from FDS.")
);

static TAutoConsoleVariable<float> CVarJitterRadius(
	TEXT("JitterRadius"),
	0,
	TEXT("Controls the maximum radius of Jitter that is applied (works as a factor). Defaults to 0 (no Jitter).")
);

UCLASS()
class VRSMOKEVIS_API UVRSSGameInstanceSubsystem : public UGameInstanceSubsystem 
{
	GENERATED_BODY()

public:
	// Sets default values
	UVRSSGameInstanceSubsystem();

	FUpdateVolumeEvent& RegisterTextureLoad(const FString& Directory, TArray<FAssetData>* TextureArray);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	UFUNCTION()
	void NextTimeStep();
	
public:	
	UPROPERTY(EditAnywhere)
	int CurrentTimeStep;

	UPROPERTY(VisibleAnywhere)
	int MaxTimeStep;
	
private:
	/** Used to asynchronously load assets at runtime. **/
	FStreamableManager* StreamableManager;
	
	/** Handle to manage the update timer. **/
	FTimerHandle UpdateTimerHandle;
	
	/** Contains all the arrays for which assets will be loaded. **/
	TArray<TArray<FAssetData>*> VolumeTextureArrays;

	FUpdateVolumeEvent UpdateVolumeEvent;
};