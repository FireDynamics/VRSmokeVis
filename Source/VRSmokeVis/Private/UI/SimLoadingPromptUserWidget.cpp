#include "UI/SimLoadingPromptUserWidget.h"

#include "Actor/Simulation.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Assets/SimulationAsset.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Util/AssetCreationUtilities.h"
#include "Util/ImportUtilities.h"

// #include "Blueprint/WidgetTree.h"

// Sets default values
USimLoadingPromptUserWidget::USimLoadingPromptUserWidget(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer)
{
}

bool USimLoadingPromptUserWidget::Initialize()
{
	return Super::Initialize();
}

void USimLoadingPromptUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CancelButton->OnClicked.AddUniqueDynamic(this, &USimLoadingPromptUserWidget::CancelPressed);
	OkButton->OnClicked.AddUniqueDynamic(this, &USimLoadingPromptUserWidget::OkPressed);
	
	FSlateFontInfo FontInfo = SimulationFilePathInputText->WidgetStyle.Font;
	FontInfo.Size = 14;
	SimulationFilePathInputText->WidgetStyle.SetFont(FontInfo);
	SimulationOutDirInputText->WidgetStyle.SetFont(FontInfo);
}

void USimLoadingPromptUserWidget::NativeTick(const FGeometry& MyGeometry, const float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
}

void USimLoadingPromptUserWidget::CancelPressed()
{
	this->RemoveFromViewport();
	Cast<APlayerController>(GetWorld()->GetFirstPlayerController())->SetShowMouseCursor(false);
}

void USimLoadingPromptUserWidget::OkPressed()
{
	const FString& FileName = SimulationFilePathInputText->GetText().ToString();
	const FString OutDir = FPaths::Combine("/Game/", SimulationOutDirInputText->GetText().ToString());
	
	const FString OutputDirAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), OutDir.RightChop(6)));
	FImportUtils::VerifyOrCreateDirectory(OutputDirAbsolutePath);
	
	USimulationAsset* SimAsset = Cast<USimulationAsset>(FAssetCreationUtils::CreateSimulation(FileName, OutDir));

	TArray<FString> PathsToScan = TArray<FString>();
	PathsToScan.Add(OutDir);
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan, true);
		
	// Todo: Make location and rotation configurable in widget
	FTransform ZeroTransform;
	ASimulation* Sim = Cast<ASimulation>(GetWorld()->SpawnActorDeferred<ASimulation>(SimulationClass, ZeroTransform));
	Sim->SimulationAsset = SimAsset;
	Sim->FinishSpawning(ZeroTransform);
	Sim->SpawnSimulationGeometry();
	
	this->RemoveFromViewport();
	Cast<APlayerController>(GetWorld()->GetFirstPlayerController())->SetShowMouseCursor(false);
}