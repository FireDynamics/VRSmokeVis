﻿#include "Actor/Simulation.h"

#include "VRSSConfig.h"
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
#include "AssetRegistry/AssetRegistryModule.h"
#include "Assets/FdsDataAsset.h"
#include "Assets/ObstAsset.h"
#include "Assets/SliceAsset.h"
#include "Assets/SimulationAsset.h"
#include "Assets/VolumeAsset.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Util/AssetCreationUtilities.h"
#include "Util/ImportUtilities.h"


DEFINE_LOG_CATEGORY(LogSimulation)


ASimulation::ASimulation()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ASimulation::BeginPlay()
{
	Super::BeginPlay();
	
	// Spawn all obstructions, slices and volumes, but hide them for now
	InitObstructions();
	InitSlices();
	InitVolumes();

	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->RegisterSimulation(this);
}

void ASimulation::InitObstructions()
{
	if (!SimulationAsset->AssetDirectories.Contains("Obst")) return;
	
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UObstAsset::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(SimulationAsset->AssetDirectories["Obst"]);
	ObjectLibrary->GetAssetDataList(SimulationAsset->Obstructions);

	if (SimulationAsset->Obstructions.Num() == 0) return;

	// Set some default obst quantity as active
	const UVRSSGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>();
	TArray<FString> ObstQuantities;
	const UObstAsset* RandomObstAsset = Cast<UObstAsset>(
		StreamableManager.LoadSynchronous(SimulationAsset->Obstructions[0].ToSoftObjectPath()));
	Cast<UBoundaryDataInfo>(RandomObstAsset->DataInfo)->ScaleFactors.GetKeys(ObstQuantities);

	if (!ObstQuantities.Contains(GI->Config->GetActiveObstQuantity()))
	{
		GI->Config->SetActiveObstQuantity(ObstQuantities[0]);
	}

	const FString& ActiveObstQuantity = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->Config->
	                                                       GetActiveObstQuantity();

	const FTransform ZeroTransform;
	for (FAssetData& ObstAsset : SimulationAsset->Obstructions)
	{
		AObst* NewObst = GetWorld()->SpawnActorDeferred<AObst>(ObstClass, ZeroTransform, this);
		Obstructions.Add(NewObst);
		NewObst->DataAsset = Cast<UObstAsset>(StreamableManager.LoadSynchronous(ObstAsset.ToSoftObjectPath()));
#if WITH_EDITOR
		NewObst->SetActorLabel(NewObst->DataAsset->DataInfo->ImportName);
#endif
		NewObst->SetActorHiddenInGame(true);
		NewObst->SetActorEnableCollision(false);
		NewObst->FinishSpawning(ZeroTransform);
		NewObst->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		NewObst->UseSimulationTransform();
		NewObst->SetActiveQuantity(ActiveObstQuantity);
	}
}

