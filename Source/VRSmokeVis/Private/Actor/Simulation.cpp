#include "Actor/Simulation.h"

#include "VRSSGameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/VolumeTexture.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetTextLibrary.h"
#include "Components/TimelineComponent.h"
#include "Misc/DefaultValueHelper.h"
#include "UI/UserInterfaceUserWidget.h"
#include "UI/TimeUserWidget.h"
#include "UI/SimControllerUserWidget.h"
#include "UI/VRSSHUD.h"
#include "Actor/Slice.h"
#include "Actor/Obst.h"
#include "Actor/RaymarchLight.h"
#include "Actor/RaymarchVolume.h"
#include "Assets/ObstAsset.h"
#include "Assets/SliceAsset.h"
#include "Assets/SimulationAsset.h"
#include "Assets/VolumeAsset.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/StreamableManager.h"


ASimulation::ASimulation()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	StreamableManager = new FStreamableManager();
}

void ASimulation::BeginPlay()
{
	Super::BeginPlay();

	SimControllerUserWidget = CreateWidget<USimControllerUserWidget>(GetWorld(), SimControllerUserWidgetClass);
	SimControllerUserWidget->InitSimulation(this);

	// Spawn all obstructions, slices and volumes, but hide them for now
	for (FAssetData& ObstAsset : SimulationAsset->Obstructions)
	{
		AObst* NewObst = GetWorld()->SpawnActor<AObst>();
		Obstructions.Add(NewObst);
		NewObst->ObstAsset = Cast<UObstAsset>(StreamableManager->LoadSynchronous(ObstAsset.ToSoftObjectPath()));
		NewObst->UseSimulationTransform();
		NewObst->SetActorHiddenInGame(true);
		NewObst->SetActorEnableCollision(false);
	}
	for (FAssetData& SliceAsset : SimulationAsset->Slices)
	{
		ASlice* NewSlice = GetWorld()->SpawnActor<ASlice>();
		Slices.Add(NewSlice);
		NewSlice->SliceAsset = Cast<USliceAsset>(StreamableManager->LoadSynchronous(SliceAsset.ToSoftObjectPath()));
		NewSlice->UseSimulationTransform();
		NewSlice->SetActorHiddenInGame(true);
		NewSlice->SetActorEnableCollision(false);
	}
	for (FAssetData& VolumeAsset : SimulationAsset->Volumes)
	{
		ARaymarchVolume* NewVolume = GetWorld()->SpawnActor<ARaymarchVolume>();
		Volumes.Add(NewVolume);
		NewVolume->VolumeAsset = Cast<UVolumeAsset>(StreamableManager->LoadSynchronous(VolumeAsset.ToSoftObjectPath()));
		NewVolume->UseSimulationTransform();
		NewVolume->SetActorHiddenInGame(true);
		NewVolume->SetActorEnableCollision(false);
	}
}

void ASimulation::UpdateColorMaps(const TMap<FString, float> Mins, const TMap<FString, float> Maxs)
{
	GetMaxMins(Mins, Maxs);
	for (const AObst* Obst : Obstructions)
		Obst->UpdateColorMapScale(Mins[Obst->ActiveQuantity], Maxs[Obst->ActiveQuantity]);

	for (const ASlice* Slice : Slices)
		Slice->UpdateColorMapScale(Mins[Slice->SliceAsset->SliceInfo.Quantity],
		                           Maxs[Slice->SliceAsset->SliceInfo.Quantity]);
}

void ASimulation::CheckObstActivations(const TArray<bool> ObstsActive)
{
	for (int i = 0; i < ObstsActive.Num(); ++i)
	{
		if (AObst* Obst = Obstructions[i]; Obst->IsHidden() && ObstsActive[i])
		{
			// Show visible components
			Obst->SetActorHiddenInGame(false);
			// Activate collision components
			Obst->SetActorEnableCollision(true);
		}
		else
		{
			// Hides visible components
			Obst->SetActorHiddenInGame(true);
			// Disables collision components
			Obst->SetActorEnableCollision(false);
		}
	}

	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->OnActiveAssetsChanged();
}

