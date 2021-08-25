#include "VRSSGameInstanceSubsystem.h"
#include "Engine/VolumeTexture.h"
#include "TimerManager.h"
#include "Misc/DefaultValueHelper.h"

UVRSSGameInstanceSubsystem::UVRSSGameInstanceSubsystem(): UGameInstanceSubsystem()
{
	StreamableManager = new FStreamableManager();
	CurrentTimeStep = MaxTimeStep = 0;
}

void UVRSSGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set timer to set next frame (update next volume) after some time
	GetGameInstance()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UVRSSGameInstanceSubsystem::NextTimeStep,
	                                              CVarUpdateRate.GetValueOnGameThread(), true, 1.f);
}

void UVRSSGameInstanceSubsystem::NextTimeStep()
{
	CurrentTimeStep = (CurrentTimeStep + 1) % MaxTimeStep;

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
}

FUpdateVolumeEvent& UVRSSGameInstanceSubsystem::RegisterTextureLoad(const FString& Directory,
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
