#include "VRSSGameInstanceSubsystem.h"
#include "VRSSConfig.h"
#include "Actor/Simulation.h"
#include "Assets/SimulationAsset.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UserInterfaceUserWidget.h"
#include "UI/VRSSHUD.h"


UVRSSGameInstanceSubsystem::UVRSSGameInstanceSubsystem()
{
	Simulations = TArray<ASimulation*>();
	Config = NewObject<UVRSSConfig>();
}

void UVRSSGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Todo: tmp!
	Config->ColorMapsPath = "/Game";
	Config->ObstCutOffValues.Add("wall_temparature", 50);
	Config->SliceCutOffValues.Add("temparature", 35);
	Config->SaveConfig();
}

void UVRSSGameInstanceSubsystem::RegisterSimulation(ASimulation* Simulation)
{
	TMap<FString, float> Mins = TMap<FString, float>(), Maxs = TMap<FString, float>(), TmpMins = TMap<FString, float>(),
	                     TmpMaxs = TMap<FString, float>();
	// Get maximum and minimum values per quantity (value range) for the new simulation
	Simulation->GetMaxMins(Mins, Maxs);
	// Merge with other simulation ranges
	for (const ASimulation* Sim : Simulations)
	{
		Sim->GetMaxMins(TmpMins, TmpMaxs);
		for (auto Min : TmpMins)
		{
			if (!Mins.Contains(Min.Key))
			{
				Mins.Add(Min.Key, Min.Value);
				Maxs.Add(Min.Key, TmpMaxs[Min.Key]);
			}
			else
			{
				Mins[Min.Key] = FMath::Min(Min.Value, Mins[Min.Key]);
				Maxs[Min.Key] = FMath::Max(TmpMaxs[Min.Key], Maxs[Min.Key]);
			}
		}
	}

	Simulations.Add(Simulation);

	// Update the colormaps in the UI to reflect the new value range
	UVRSSGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>();
	AVRSSHUD* HUD = Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	HUD->InitHUD();
	HUD->UserInterfaceUserWidget->InitColorMaps(GI->Config, Mins, Maxs);
	TScriptDelegate<> ToggleSimControllerDelegate;
	ToggleSimControllerDelegate.BindUFunction(Simulations.Last(), "ToggleControllerUI");
	HUD->UserInterfaceUserWidget->AddSimulationController(ToggleSimControllerDelegate,
	                                                      Simulation->SimulationAsset->GetName());

	// Update the colormaps for all assets in each simulation, to draw the correct colors for each value
	for (ASimulation* Sim : Simulations)
		Sim->UpdateColorMaps(Mins, Maxs);
}

void UVRSSGameInstanceSubsystem::ToggleHUDVisibility() const
{
	// Todo: Test this
	if (const AVRSSHUD* HUD = Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD()); HUD->
		UserInterfaceUserWidget->IsInViewport())
		HUD->UserInterfaceUserWidget->RemoveFromViewport();
	else HUD->UserInterfaceUserWidget->AddToViewport();
	// HUD->UserInterfaceUserWidget->SetVisibility(HUD->UserInterfaceUserWidget->IsVisible()
	// 	                                            ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}

void UVRSSGameInstanceSubsystem::OnActiveAssetsChanged() const
{
	TSet<FString> ActiveQuantities = TSet<FString>();
	for (const ASimulation* Sim : Simulations)
		ActiveQuantities.Append(Sim->GetQuantities(true));

	Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD())->UserInterfaceUserWidget->
		UpdateColorMaps(ActiveQuantities.Array());
}
