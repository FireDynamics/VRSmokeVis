#include "VRSSGameInstance.h"
#include "Engine/VolumeTexture.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetTextLibrary.h"
#include "Misc/DefaultValueHelper.h"
#include "Actor/RaymarchLight.h"
#include "UI/UserInterfaceUserWidget.h"
#include "UI/TimeUserWidget.h"
#include "UI/VRSSHUD.h"
#include "Components/TimelineComponent.h"
#include "Actor/Slice.h"
#include "Actor/Obst.h"

UVRSSGameInstance::UVRSSGameInstance(): UGameInstance()
{
	StreamableManager = new FStreamableManager();
}

void UVRSSGameInstance::Init()
{
	Super::Init();
}

void UVRSSGameInstance::InitUpdateRate(const FString Type, const float UpdateRateSuggestion)
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
	GetTimerManager().SetTimer(UpdateTimerHandles[Type], Delegate, UpdateRates[Type], true, 1.f);

	if (UpdateRates.Num() == 1)
	{
		// Set the curve length of all (controlled) lights in the scene to the simulation time
		TArray<AActor*> FoundLights;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), RaymarchLightClass, FoundLights);
		for (AActor* Light : FoundLights)
		{
			FString& RandomType = MaxTimeSteps.begin()->Key;

			Cast<ARaymarchLight>(Light)->LightIntensityTimelineComponent->SetTimelineLength(
				MaxTimeSteps[RandomType] * UpdateRates[RandomType]);
		}
	}
}

void UVRSSGameInstance::SetUpdateRate(const FString Type, const float NewUpdateRate)
{
	UpdateRates[Type] = NewUpdateRate;
	GetTimerManager().ClearTimer(UpdateTimerHandles[Type]);
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, FName("NextTimeStep"), Type);
	// Delegate.BindLambda([this, Type]{NextTimeStep(Type);});
	GetTimerManager().SetTimer(UpdateTimerHandles[Type], Delegate, UpdateRates[Type], true, 1.f);

	Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD())->UserInterfaceUserWidget->TimeUserWidget->SimTimeScale =
		UpdateRates[Type] /
		NewUpdateRate;
}

void UVRSSGameInstance::FastForwardSimulation(const float Amount)
{
	for (TTuple<FString, int>& CurrentTimeStep : CurrentTimeSteps)
	{
		CurrentTimeStep.Value += Amount - 1;
		NextTimeStep(CurrentTimeStep.Key);
	}

	// Get first data type to update the UI
	TArray<FString> Types;
	CurrentTimeSteps.GetKeys(Types);
	FString UIType = Types[0];
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD());
	HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = CurrentTimeSteps[UIType] * UpdateRates[UIType];
	for (FString Type : Types)
	{
		HUD->UserInterfaceUserWidget->TimeUserWidget->GetTextBlockValueTimesteps(Type)->SetText(
			FText::AsNumber(CurrentTimeSteps[Type]));
	}
	HUD->UserInterfaceUserWidget->TimeUserWidget->UpdateTimeTextBlocks();
}

void UVRSSGameInstance::RewindSimulation(const float Amount)
{
	for (TTuple<FString, int>& CurrentTimeStep : CurrentTimeSteps)
	{
		CurrentTimeStep.Value -= FMath::Min<float>(Amount, CurrentTimeStep.Value) + 1;
		NextTimeStep(CurrentTimeStep.Key);
	}

	// Get first data type to update the UI
	TArray<FString> Types;
	CurrentTimeSteps.GetKeys(Types);
	FString UIType = Types[0];
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD());
	HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = CurrentTimeSteps[UIType] * UpdateRates[UIType];
	for (FString Type : Types)
	{
		HUD->UserInterfaceUserWidget->TimeUserWidget->GetTextBlockValueTimesteps(Type)->SetText(
			FText::AsNumber(CurrentTimeSteps[Type]));
	}
	HUD->UserInterfaceUserWidget->TimeUserWidget->UpdateTimeTextBlocks();
}

void UVRSSGameInstance::TogglePauseSimulation()
{
	bIsPaused = !bIsPaused;
	UGameplayStatics::SetGamePaused(GetWorld(), bIsPaused);

	// Inform UI
	Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD())->UserInterfaceUserWidget->TimeUserWidget->bIsPaused =
		bIsPaused;
}

void UVRSSGameInstance::ToggleHUDVisibility() const
{
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD());
	HUD->UserInterfaceUserWidget->SetVisibility(HUD->UserInterfaceUserWidget->IsVisible()
		                                            ? ESlateVisibility::Hidden
		                                            : ESlateVisibility::Visible);
}

void UVRSSGameInstance::NextTimeStep(const FString Type)
{
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD());
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

TArray<FString> UVRSSGameInstance::GetActiveSliceQuantities() const
{
	TArray<FString> Quantities = TArray<FString>();
	for (const ASlice* Slice : ActiveSlices)
	{
		if (!Quantities.Contains(Slice->SliceAsset->SliceInfo.Quantity))
			Quantities.Add(Slice->SliceAsset->SliceInfo.Quantity);
	}
	return Quantities;
}

