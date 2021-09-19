#include "VRSSGameInstance.h"
#include "Engine/VolumeTexture.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DefaultValueHelper.h"
#include "Kismet/KismetTextLibrary.h"
#include "Actor/RaymarchLight.h"
#include "UI/TimeUserWidget.h"
#include "Components/TimelineComponent.h"

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
	TArray<AActor *> FoundLights;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), RaymarchLightClass, FoundLights);
	for (AActor* Light : FoundLights)
	{
		Cast<ARaymarchLight>(Light)->LightIntensityTimelineComponent->SetTimelineLength(MaxTimeStep);
	}

	// Setup time UI
	TimeUserWidget = CreateWidget<UTimeUserWidget>(GetWorld(), TimeUserWidgetClass);
	// TimeUserWidget->AddToViewport();
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

	TimeUserWidget->SimTimeScale = SimTimeStepLength / NewUpdateRate;
}

void UVRSSGameInstance::FastForwardSimulation(const float Amount)
{
	CurrentTimeStep += Amount - 1;
	NextTimeStep();

	// Update the UI
	TimeUserWidget->CurrentSimTime = CurrentTimeStep * SimTimeStepLength;
	TimeUserWidget->TextBlockValueTimestep->SetText(FText::AsNumber(CurrentTimeStep));
	TimeUserWidget->UpdateTimeTextBlocks();
}

void UVRSSGameInstance::RewindSimulation(const float Amount)
{
	CurrentTimeStep -= FMath::Min<float>(Amount, CurrentTimeStep) + 1;
	NextTimeStep();
	
	// Update the UI
	TimeUserWidget->CurrentSimTime = CurrentTimeStep * SimTimeStepLength;
	TimeUserWidget->TextBlockValueTimestep->SetText(FText::AsNumber(CurrentTimeStep));
	TimeUserWidget->UpdateTimeTextBlocks();
}

void UVRSSGameInstance::TogglePauseSimulation()
{
	bIsPaused = !bIsPaused;
	UGameplayStatics::SetGamePaused(GetWorld(), bIsPaused);

	// Inform UI
	TimeUserWidget->bIsPaused = bIsPaused;
}

void UVRSSGameInstance::NextTimeStep()
{
	CurrentTimeStep += 1;
	if (CurrentTimeStep >= MaxTimeStep)
	{
		CurrentTimeStep = 0;
		TimeUserWidget->CurrentSimTime = .0f;
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
	TimeUserWidget->TextBlockValueTimestep->SetText(FText::AsNumber(CurrentTimeStep));
}

FUpdateVolumeEvent& UVRSSGameInstance::RegisterTextureLoad(const FString& Directory,
                                                           TArray<FAssetData>* TextureArray)
{
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UVolumeTexture::StaticClass(), false, GIsEditor);
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