void ASimulation::CheckSliceActivations(const TArray<bool> SlicesActive)
{
	for (int i = 0; i < SlicesActive.Num(); ++i)
	{
		if (ASlice* Slice = Slices[i]; Slice->IsHidden() && SlicesActive[i])
		{
			// Show visible components
			Slice->SetActorHiddenInGame(false);
			// Activate collision components
			Slice->SetActorEnableCollision(true);
		}
		else
		{
			// Hides visible components
			Slice->SetActorHiddenInGame(true);
			// Disables collision components
			Slice->SetActorEnableCollision(false);
		}
	}
	
	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->OnActiveAssetsChanged();
}

void ASimulation::CheckVolumeActivations(const TArray<bool> VolumesActive)
{
	for (int i = 0; i < VolumesActive.Num(); ++i)
	{
		if (ARaymarchVolume* Volume = Volumes[i]; Volume->IsHidden() && VolumesActive[i])
		{
			// Show visible components
			Volume->SetActorHiddenInGame(false);
			// Activate collision components
			Volume->SetActorEnableCollision(true);
		}
		else
		{
			// Hides visible components
			Volume->SetActorHiddenInGame(true);
			// Disables collision components
			Volume->SetActorEnableCollision(false);
		}
	}
	
	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->OnActiveAssetsChanged();
}

void ASimulation::ToggleControllerUI()
{
	if (SimControllerUserWidget->IsInViewport()) SimControllerUserWidget->RemoveFromViewport();
	else SimControllerUserWidget->AddToViewport();
}

void ASimulation::InitUpdateRate(const FString Type, const float UpdateRateSuggestion)
{
	if (UpdateRates.Contains(Type)) return;
	UpdateRates.Add(Type, UpdateRateSuggestion);
	CurrentTimeSteps.Add(Type, 0);
	UpdateDataEvents.Add(Type, FUpdateDataEvent());
	UpdateTimerHandles.Add(Type, FTimerHandle());

	// Set timer to set next frame (update next texture) after some time
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, FName("NextTimeStep"), Type);
	// Delegate.BindLambda([this, Type](){NextTimeStep(Type);});
	// Todo: Check first delay
	GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandles[Type], Delegate, UpdateRates[Type], true, 1.f);

	if (UpdateRates.Num() == 1)
	{
		// Set the curve length of all (controlled) lights in the scene to the simulation time
		TArray<AActor*> FoundLights;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), RaymarchLightClass, FoundLights);
		for (AActor* Light : FoundLights)
		{
			FString& RandomType = MaxTimeSteps.begin().Key();

			Cast<ARaymarchLight>(Light)->LightIntensityTimelineComponent->SetTimelineLength(
				MaxTimeSteps[RandomType] * UpdateRates[RandomType]);
		}
	}
}

void ASimulation::SetUpdateRate(const FString Type, const float NewUpdateRate)
{
	UpdateRates[Type] = NewUpdateRate;
	GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandles[Type]);
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, FName("NextTimeStep"), Type);
	// Delegate.BindLambda([this, Type]{NextTimeStep(Type);});
	GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandles[Type], Delegate, UpdateRates[Type], true, 1.f);

	Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD())->UserInterfaceUserWidget->
		TimeUserWidget->SimTimeScale =
		UpdateRates[Type] /
		NewUpdateRate;
}

void ASimulation::FastForwardSimulation(const float Amount)
{
	for (TTuple<FString, int>& CurrentTimeStep : CurrentTimeSteps)
	{
		CurrentTimeStep.Value += Amount - 1;
		NextTimeStep(CurrentTimeStep.Key);
	}

	// Get first data type to update the UI
	TArray<FString> Types;
	CurrentTimeSteps.GetKeys(Types);
	const FString UIType = Types[0];
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = CurrentTimeSteps[UIType] * UpdateRates[UIType];
	for (FString Type : Types)
	{
		HUD->UserInterfaceUserWidget->TimeUserWidget->GetTextBlockValueTimesteps(Type)->SetText(
			FText::AsNumber(CurrentTimeSteps[Type]));
	}
	HUD->UserInterfaceUserWidget->TimeUserWidget->UpdateTimeTextBlocks();
}