void ASimulation::InitSlices()
{
	if (!SimulationAsset->AssetDirectories.Contains("Slice")) return;
	
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(USliceAsset::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(SimulationAsset->AssetDirectories["Slice"]);
	ObjectLibrary->GetAssetDataList(SimulationAsset->Slices);

	const FTransform ZeroTransform;
	for (FAssetData& SliceAsset : SimulationAsset->Slices)
	{
		ASlice* NewSlice = GetWorld()->SpawnActorDeferred<ASlice>(SliceClass, ZeroTransform, this);
		Slices.Add(NewSlice);
		NewSlice->DataAsset = Cast<USliceAsset>(StreamableManager.LoadSynchronous(SliceAsset.ToSoftObjectPath()));
#if WITH_EDITOR
		NewSlice->SetActorLabel(NewSlice->DataAsset->DataInfo->ImportName);
#endif
		NewSlice->SetActorHiddenInGame(true);
		NewSlice->SetActorEnableCollision(false);
		NewSlice->FinishSpawning(ZeroTransform);
		NewSlice->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		NewSlice->UseSimulationTransform();
	}
}

void ASimulation::InitVolumes()
{
	if (!SimulationAsset->AssetDirectories.Contains("Volume")) return;
	
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UVolumeAsset::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(SimulationAsset->AssetDirectories["Volume"]);
	ObjectLibrary->GetAssetDataList(SimulationAsset->Volumes);

	const FTransform ZeroTransform;
	for (FAssetData& VolumeAsset : SimulationAsset->Volumes)
	{
		ARaymarchVolume* NewVolume = GetWorld()->SpawnActorDeferred<ARaymarchVolume>(VolumeClass, ZeroTransform, this);
		Volumes.Add(NewVolume);
		NewVolume->DataAsset = Cast<UVolumeAsset>(StreamableManager.LoadSynchronous(VolumeAsset.ToSoftObjectPath()));
#if WITH_EDITOR
		NewVolume->SetActorLabel(NewVolume->DataAsset->DataInfo->ImportName);
#endif
		NewVolume->SetActorHiddenInGame(true);
		NewVolume->SetActorEnableCollision(false);
		NewVolume->FinishSpawning(ZeroTransform);
		NewVolume->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		NewVolume->UseSimulationTransform();
	}
}

void ASimulation::SpawnSimulationGeometry()
{
	if (!SimulationAsset->AssetDirectories.Contains("Obst")) return;
	
	TArray<FAssetData> Geometries;
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UDataAsset::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(SimulationAsset->AssetDirectories["Obst"]);
	ObjectLibrary->GetAssetDataList(Geometries);

	for (FAssetData& ObstAssetData : Geometries)
	{
		UObstAsset* ObstAsset = Cast<UObstAsset>(StreamableManager.LoadSynchronous(ObstAssetData.ToSoftObjectPath()));

		const FVector ObstScale = FVector(ObstAsset->BoundingBox[1] - ObstAsset->BoundingBox[0],
										  ObstAsset->BoundingBox[3] - ObstAsset->BoundingBox[2],
										  ObstAsset->BoundingBox[5] - ObstAsset->BoundingBox[4]);
		// The pivot point of the mesh is not in the center, but on the lower side instead. We therefore have to adjust
		// the z-coordinate to re-center the actor
		const FVector Adjustment = FVector(ObstScale.X, ObstScale.Y, 0);
		const FVector ObstLocation = FVector(ObstAsset->BoundingBox[0], ObstAsset->BoundingBox[2],
											 ObstAsset->BoundingBox[4]) + Adjustment / 2;
		FTransform Transform = FTransform(ObstLocation * 100);
		
		AStaticMeshActor* ObstCuboid = GetWorld()->SpawnActorDeferred<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Transform);
#if WITH_EDITOR
		ObstCuboid->SetActorLabel(ObstAsset->DataInfo->ImportName + "-Geometry");
#endif
		ObstCuboid->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		ObstCuboid->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		ObstCuboid->GetStaticMeshComponent()->SetStaticMesh(CubeStaticMesh);
		for (int i = -3; i <= 3; ++i)
			if (i != 0)
				ObstCuboid->GetStaticMeshComponent()->SetMaterialByName(*FString::FromInt(i), CubeDefaultMaterial);

		ObstCuboid->SetActorScale3D(FVector{1, 1, 1});
		ObstCuboid->GetStaticMeshComponent()->SetRelativeLocation(FVector{0, 0, 0});

		ObstCuboid->GetStaticMeshComponent()->SetRelativeScale3D(ObstScale);
		ObstCuboid->SetActorRelativeLocation(ObstLocation * 100);
		ObstCuboid->SetMobility(EComponentMobility::Static);
		ObstCuboid->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
		
		ObstCuboid->FinishSpawning(Transform);
	}
}

void ASimulation::UpdateColorMaps(const TMap<FString, float>& Mins, const TMap<FString, float>& Maxs)
{
	const FString& ActiveObstQuantity = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->Config->
	                                                       GetActiveObstQuantity();

	for (const AObst* Obst : Obstructions)
		Obst->UpdateColorMapScale(Mins[ActiveObstQuantity], Maxs[ActiveObstQuantity]);

	for (const ASlice* Slice : Slices)
		Slice->UpdateColorMapScale(Mins[Cast<USliceDataInfo>(Slice->DataAsset->DataInfo)->Quantity],
		                           Maxs[Cast<USliceDataInfo>(Slice->DataAsset->DataInfo)->Quantity]);
}