void UVRSSGameInstance::GetActiveSlicesMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
{
	MinOut = TNumericLimits<float>::Max();
	MaxOut = TNumericLimits<float>::Min();
	for (const ASlice* Slice : ActiveSlices)
	{
		if (Slice->SliceAsset->SliceInfo.Quantity.Equals(Quantity))
		{
			MinOut = FMath::Min(Slice->SliceAsset->SliceInfo.MinValue, MinOut);
			MaxOut = FMath::Max(Slice->SliceAsset->SliceInfo.MaxValue, MaxOut);
		}
	}
}

void UVRSSGameInstance::AddSlice(ASlice* Slice)
{
	ActiveSlices.Add(Slice);

	// Bind to Event that gets called whenever a new slice is shown (this might affect our ColorMap scale)
	ColorMapUpdateEvent.AddUObject(Slice, &ASlice::UpdateColorMapScale);

	float Min, Max;
	const FString Quantity = Slice->SliceAsset->SliceInfo.Quantity;
	GetActiveMaxMinForQuantity(Quantity, Min, Max);
	ColorMapUpdateEvent.Broadcast(Quantity, Min, Max);
}

void UVRSSGameInstance::RemoveSlice(ASlice* Slice)
{
	const TArray<FString> QuantitiesBefore = GetActiveSliceQuantities();
	ActiveSlices.Remove(Slice);
	const TArray<FString> QuantitiesAfter = GetActiveSliceQuantities();

	float Min, Max;
	const FString Quantity = Slice->SliceAsset->SliceInfo.Quantity;
	GetActiveMaxMinForQuantity(Quantity, Min, Max);
	ColorMapUpdateEvent.Broadcast(Quantity, Min, Max);

	if (QuantitiesBefore.Num() != QuantitiesAfter.Num())
	{
		Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD())->UserInterfaceUserWidget->UpdateColorMaps();
	}

	if (ActiveSlices.Num() == 0)
	{
		TextureArrays["Slice"].Empty();
		GetTimerManager().ClearTimer(UpdateTimerHandles["Slice"]);
	}
}

TArray<FString> UVRSSGameInstance::GetActiveObstQuantities() const
{
	TArray<FString> Quantities = TArray<FString>();
	for (const AObst* Obst : ActiveObstructions)
	{
		if (!Quantities.Contains(Obst->ActiveQuantity))
			Quantities.Add(Obst->ActiveQuantity);
	}
	return Quantities;
}

void UVRSSGameInstance::GetActiveObstructionsMaxMinForQuantity(const FString Quantity, float& MinOut,
                                                               float& MaxOut) const
{
	MinOut = TNumericLimits<float>::Max();
	MaxOut = TNumericLimits<float>::Min();
	for (const AObst* Obst : ActiveObstructions)
	{
		if (Obst->ActiveQuantity.Equals(Quantity))
		{
			MinOut = FMath::Min(Obst->ObstAsset->ObstInfo.MinValues[Obst->ActiveQuantity], MinOut);
			MaxOut = FMath::Max(Obst->ObstAsset->ObstInfo.MaxValues[Obst->ActiveQuantity], MaxOut);
		}
	}
}

void UVRSSGameInstance::AddObst(AObst* Obst)
{
	ActiveObstructions.Add(Obst);

	// Bind to Event that gets called whenever a new obst displays boundary data (this might affect our ColorMap scale)
	ColorMapUpdateEvent.AddUObject(Obst, &AObst::UpdateColorMapScale);

	ChangeObstQuantity(Obst);
}

void UVRSSGameInstance::RemoveObst(AObst* Obst)
{
	const TArray<FString> QuantitiesBefore = GetActiveObstQuantities();
	ActiveObstructions.Remove(Obst);
	const TArray<FString> QuantitiesAfter = GetActiveObstQuantities();

	ChangeObstQuantity(Obst);

	if (QuantitiesBefore.Num() != QuantitiesAfter.Num())
	{
		Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD())->UserInterfaceUserWidget->UpdateColorMaps();
	}

	if (ActiveObstructions.Num() == 0)
	{
		TextureArrays["Obst"].Empty();
		GetTimerManager().ClearTimer(UpdateTimerHandles["Obst"]);
	}
}

void UVRSSGameInstance::ChangeObstQuantity(AObst* Obst)
{
	float Min, Max;
	const FString Quantity = Obst->ActiveQuantity;
	GetActiveMaxMinForQuantity(Quantity, Min, Max);
	ColorMapUpdateEvent.Broadcast(Quantity, Min, Max);
}

void UVRSSGameInstance::GetActiveMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
{
	float SMin, SMax, OMin, OMax;
	GetActiveSlicesMaxMinForQuantity(Quantity, SMin, SMax);
	GetActiveObstructionsMaxMinForQuantity(Quantity, OMin, OMax);
	MinOut = FMath::Min(SMin, OMin);
	MaxOut = FMath::Max(SMax, OMax);
}

FUpdateDataEvent& UVRSSGameInstance::RegisterTextureLoad(const FString Type, const FString& Directory,
                                                         TArray<FAssetData>* TextureArray)
{
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UTexture::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(Directory);

	ObjectLibrary->GetAssetDataList(*TextureArray);

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
	return UpdateDataEvents[Type];
}
