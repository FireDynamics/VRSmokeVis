

#include "UI/SimControllerUserWidget.h"

#include "Actor/Obst.h"
#include "Actor/RaymarchVolume.h"
#include "Actor/Simulation.h"
#include "Actor/Slice.h"
#include "Assets/ObstAsset.h"
#include "Assets/SliceAsset.h"
#include "Assets/VolumeAsset.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

// Sets default values
USimControllerUserWidget::USimControllerUserWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

bool USimControllerUserWidget::Initialize()
{	
	return Super::Initialize();
}

void USimControllerUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USimControllerUserWidget::NativeTick(const FGeometry& MyGeometry, const float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
}

void USimControllerUserWidget::InitSimulation(ASimulation* Simulation)
{
	Sim = Simulation;

	InitObstCheckboxes();
	InitSliceCheckboxes();
	InitVolumeCheckboxes();
}

void USimControllerUserWidget::InitObstCheckboxes() const
{
	TScriptDelegate<> ObstsDelegate;
	ObstsDelegate.BindUFunction(Sim, "OnObstCheckboxesStateChanged");
	TArray<AObst*> Obstructions = Sim->GetAllObstructions();
	for (int i = 0; i < Obstructions.Num(); ++i) {
		const FString ObstName = Obstructions[i]->ObstAsset->ObstInfo.DataFileNames.begin().Value();
		ObstsVerticalBox->AddChildToVerticalBox(ConstructCheckboxRow(ObstsDelegate, ObstName));
	}
}

void USimControllerUserWidget::InitSliceCheckboxes() const
{
	TScriptDelegate<> SlicesDelegate;
	SlicesDelegate.BindUFunction(Sim, "OnSliceCheckboxesStateChanged");
	TArray<ASlice*> Slices = Sim->GetAllSlices();
	for (int i = 0; i < Slices.Num(); ++i) {
		const FString SliceName = Slices[i]->SliceAsset->SliceInfo.DataFileName;
		SlicesVerticalBox->AddChildToVerticalBox(ConstructCheckboxRow(SlicesDelegate, SliceName));
	}
}

void USimControllerUserWidget::InitVolumeCheckboxes() const
{
	// Volumes
	TScriptDelegate<> VolumesDelegate;
	VolumesDelegate.BindUFunction(Sim, "OnVolumeCheckboxesStateChanged");
	TArray<ARaymarchVolume*> Volumes = Sim->GetAllVolumes();
	for (int i = 0; i < Volumes.Num(); ++i) {
		const FString VolumeName = Volumes[i]->VolumeAsset->VolumeInfo.DataFileName;
		VolumesVerticalBox->AddChildToVerticalBox(ConstructCheckboxRow(VolumesDelegate, VolumeName));
	}
}

UHorizontalBox* USimControllerUserWidget::ConstructCheckboxRow(const TScriptDelegate<> CheckboxDelegate, const FString CheckboxName) const
{
	UHorizontalBox* CheckboxRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), FName(*("CheckboxRow" + CheckboxName)));
		
	UTextBlock* CheckboxLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(*("Label" + CheckboxName)));
	CheckboxRow->AddChildToHorizontalBox(CheckboxLabel);
	CheckboxLabel->SetText(FText::FromString(CheckboxName));
		
	UCheckBox* Checkbox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), FName(*("CheckBox" + CheckboxName)));
	CheckboxRow->AddChildToHorizontalBox(Checkbox);
	Checkbox->OnCheckStateChanged.Add(CheckboxDelegate);

	return CheckboxRow;
}

void USimControllerUserWidget::OnObstCheckboxesStateChanged() const
{
	TArray<bool> ObstsActive = TArray<bool>();
	for(UWidget* Child : ObstsVerticalBox->GetAllChildren())
	{
		ObstsActive.Add(Cast<UCheckBox>(Cast<UHorizontalBox>(Child)->GetChildAt(1))->IsChecked());
	}
	Sim->CheckObstActivations(ObstsActive);
}

void USimControllerUserWidget::OnSliceCheckboxesStateChanged() const
{
	TArray<bool> SlicesActive = TArray<bool>();
	for(UWidget* Child : SlicesVerticalBox->GetAllChildren())
	{
		SlicesActive.Add(Cast<UCheckBox>(Cast<UHorizontalBox>(Child)->GetChildAt(1))->IsChecked());
	}
	Sim->CheckSliceActivations(SlicesActive);
}

void USimControllerUserWidget::OnVolumeCheckboxesStateChanged() const
{
	TArray<bool> VolumesActive = TArray<bool>();
	for(UWidget* Child : VolumesVerticalBox->GetAllChildren())
	{
		VolumesActive.Add(Cast<UCheckBox>(Cast<UHorizontalBox>(Child)->GetChildAt(1))->IsChecked());
	}
	Sim->CheckVolumeActivations(VolumesActive);
}