void ASimulation::CheckObstActivations()
{
	TArray<UWidget*> CheckBoxes = SimControllerUserWidget->ObstsScrollBox->GetAllChildren();
	TArray<UHorizontalBox*> FilteredCheckboxes = TArray<UHorizontalBox*>(); 
	for (int i = 0; i < CheckBoxes.Num(); ++i)
	{
		if (UHorizontalBox* CheckBox = Cast<UHorizontalBox>(CheckBoxes[i]); CheckBox)
			FilteredCheckboxes.Add(CheckBox);
	}
		
	for (int i = 0; i < FilteredCheckboxes.Num(); ++i)
	{
		const bool CheckBoxChecked = Cast<UCheckBox>(FilteredCheckboxes[i]->GetChildAt(1))->IsChecked();
		if (AObst* Obst = Obstructions[i]; Obst->IsHidden() && CheckBoxChecked)
		{
			ActivateObst(Obst);
		}
		else if (!Obst->IsHidden() && !CheckBoxChecked)
		{
			DeactivateObst(Obst);
		}
	}

	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->OnActiveAssetsChanged();
}

void ASimulation::ActivateObst(AObst* Obst)
{
	const FString& ActiveObstQuantity = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->Config->
	                                                       GetActiveObstQuantity();

	TArray<int> Orientations;
	UBoundaryDataInfo *ObstDataInfo = Cast<UBoundaryDataInfo>(Obst->DataAsset->DataInfo);
	ObstDataInfo->Dimensions.GetKeys(Orientations);
	for (const int Orientation : Orientations)
	{
		ObstUpdateDataEventDelegateHandles.FindOrAdd(Obst->GetName(), TMap<int, FDelegateHandle>());
		// Remove the existing update event delegate before assigning a new one
		if (ObstUpdateDataEventDelegateHandles[Obst->GetName()].Contains(Orientation))
			UpdateDataEvents["Obst"].Remove(ObstUpdateDataEventDelegateHandles[Obst->GetName()][Orientation]);

		// Registering the automatic async texture loading each timestep
		if (!RegisterTextureLoad("Obst", Obst,
		                         ObstDataInfo->TextureDirs[ActiveObstQuantity].FaceDirs[Orientation],
		                         Cast<UObstAsset>(Obst->DataAsset)->ObstTextures[ActiveObstQuantity].ForOrientation[Orientation].Textures,
		                         ObstDataInfo->Dimensions[Orientation].W))
			return;
		FDelegateHandle Handle = UpdateDataEvents["Obst"].AddUObject(Obst, &AObst::UpdateTexture, Orientation);
		if (ObstUpdateDataEventDelegateHandles[Obst->GetName()].Contains(Orientation))
			ObstUpdateDataEventDelegateHandles[Obst->GetName()][Orientation] = Handle;
		else
			ObstUpdateDataEventDelegateHandles[Obst->GetName()].Add(Orientation, Handle);
	}

	for (const int Orientation : Orientations)
	{
		// Initialize resources for timestep t=-1 and t=0 (for time interpolation)
		Obst->UpdateTexture(CurrentTimeSteps["Obst"] - 1, Orientation);
		Obst->UpdateTexture(CurrentTimeSteps["Obst"], Orientation);
	}

	// Show visible components
	Obst->SetActorHiddenInGame(false);
	// Activate collision components
	Obst->SetActorEnableCollision(true);
}

void ASimulation::DeactivateObst(AObst* Obst)
{
	// Remove the existing update event delegates
	TArray<int> Orientations;
	Cast<UBoundaryDataInfo>(Obst->DataAsset->DataInfo)->Dimensions.GetKeys(Orientations);
	for (const int Orientation : Orientations)
		UpdateDataEvents["Obst"].Remove(ObstUpdateDataEventDelegateHandles[Obst->GetName()][Orientation]);

	// Hides visible components
	Obst->SetActorHiddenInGame(true);
	// Disables collision components
	Obst->SetActorEnableCollision(false);
}

void ASimulation::CheckSliceActivations()
{
	TArray<UWidget*> CheckBoxes = SimControllerUserWidget->SlicesScrollBox->GetAllChildren();
	TArray<UHorizontalBox*> FilteredCheckboxes = TArray<UHorizontalBox*>(); 
	for (int i = 0; i < CheckBoxes.Num(); ++i)
	{
		if (UHorizontalBox* CheckBox = Cast<UHorizontalBox>(CheckBoxes[i]); CheckBox)
			FilteredCheckboxes.Add(CheckBox);
	}
		
	for (int i = 0; i < FilteredCheckboxes.Num(); ++i)
	{
		const bool CheckBoxChecked = Cast<UCheckBox>(FilteredCheckboxes[i]->GetChildAt(1))->IsChecked();
		if (ASlice* Slice = Slices[i]; Slice->IsHidden() && CheckBoxChecked)
		{
			ActivateSlice(Slice);
		}
		else if (!Slice->IsHidden() && !CheckBoxChecked)
		{
			DeactivateSlice(Slice);
		}
	}

	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->OnActiveAssetsChanged();
}

