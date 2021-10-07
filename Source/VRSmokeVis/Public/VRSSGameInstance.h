// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StreamableManager.h"
#include "Engine/ObjectLibrary.h"
#include "VRSSGameInstance.generated.h"

DECLARE_EVENT_OneParam(UVRSSGameInstance, FUpdateVolumeEvent, int)

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
	void ToggleHUDVisibility();
	
protected:
	UFUNCTION()
	void NextTimeStep();
	
public:
	/** The class of the raymarch lights that are dimmed over time. */
	TSubclassOf<class ARaymarchLight> RaymarchLightClass;

	/** Controls the maximum radius of Jitter that is applied (works as a factor). Defaults to 0 (no Jitter). */
	UPROPERTY(EditAnywhere)
	float JitterRadius = 0;

	/** Controls the time between two updates of RaymarchingVolume textures. Defaults to the rate specified by the input from FDS if set to 0. */
	UPROPERTY(EditAnywhere, BlueprintSetter=SetUpdateRate)
	float UpdateRate = 0;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UTimeUserWidget> TimeUserWidgetClass;

	UPROPERTY(BlueprintReadOnly)
	class UTimeUserWidget *TimeUserWidget;
	
protected:
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

	bool bHUDHidden = false;
};