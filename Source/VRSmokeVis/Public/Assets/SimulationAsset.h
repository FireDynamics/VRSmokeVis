#pragma once

#include "SimulationInfo.h"
#include "Engine/DataAsset.h"
#include "SimulationAsset.generated.h"


UCLASS()
class VRSMOKEVIS_API USimulationAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Holds the general info about the simulation read from disk
	UPROPERTY(EditAnywhere)
	USimulationInfo *SimInfo;

	UPROPERTY(VisibleAnywhere)
	FString ObstructionsDirectory;
	UPROPERTY(VisibleAnywhere)
	FString SlicesDirectory;
	UPROPERTY(VisibleAnywhere)
	FString VolumesDirectory;

	UPROPERTY(VisibleAnywhere, Transient)
	TArray<FAssetData> Obstructions;
	UPROPERTY(VisibleAnywhere, Transient)
	TArray<FAssetData> Slices;
	UPROPERTY(VisibleAnywhere, Transient)
	TArray<FAssetData> Volumes;
};