void ASimulation::ActivateSlice(ASlice* Slice)
{
	// Remove the existing update event delegate before assigning a new one
	if (SliceUpdateDataEventDelegateHandles.Contains(Slice->GetName()))
		UpdateDataEvents["Slice"].Remove(SliceUpdateDataEventDelegateHandles[Slice->GetName()]);
	// Registering the automatic async texture loading each timestep
	if (!RegisterTextureLoad("Slice", Slice, Cast<USliceDataInfo>(Slice->DataAsset->DataInfo)->TextureDir, Cast<USliceAsset>(Slice->DataAsset)->SliceTextures,
	                         Cast<USliceDataInfo>(Slice->DataAsset->DataInfo)->Dimensions.W))
		return;
	FDelegateHandle Handle = UpdateDataEvents["Slice"].AddUObject(Slice, &ASlice::UpdateTexture);
	if (SliceUpdateDataEventDelegateHandles.Contains(Slice->GetName()))
		SliceUpdateDataEventDelegateHandles[Slice->GetName()] = Handle;
	else
		SliceUpdateDataEventDelegateHandles.Add(Slice->GetName(), Handle);

	// Initialize resources for first timesteps (for time interpolation)
	Slice->UpdateTexture(CurrentTimeSteps["Slice"] - 1);
	Slice->UpdateTexture(CurrentTimeSteps["Slice"]);

	// Show visible components
	Slice->SetActorHiddenInGame(false);
	// Activate collision components
	Slice->SetActorEnableCollision(true);
}

void ASimulation::DeactivateSlice(ASlice* Slice)
{
	// Remove the existing update event delegate 
	UpdateDataEvents["Slice"].Remove(SliceUpdateDataEventDelegateHandles[Slice->GetName()]);

	// Hides visible components
	Slice->SetActorHiddenInGame(true);
	// Disables collision components
	Slice->SetActorEnableCollision(false);
}

void ASimulation::CheckVolumeActivations()
{
	TArray<UWidget*> CheckBoxes = SimControllerUserWidget->VolumesScrollBox->GetAllChildren();

	TArray<UHorizontalBox*> FilteredCheckboxes = TArray<UHorizontalBox*>(); 
	for (int i = 0; i < CheckBoxes.Num(); ++i)
	{
		if (UHorizontalBox* CheckBox = Cast<UHorizontalBox>(CheckBoxes[i]); CheckBox)
			FilteredCheckboxes.Add(CheckBox);
	}
		
	for (int i = 0; i < FilteredCheckboxes.Num(); ++i)
	{
		const bool CheckBoxChecked = Cast<UCheckBox>(FilteredCheckboxes[i]->GetChildAt(1))->IsChecked();
		if (ARaymarchVolume* Volume = Volumes[i]; Volume->IsHidden() && CheckBoxChecked)
		{
			ActivateVolume(Volume);
		}
		else if (!Volume->IsHidden() && !CheckBoxChecked)
		{
			DeactivateVolume(Volume);
		}
	}

	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->OnActiveAssetsChanged();
}