void ASimulation::RewindSimulation(const float Amount)
{
	for (TTuple<FString, int>& CurrentTimeStep : CurrentTimeSteps)
	{
		CurrentTimeStep.Value -= FMath::Min<float>(Amount, CurrentTimeStep.Value) + 1;
		NextTimeStep(CurrentTimeStep.Key);
	}

	// Get first data type to update the UI
	TArray<FString> Types;
	CurrentTimeSteps.GetKeys(Types);
	const FString UIType = Types[0];
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = CurrentTimeSteps[UIType] * UpdateRates[UIType];
	for (FString Type : Types)
	{
		HUD->UserInterfaceUserWidget->TimeUserWidget->GetTextBlockValueTimesteps(Type)->SetText(
			FText::AsNumber(CurrentTimeSteps[Type]));
	}
	HUD->UserInterfaceUserWidget->TimeUserWidget->UpdateTimeTextBlocks();
}

void ASimulation::TogglePauseSimulation()
{
	bIsPaused = !bIsPaused;
	UGameplayStatics::SetGamePaused(GetWorld(), bIsPaused);

	// Inform UI
	Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD())->UserInterfaceUserWidget->
		TimeUserWidget->bIsPaused = bIsPaused;
}

TArray<AObst*>& ASimulation::GetAllObstructions()
{
	return Obstructions;
}

TArray<ASlice*>& ASimulation::GetAllSlices()
{
	return Slices;
}

TArray<ARaymarchVolume*>& ASimulation::GetAllVolumes()
{
	return Volumes;
}

TArray<FString> ASimulation::GetQuantities(const bool ActiveOnly) const
{
	TArray<FString> Quantities = TArray<FString>();
	Quantities.Append(GetObstQuantities(ActiveOnly));
	Quantities.Append(GetSliceQuantities(ActiveOnly));
	return Quantities;
}

TArray<FString> ASimulation::GetObstQuantities(const bool ActiveOnly) const
{
	TSet<FString> Quantities = TSet<FString>();
	for (const AObst* Obst : Obstructions)
		if (!ActiveOnly || !Obst->IsHidden())
			Quantities.Add(Obst->ActiveQuantity);
	return Quantities.Array();
}

TArray<FString> ASimulation::GetSliceQuantities(const bool ActiveOnly) const
{
	TSet<FString> Quantities = TSet<FString>();
	for (const ASlice* Slice : Slices)
		if (!ActiveOnly || !Slice->IsHidden())
			Quantities.Add(Slice->SliceAsset->SliceInfo.Quantity);
	return Quantities.Array();
}

void ASimulation::GetSlicesMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
{
	MinOut = TNumericLimits<float>::Max();
	MaxOut = TNumericLimits<float>::Min();
	for (const ASlice* Slice : Slices)
	{
		if (Slice->SliceAsset->SliceInfo.Quantity.Equals(Quantity))
		{
			MinOut = FMath::Min(Slice->SliceAsset->SliceInfo.MinValue, MinOut);
			MaxOut = FMath::Max(Slice->SliceAsset->SliceInfo.MaxValue, MaxOut);
		}
	}
}

void ASimulation::GetObstructionsMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
{
	MinOut = TNumericLimits<float>::Max();
	MaxOut = TNumericLimits<float>::Min();
	for (const AObst* Obst : Obstructions)
	{
		MinOut = FMath::Min(Obst->ObstAsset->ObstInfo.MinValues[Quantity], MinOut);
		MaxOut = FMath::Max(Obst->ObstAsset->ObstInfo.MaxValues[Quantity], MaxOut);
	}
}

void ASimulation::ChangeObstQuantity(AObst* Obst)
{
	float Min, Max;
	const FString Quantity = Obst->ActiveQuantity;
	GetMaxMinForQuantity(Quantity, Min, Max);
	Obst->UpdateColorMapScale(Min, Max);
	
	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->OnActiveAssetsChanged();
}

