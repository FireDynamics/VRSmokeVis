#pragma once

#include "VRSSGameInstanceSubsystem.generated.h"


UCLASS()
class VRSMOKEVIS_API UVRSSGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UVRSSGameInstanceSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void RegisterSimulation(class ASimulation* Simulation);

protected:
	TArray<class ASimulation*> Simulations;
};