void ASimulation::ActivateVolume(ARaymarchVolume* Volume)
{
	// Remove the existing update event delegate before assigning a new one
	if (VolumeUpdateDataEventDelegateHandles.Contains(Volume->GetName()))
		UpdateDataEvents["Volume"].Remove(VolumeUpdateDataEventDelegateHandles[Volume->GetName()]);
	// Registering the automatic async texture loading each timestep
	if (!RegisterTextureLoad("Volume", Volume, Cast<UVolumeDataInfo>(Volume->DataAsset->DataInfo)->TextureDir,
	                         Cast<UVolumeAsset>(Volume->DataAsset)->VolumeTextures, Cast<UVolumeDataInfo>(Volume->DataAsset->DataInfo)->Dimensions.W))
		return;

	FDelegateHandle Handle = UpdateDataEvents["Volume"].AddUObject(Volume, &ARaymarchVolume::UpdateVolume);
	if (VolumeUpdateDataEventDelegateHandles.Contains(Volume->GetName()))
		VolumeUpdateDataEventDelegateHandles[Volume->GetName()] = Handle;
	else
		VolumeUpdateDataEventDelegateHandles.Add(Volume->GetName(), Handle);

	// Initialize resources for timestep t=-1 and t=0 (for time interpolation)
	Volume->UpdateVolume(CurrentTimeSteps["Volume"] - 1);
	Volume->UpdateVolume(CurrentTimeSteps["Volume"]);

	// Show visible components
	Volume->SetActorHiddenInGame(false);
	// Activate collision components
	Volume->SetActorEnableCollision(true);
}

void ASimulation::DeactivateVolume(ARaymarchVolume* Volume)
{
	// Remove the existing update event delegate
	UpdateDataEvents["Volume"].Remove(VolumeUpdateDataEventDelegateHandles[Volume->GetName()]);

	// Hides visible components
	Volume->SetActorHiddenInGame(true);
	// Disables collision components
	Volume->SetActorEnableCollision(false);
}

void ASimulation::InitUpdateRate(const FString Type, const float UpdateRateSuggestion, const int MaxNumUpdates)
{
	if (UpdateRates.Contains(Type)) return;
	UpdateRates.Add(Type, UpdateRateSuggestion);
	CurrentTimeSteps.Add(Type, 0);
	MaxTimeSteps.Add(Type, MaxNumUpdates);
	UpdateDataEvents.Add(Type, FUpdateDataEvent());
	UpdateTimerHandles.Add(Type, FTimerHandle());

	// Set timer to set next frame (update next texture) after some time
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, FName("NextTimeStep"), Type);

	GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandles[Type], Delegate, UpdateRates[Type], true, UpdateRates[Type]);
	
	// For the first update rate that gets initialized, set the length of the timeline for raymarch lights
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

	for (AObst* Obst : Obstructions)
	{
		Obst->SetActorTickEnabled(!bIsPaused);
	}
	for (ASlice* Slice : Slices)
	{
		Slice->SetActorTickEnabled(!bIsPaused);
	}
	for (ARaymarchVolume* Volume : Volumes)
	{
		Volume->SetActorTickEnabled(!bIsPaused);
	}
	for (const auto& UpdateTimerHandle : UpdateTimerHandles)
	{
		if (bIsPaused)
			GetWorld()->GetTimerManager().PauseTimer(UpdateTimerHandle.Value);
		else
			GetWorld()->GetTimerManager().UnPauseTimer(UpdateTimerHandle.Value);
	}

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
	if (AnyObstActive() || !ActiveOnly)
		Quantities.Add(GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->Config->GetActiveObstQuantity());
	Quantities.Append(GetSliceQuantities(ActiveOnly));
	return Quantities;
}

bool ASimulation::AnyObstActive() const
{
	for (const AObst* Obst : Obstructions)
	{
		if (!Obst->IsHidden()) return true;
	}
	return false;
}

TArray<FString> ASimulation::GetSliceQuantities(const bool ActiveOnly) const
{
	TSet<FString> Quantities = TSet<FString>();
	for (const ASlice* Slice : Slices)
		if (!ActiveOnly || !Slice->IsHidden())
			Quantities.Add(Cast<USliceDataInfo>(Slice->DataAsset->DataInfo)->Quantity);
	return Quantities.Array();
}

void ASimulation::GetSlicesMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
{
	MinOut = TNumericLimits<float>::Max();
	MaxOut = TNumericLimits<float>::Min();
	for (const ASlice* Slice : Slices)
	{
		const USliceDataInfo* DataInfo = Cast<USliceDataInfo>(Slice->DataAsset->DataInfo);
		if (DataInfo->Quantity.Equals(Quantity))
		{
			MinOut = FMath::Min(DataInfo->MinValue, MinOut);
			MaxOut = FMath::Max(DataInfo->MaxValue, MaxOut);
		}
	}
}

