#include "VRSSGameInstanceSubsystem.h"

#include "VRSSConfig.h"
#include "Actor/Simulation.h"
#include "Actor/VRSSMotionController.h"
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

	// Todo: Improve UI so multiple simulation controllers actually fit on the screen
	// Update the colormaps in the UI to reflect the new value range
	UVRSSGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>();
	AVRSSHUD* HUD = Cast<AVRSSHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	HUD->InitHUD();
	HUD->UserInterfaceUserWidget->InitColorMaps(GI->Config, Mins, Maxs);
	HUD->UserInterfaceUserWidget->AddSimulationController(Simulation);

	// Update the colormaps for all assets in each simulation, to draw the correct colors for each value
	for (ASimulation* Sim : Simulations)
		Sim->UpdateColorMaps(Mins, Maxs);
}

void UVRSSGameInstanceSubsystem::ToggleHUDVisibility() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(PC->GetHUD());
	if (HUD->UserInterfaceUserWidget->IsInViewport())
	{
		HUD->UserInterfaceUserWidget->RemoveFromViewport();
		// Todo: Check if this would be better
		// PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
	else
	{
		HUD->UserInterfaceUserWidget->AddToViewport();
		// PC->SetInputMode(FInputModeUIOnly());
		PC->SetShowMouseCursor(true);
	}
}

void UVRSSGameInstanceSubsystem::OnActiveAssetsChanged() const
{
	TSet<FString> ActiveQuantities = TSet<FString>();
	for (const ASimulation* Sim : Simulations)
		ActiveQuantities.Append(Sim->GetQuantities(true));

	const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AHUD* HUD = PC->GetHUD();
	UUserInterfaceUserWidget* UW = Cast<AVRSSHUD>(HUD)->UserInterfaceUserWidget;
	UW->UpdateColorMaps(ActiveQuantities.Array());
}

void UVRSSGameInstanceSubsystem::ChangeObstQuantity(FString& NewQuantity)
{
	Config->SetActiveObstQuantity(NewQuantity);

	for (ASimulation* Sim : Simulations)
	{
		Sim->ChangeObstQuantity(NewQuantity);
	}
}
