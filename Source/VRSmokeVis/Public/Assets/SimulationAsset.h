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
	FSimulationInfo SimInfo;
	
	TArray<FAssetData> Obstructions;
	TArray<FAssetData> Slices;
	TArray<FAssetData> Volumes;
};
