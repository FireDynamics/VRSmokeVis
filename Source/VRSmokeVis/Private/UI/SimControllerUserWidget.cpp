#include "UI/SimControllerUserWidget.h"

#include "Actor/Obst.h"
#include "Actor/RaymarchVolume.h"
#include "Actor/Simulation.h"
#include "Actor/Slice.h"
#include "Assets/SliceDataInfo.h"
#include "Assets/VolumeAsset.h"
#include "Assets/VolumeDataInfo.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"

// Sets default values
USimControllerUserWidget::USimControllerUserWidget(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer)
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
	ObstsDelegate.BindUFunction(Sim, "CheckObstActivations");
	TArray<AObst*> Obstructions = Sim->GetAllObstructions();
	for (int i = 0; i < Obstructions.Num(); ++i)
	{
		const FString ObstName = Obstructions[i]->DataAsset->DataInfo->FdsName;
		ObstsScrollBox->AddChild(ConstructCheckboxRow(ObstsDelegate, ObstName));
	}
}

void USimControllerUserWidget::InitSliceCheckboxes() const
{
	TScriptDelegate<> SlicesDelegate;
	SlicesDelegate.BindUFunction(Sim, "CheckSliceActivations");
	TArray<ASlice*> Slices = Sim->GetAllSlices();
	for (int i = 0; i < Slices.Num(); ++i)
	{
		const FString SliceName = Cast<USliceDataInfo>(Slices[i]->DataAsset->DataInfo)->Quantity + " - " + Slices[i]->DataAsset->DataInfo->FdsName;
		SlicesScrollBox->AddChild(ConstructCheckboxRow(SlicesDelegate, SliceName));
	}
}

void USimControllerUserWidget::InitVolumeCheckboxes() const
{
	// Volumes
	TScriptDelegate<> VolumesDelegate;
	VolumesDelegate.BindUFunction(Sim, "CheckVolumeActivations");
	TArray<ARaymarchVolume*> Volumes = Sim->GetAllVolumes();
	for (int i = 0; i < Volumes.Num(); ++i)
	{
		const FString VolumeName = Cast<UVolumeDataInfo>(Volumes[i]->DataAsset->DataInfo)->Quantity + " - " + Volumes[i]->DataAsset->DataInfo->FdsName;
		VolumesScrollBox->AddChild(ConstructCheckboxRow(VolumesDelegate, VolumeName));
	}
}

UHorizontalBox* USimControllerUserWidget::ConstructCheckboxRow(const TScriptDelegate<> CheckboxDelegate,
                                                               const FString CheckboxName) const
{
	UHorizontalBox* CheckboxRow = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), FName(*("CheckboxRow" + CheckboxName)));

	UTextBlock* CheckboxLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
	                                                                    FName(*("Label" + CheckboxName)));
	FSlateFontInfo FontInfo = CheckboxLabel->Font;
	FontInfo.Size = 10;
	CheckboxLabel->SetFont(FontInfo);
	CheckboxLabel->SetText(FText::FromString(CheckboxName));
	CheckboxRow->AddChildToHorizontalBox(CheckboxLabel);

	UCheckBox* Checkbox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(),
	                                                             FName(*("CheckBox" + CheckboxName)));
	CheckboxRow->AddChildToHorizontalBox(Checkbox);
	Checkbox->OnCheckStateChanged.Add(CheckboxDelegate);

	return CheckboxRow;
}
