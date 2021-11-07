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

	// Set timer to set next frame (update next volume) after some time
	GetTimerManager().SetTimer(UpdateTimerHandle, this, &UVRSSGameInstance::NextTimeStep, UpdateRate, true, 1.f);

	// Set the curve length of all (controlled) lights in the scene to the simulation time
	TArray<AActor*> FoundLights;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), RaymarchLightClass, FoundLights);
	for (AActor* Light : FoundLights)
	{
		Cast<ARaymarchLight>(Light)->LightIntensityTimelineComponent->SetTimelineLength(MaxTimeStep);
	}
}

void UVRSSGameInstance::InitUpdateRate(const float UpdateRateSuggestion)
{
	if (UpdateRate == 0) UpdateRate = UpdateRateSuggestion;

	SimTimeStepLength = UpdateRate;
}

void UVRSSGameInstance::SetUpdateRate(const float NewUpdateRate)
{
	UpdateRate = NewUpdateRate;
	GetTimerManager().ClearTimer(UpdateTimerHandle);
	GetTimerManager().SetTimer(UpdateTimerHandle, this, &UVRSSGameInstance::NextTimeStep, UpdateRate, true, 1.f);

	Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD())->UserInterfaceUserWidget->TimeUserWidget->SimTimeScale = SimTimeStepLength /
		NewUpdateRate;
}

void UVRSSGameInstance::FastForwardSimulation(const float Amount)
{
	CurrentTimeStep += Amount - 1;
	NextTimeStep();

	// Update the UI
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD());
	HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = CurrentTimeStep * SimTimeStepLength;
	HUD->UserInterfaceUserWidget->TimeUserWidget->TextBlockValueTimestep->SetText(FText::AsNumber(CurrentTimeStep));
	HUD->UserInterfaceUserWidget->TimeUserWidget->UpdateTimeTextBlocks();
}

void UVRSSGameInstance::RewindSimulation(const float Amount)
{
	CurrentTimeStep -= FMath::Min<float>(Amount, CurrentTimeStep) + 1;
	NextTimeStep();

	// Update the UI
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD());
	HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = CurrentTimeStep * SimTimeStepLength;
	HUD->UserInterfaceUserWidget->TimeUserWidget->TextBlockValueTimestep->SetText(FText::AsNumber(CurrentTimeStep));
	HUD->UserInterfaceUserWidget->TimeUserWidget->UpdateTimeTextBlocks();
}

void UVRSSGameInstance::TogglePauseSimulation()
{
	bIsPaused = !bIsPaused;
	UGameplayStatics::SetGamePaused(GetWorld(), bIsPaused);

	// Inform UI
	Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD())->UserInterfaceUserWidget->TimeUserWidget->bIsPaused = bIsPaused;
}

void UVRSSGameInstance::ToggleHUDVisibility() const
{
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD());
	HUD->UserInterfaceUserWidget->SetVisibility(HUD->UserInterfaceUserWidget->IsVisible()
		                                   ? ESlateVisibility::Hidden
		                                   : ESlateVisibility::Visible);
}

void UVRSSGameInstance::NextTimeStep()
{
	const AVRSSHUD* HUD = Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD());
	CurrentTimeStep += 1;
	if (CurrentTimeStep >= MaxTimeStep)
	{
		CurrentTimeStep = 0;
		HUD->UserInterfaceUserWidget->TimeUserWidget->CurrentSimTime = .0f;
	}

	// Unload the VolumeTexture of the second to last time step (not the last one as it might still be referenced
	const int PreviousTextureIndex = (CurrentTimeStep + MaxTimeStep - 2) % MaxTimeStep;
	const int NextTextureIndex = (CurrentTimeStep + 9) % MaxTimeStep;
	TArray<FSoftObjectPath> AssetsToLoad;

	// Todo: Check for different time resolutions for different data types (e.g. slices might have more time steps than obstructions)
	for (TArray<FAssetData>* AssetsData : TextureArrays)
	{
		StreamableManager->Unload((*AssetsData)[PreviousTextureIndex].ToSoftObjectPath());
		AssetsToLoad.Add((*AssetsData)[NextTextureIndex].ToSoftObjectPath());
	}
	UpdateDataEvent.Broadcast(CurrentTimeStep);

	StreamableManager->RequestAsyncLoad(AssetsToLoad);

	// Update the UI
	HUD->UserInterfaceUserWidget->TimeUserWidget->TextBlockValueTimestep->SetText(FText::AsNumber(CurrentTimeStep));
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

void UVRSSGameInstance::GetActiveObstructionsMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
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

	float Min, Max;
	const FString Quantity = Obst->ActiveQuantity;
	GetActiveMaxMinForQuantity(Quantity, Min, Max);
	ColorMapUpdateEvent.Broadcast(Quantity, Min, Max);
}

void UVRSSGameInstance::RemoveObst(AObst* Obst)
{
	const TArray<FString> QuantitiesBefore = GetActiveObstQuantities();
	ActiveObstructions.Remove(Obst);
	const TArray<FString> QuantitiesAfter = GetActiveObstQuantities();

	float Min, Max;
	const FString Quantity = Obst->ActiveQuantity;
	GetActiveMaxMinForQuantity(Quantity, Min, Max);
	ColorMapUpdateEvent.Broadcast(Quantity, Min, Max);

	if (QuantitiesBefore.Num() != QuantitiesAfter.Num())
	{
		Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD())->UserInterfaceUserWidget->UpdateColorMaps();
	}
}

void UVRSSGameInstance::GetActiveMaxMinForQuantity(const FString Quantity, float& MinOut, float& MaxOut) const
{
	float SMin, SMax, OMin, OMax;
	GetActiveSlicesMaxMinForQuantity(Quantity, SMin, SMax);
	GetActiveObstructionsMaxMinForQuantity(Quantity, OMin, OMax);
	MinOut = FMath::Min(SMin, OMin);
	MaxOut = FMath::Max(SMax, OMax);
}

FUpdateDataEvent& UVRSSGameInstance::RegisterTextureLoad(const FString& Directory, TArray<FAssetData>* TextureArray)
{
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UTexture::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	ObjectLibrary->LoadAssetDataFromPath(Directory);

	ObjectLibrary->GetAssetDataList(*TextureArray);

	MaxTimeStep = TextureArray->Num();

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

	TextureArrays.Add(TextureArray);

	// Load first n (max 10) textures synchronously so they will be available from the very beginning
	const int TexturesToLoad = FMath::Min(TextureArray->Num(), 10);

	// Make sure any Textures could be found
	check(TexturesToLoad)

	for (int i = 0; i < TexturesToLoad; ++i)
	{
		StreamableManager->LoadSynchronous((*TextureArray)[i].ToSoftObjectPath());
	}
	return UpdateDataEvent;
}
