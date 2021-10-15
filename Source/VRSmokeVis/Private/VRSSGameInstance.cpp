#include "VRSSGameInstance.h"
#include "Engine/VolumeTexture.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DefaultValueHelper.h"
#include "Kismet/KismetTextLibrary.h"
#include "Actor/RaymarchLight.h"
#include "UI/UserInterfaceUserWidget.h"
#include "UI/TimeUserWidget.h"
#include "UI/VRSSHUD.h"
#include "Components/TimelineComponent.h"
#include "Actor/Slice.h"

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

	for (TArray<FAssetData>* AssetsData : VolumeTextureArrays)
	{
		StreamableManager->Unload((*AssetsData)[PreviousTextureIndex].ToSoftObjectPath());
		AssetsToLoad.Add((*AssetsData)[NextTextureIndex].ToSoftObjectPath());
	}
	UpdateVolumeEvent.Broadcast(CurrentTimeStep);

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
			Quantities.Add(
				Slice->SliceAsset->SliceInfo.Quantity);
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
	GetActiveSlicesMaxMinForQuantity(Quantity, Min, Max);
	SliceUpdateEvent.Broadcast(Quantity, Min, Max);
}

void UVRSSGameInstance::RemoveSlice(ASlice* Slice)
{
	const TArray<FString> QuantitiesBefore = GetActiveSliceQuantities();
	ActiveSlices.Remove(Slice);
	const TArray<FString> QuantitiesAfter = GetActiveSliceQuantities();

	float Min, Max;
	const FString Quantity = Slice->SliceAsset->SliceInfo.Quantity;
	GetActiveSlicesMaxMinForQuantity(Quantity, Min, Max);
	SliceUpdateEvent.Broadcast(Quantity, Min, Max);

	if (QuantitiesBefore.Num() != QuantitiesAfter.Num())
	{
		Cast<AVRSSHUD>(GetPrimaryPlayerController()->GetHUD())->UserInterfaceUserWidget->UpdateColorMaps();
	}
}

FUpdateVolumeEvent& UVRSSGameInstance::RegisterTextureLoad(const FString& Directory, TArray<FAssetData>* TextureArray)
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

	VolumeTextureArrays.Add(TextureArray);

	// Load first n (max 10) textures synchronously so they will be available from the very beginning
	const int TexturesToLoad = FMath::Min(TextureArray->Num(), 10);

	// Make sure any Textures could be found
	check(TexturesToLoad)

	for (int i = 0; i < TexturesToLoad; ++i)
	{
		StreamableManager->LoadSynchronous((*TextureArray)[i].ToSoftObjectPath());
	}
	return UpdateVolumeEvent;
}
