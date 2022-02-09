#include "VRSSGameInstanceSubsystem.h"


UVRSSGameInstanceSubsystem::UVRSSGameInstanceSubsystem()
{
}

void UVRSSGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UVRSSGameInstanceSubsystem::RegisterSimulation(ASimulation* Simulation)
{
	Simulations.Add(Simulation);
}