void ASimulation::GetObstructionsMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
{
	MinOut = TNumericLimits<float>::Max();
	MaxOut = TNumericLimits<float>::Min();
	for (const AObst* Obst : Obstructions)
	{
		const UBoundaryDataInfo* DataInfo = Cast<UBoundaryDataInfo>(Obst->DataAsset->DataInfo);
		if (const float* MinVal = DataInfo->MinValues.Find(Quantity)) MinOut = FMath::Min(*MinVal, MinOut);
		if (const float* MaxVal = DataInfo->MaxValues.Find(Quantity)) MaxOut = FMath::Max(*MaxVal, MaxOut);
	}
}

void ASimulation::ChangeObstQuantity(const FString& NewQuantity)
{
	float Min, Max;
	GetMaxMinForQuantity(NewQuantity, Min, Max);
	for (AObst* Obst : Obstructions)
	{
		Obst->UpdateColorMapScale(Min, Max);

		TArray<int> Orientations;
		Cast<UBoundaryDataInfo>(Obst->DataAsset->DataInfo)->Dimensions.GetKeys(Orientations);
		for (const int Orientation : Orientations)
		{
			// Initialize resources for timestep t=-1 and t=0 (for time interpolation)
			Obst->UpdateTexture(CurrentTimeSteps["Obst"] - 1, Orientation);
			Obst->UpdateTexture(CurrentTimeSteps["Obst"], Orientation);
		}
	}

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

void ASimulation::GetMaxMins(UPARAM(ref) TMap<FString, float>& Mins, UPARAM(ref) TMap<FString, float>& Maxs) const
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

bool ASimulation::RegisterTextureLoad(const FString Type, const AActor* Asset, const FString& TextureDirectory,
                                      UPARAM(ref) TArray<FAssetData>& TextureArray, const int NumTextures)
{
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UTexture::StaticClass(), false, false);
	ObjectLibrary->AddToRoot();
	TextureArray.Reserve(NumTextures);
	// Check if there are any assets in the directory
	ObjectLibrary->LoadAssetDataFromPath(TextureDirectory);
	ObjectLibrary->GetAssetDataList(TextureArray);

	// Check if the expected amount of data (textures) could be found in the given directory
	if (TextureArray.Num() != NumTextures)
	{
		FString OriginalDataDirectory, SimName;
		FImportUtils::SplitPath(SimulationAsset->SimInfo->SmokeViewOriginalFilePath, OriginalDataDirectory, SimName);
		// If not, load the data now
		UDataInfo* DataInfo = Cast<AFdsActor>(Asset)->DataAsset->DataInfo;
		
		FAssetCreationUtils::LoadTextures(DataInfo, Type);

		// Todo: Load the data in the background and add a loading queue in UI

		TArray<FString> PathsToScan = TArray<FString>();
		PathsToScan.Add(TextureDirectory);
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
			"AssetRegistry");
		AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan, true);

		ObjectLibrary->ClearLoaded();
		ObjectLibrary->LoadAssetDataFromPath(TextureDirectory);
		ObjectLibrary->GetAssetDataList(TextureArray);
	}

	TextureArray.Sort([](const FAssetData& Lhs, const FAssetData& Rhs)
	{
		int LeftNum, RightNum;
		FString Left, Right;
		Lhs.AssetName.ToString().Split(TEXT("_t"), &Left, &Right, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		FDefaultValueHelper::ParseInt(Right, LeftNum);
		Rhs.AssetName.ToString().Split(TEXT("_t"), &Left, &Right, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		FDefaultValueHelper::ParseInt(Right, RightNum);
		return LeftNum < RightNum;
	});

	// Load first n (max 10) textures synchronously so they will be available from the very beginning
	const int TexturesToLoad = FMath::Min(TextureArray.Num(), 10);

	// Make sure any Textures could be found
	if (TexturesToLoad == 0)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
			                                 TEXT("Please restart to finish the data loading process!"));
		return false;
	}

	for (int i = 0; i < TexturesToLoad; ++i)
	{
		StreamableManager.LoadSynchronous(TextureArray[(CurrentTimeSteps[Type] + i) % MaxTimeSteps[Type]].ToSoftObjectPath());
	}
	return true;
}