void ASimulation::GetMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
{
	float SMin, SMax, OMin, OMax;
	GetSlicesMaxMinForQuantity(Quantity, SMin, SMax);
	GetObstructionsMaxMinForQuantity(Quantity, OMin, OMax);
	MinOut = FMath::Min(SMin, OMin);
	MaxOut = FMath::Max(SMax, OMax);
}

void ASimulation::GetMaxMins(TMap<FString, float> Mins, TMap<FString, float> Maxs) const
{
	TArray<FString> Quantities = GetQuantities(false);

	float Min, Max;
	for (FString& Quantity : Quantities)
	{
		GetMaxMinForQuantity(Quantity, Min, Max);
		Mins.Add(Quantity, Min);
		Maxs.Add(Quantity, Max);
	}
}

TOptional<FUpdateDataEvent*> ASimulation::RegisterTextureLoad(const FString Type, const FString& Directory,
                                                              TArray<FAssetData>* TextureArray,
                                                              const int NumTextures)
{
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UTexture::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(Directory);

	TextureArray->Reserve(NumTextures);
	ObjectLibrary->GetAssetDataList(*TextureArray);

	if (TextureArray->Num() == NumTextures) return TOptional<FUpdateDataEvent*>();

	if (!MaxTimeSteps.Contains(Type)) MaxTimeSteps.Add(Type, TextureArray->Num());

	TextureArray->Sort([](const FAssetData& Lhs, const FAssetData& Rhs)
	{
		int LeftNum, RightNum;
		FString Left, Right;
		Lhs.AssetName.ToString().Split(TEXT("_t"), &Left, &Right, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		FDefaultValueHelper::ParseInt(Right, LeftNum);
		Rhs.AssetName.ToString().Split(TEXT("_t"), &Left, &Right, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		FDefaultValueHelper::ParseInt(Right, RightNum);
		return LeftNum < RightNum;
	});

	if (!TextureArrays.Contains(Type)) TextureArrays.Add(Type, TArray<TArray<FAssetData>*>());
	TextureArrays[Type].Add(TextureArray);

	// Load first n (max 10) textures synchronously so they will be available from the very beginning
	const int TexturesToLoad = FMath::Min(TextureArray->Num(), 10);

	// Make sure any Textures could be found
	check(TexturesToLoad != 0)

	for (int i = 0; i < TexturesToLoad; ++i)
	{
		StreamableManager->LoadSynchronous((*TextureArray)[CurrentTimeSteps[Type] + i].ToSoftObjectPath());
	}
	return &UpdateDataEvents[Type];
}

void ASimulation::NextTimeStep(const FString Type)
{
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	CurrentTimeSteps[Type] += 1;
	if (CurrentTimeSteps[Type] >= MaxTimeSteps[Type])
	{
		CurrentTimeSteps[Type] = 0;
		HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = .0f;
	}

	// Unload the VolumeTexture of the second to last time step (not the last one as it might still be referenced
	const int PreviousTextureIndex = (CurrentTimeSteps[Type] + MaxTimeSteps[Type] - 2) % MaxTimeSteps[Type];
	const int NextTextureIndex = (CurrentTimeSteps[Type] + 9) % MaxTimeSteps[Type];
	TArray<FSoftObjectPath> AssetsToLoad;

	// Todo: Check for different time resolutions for different data types (e.g. slices might have more time steps than obstructions)
	for (TArray<FAssetData>* AssetsData : TextureArrays[Type])
	{
		StreamableManager->Unload((*AssetsData)[PreviousTextureIndex].ToSoftObjectPath());
		AssetsToLoad.Add((*AssetsData)[NextTextureIndex].ToSoftObjectPath());
	}
	UpdateDataEvents[Type].Broadcast(CurrentTimeSteps[Type]);

	StreamableManager->RequestAsyncLoad(AssetsToLoad);

	// Update the UI
	HUD->UserInterfaceUserWidget->TimeUserWidget->GetTextBlockValueTimesteps(Type)->SetText(
		FText::AsNumber(CurrentTimeSteps[Type]));
}