void ASimulation::NextTimeStep(const FString Type)
{
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	CurrentTimeSteps[Type] += 1;
	if (CurrentTimeSteps[Type] >= MaxTimeSteps[Type])
	{
		HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = .0f;

		// If the last time step is reached for one type, set time for ALL types back to 0. Also reset all timers to
		// synchronize them again. This is necessary because the different types end at a different absolute time,
		// because the output rate is fixed and might not match with the simulation time. The time remaining after the
		// last output step will therefore vary for each type. We just end the simulation as soon as one type ends.
		// This behavior could be changed and is simply a design decision.
		TArray<FString> Types;
		CurrentTimeSteps.GetKeys(Types);
		for (FString& ItType : Types)
		{			
			FTimerDelegate Delegate;
			Delegate.BindUFunction(this, FName("NextTimeStep"), ItType);
			GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandles[ItType]);
			if (Type == ItType)
			{
				GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandles[ItType], Delegate, UpdateRates[ItType], true, UpdateRates[ItType]);
				
				CurrentTimeSteps[ItType] = 0;
			} else
			{
				GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandles[ItType], Delegate, UpdateRates[ItType], true, 0);

				// Step through all timesteps that will be skipped by setting time back to 0
				for (int t = CurrentTimeSteps[ItType]; t < MaxTimeSteps[ItType]; ++t){
					LoadUnloadTimeStep(t, ItType);
				}
				
				CurrentTimeSteps[ItType] = -1;
			}
		}
	}
	LoadUnloadTimeStep(CurrentTimeSteps[Type], Type);
}

void ASimulation::LoadUnloadTimeStep(const int TimeStep, const FString& Type){
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	// Unload the VolumeTexture of the second to last time step (not the last one as it might still be referenced)
	const int PreviousTextureIndex = (TimeStep + MaxTimeSteps[Type] - 2) % MaxTimeSteps[Type];
	const int NextTextureIndex = (TimeStep + 9) % MaxTimeSteps[Type];
	TArray<FSoftObjectPath> AssetsToLoad;

	TArray<FAssetData> CurrentTextureArray;
	if (Type.Equals("Obst"))
	{
		const FString& ActiveObstQuantity = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->Config->
															   GetActiveObstQuantity();

		for (const AObst* Obst : Obstructions)
		{
			if (!Obst->IsHidden())
			{
				TArray<int> Orientations;
				Cast<UBoundaryDataInfo>(Obst->DataAsset->DataInfo)->Dimensions.GetKeys(Orientations);
				for (const int Orientation : Orientations)
				{
					CurrentTextureArray = Cast<UObstAsset>(Obst->DataAsset)->ObstTextures[ActiveObstQuantity].ForOrientation[Orientation].Textures;
					StreamableManager.Unload(CurrentTextureArray[PreviousTextureIndex].ToSoftObjectPath());
					AssetsToLoad.Add(CurrentTextureArray[NextTextureIndex].ToSoftObjectPath());
				}
			}
		}
	}
	else if (Type.Equals("Slice"))
	{
		for (const ASlice* Slice : Slices)
		{
			if (!Slice->IsHidden())
			{
				CurrentTextureArray = Cast<USliceAsset>(Slice->DataAsset)->SliceTextures;
				StreamableManager.Unload(CurrentTextureArray[PreviousTextureIndex].ToSoftObjectPath());
				AssetsToLoad.Add(CurrentTextureArray[NextTextureIndex].ToSoftObjectPath());
			}
		}
	}
	else if (Type.Equals("Volume"))
	{
		for (const ARaymarchVolume* Volume : Volumes)
		{
			if (!Volume->IsHidden())
			{
				CurrentTextureArray = Cast<UVolumeAsset>(Volume->DataAsset)->VolumeTextures;
				StreamableManager.Unload(CurrentTextureArray[PreviousTextureIndex].ToSoftObjectPath());
				AssetsToLoad.Add(CurrentTextureArray[NextTextureIndex].ToSoftObjectPath());
			}
		}
	}

	if (AssetsToLoad.Num() > 0) StreamableManager.RequestAsyncLoad(AssetsToLoad);

	UpdateDataEvents[Type].Broadcast(CurrentTimeSteps[Type]);

	// Update the UI
	HUD->UserInterfaceUserWidget->TimeUserWidget->GetTextBlockValueTimesteps(Type)->SetText(
		FText::AsNumber(CurrentTimeSteps[Type]));
	HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = CurrentTimeSteps[Type] * UpdateRates[Type];
}
