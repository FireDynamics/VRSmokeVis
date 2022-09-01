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
		ObstsScrollBox->AddChild(ConstructCheckboxRow(ObstsDelegate, ObstName, ObstName));
	}
}

void USimControllerUserWidget::InitSliceCheckboxes() const
{
	TScriptDelegate<> SlicesDelegate;
	SlicesDelegate.BindUFunction(Sim, "CheckSliceActivations");
	TArray<ASlice*> Slices = Sim->GetAllSlices();

	TMap<FString, TArray<ASlice*>> GroupedSlices = TMap<FString, TArray<ASlice*>>();
	for (int i = 0; i < Slices.Num(); ++i)
	{
		const FString SliceQuantity = Cast<USliceDataInfo>(Slices[i]->DataAsset->DataInfo)->Quantity;
		if (!GroupedSlices.Contains(SliceQuantity)) GroupedSlices.Add(SliceQuantity, TArray<ASlice*>());
		GroupedSlices[SliceQuantity].Add(Slices[i]);
	}

	for (auto SliceGroup : GroupedSlices)
	{
		UTextBlock* CheckboxLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
																		FName(*("QuantityLabel" + SliceGroup.Key)));
		FSlateFontInfo FontInfo = CheckboxLabel->Font;
		FontInfo.Size = 15;
		CheckboxLabel->SetFont(FontInfo);
		CheckboxLabel->SetText(FText::FromString(SliceGroup.Key));
		SlicesScrollBox->AddChild(CheckboxLabel);
		for (int i = 0; i < SliceGroup.Value.Num(); ++i)
		{
			SlicesScrollBox->AddChild(ConstructCheckboxRow(SlicesDelegate, SliceGroup.Value[i]->DataAsset->DataInfo->ImportName, SliceGroup.Value[i]->DataAsset->DataInfo->FdsName));
		}
	}
}

void USimControllerUserWidget::InitVolumeCheckboxes() const
{
	// Volumes
	TScriptDelegate<> VolumesDelegate;
	VolumesDelegate.BindUFunction(Sim, "CheckVolumeActivations");
	TArray<ARaymarchVolume*> Volumes = Sim->GetAllVolumes();
	TMap<FString, TArray<ARaymarchVolume*>> GroupedVolumes = TMap<FString, TArray<ARaymarchVolume*>>();
	for (int i = 0; i < Volumes.Num(); ++i)
	{
		const FString VolumeQuantity = Cast<UVolumeDataInfo>(Volumes[i]->DataAsset->DataInfo)->Quantity;
		if (!GroupedVolumes.Contains(VolumeQuantity)) GroupedVolumes.Add(VolumeQuantity, TArray<ARaymarchVolume*>());
		GroupedVolumes[VolumeQuantity].Add(Volumes[i]);
	}

	for (auto VolumeGroup : GroupedVolumes)
	{
		UTextBlock* CheckboxLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
																		FName(*("QuantityLabel" + VolumeGroup.Key)));
		FSlateFontInfo FontInfo = CheckboxLabel->Font;
		FontInfo.Size = 15;
		CheckboxLabel->SetFont(FontInfo);
		CheckboxLabel->SetText(FText::FromString(VolumeGroup.Key));
		VolumesScrollBox->AddChild(CheckboxLabel);
		for (int i = 0; i < VolumeGroup.Value.Num(); ++i)
		{
			VolumesScrollBox->AddChild(ConstructCheckboxRow(VolumesDelegate, VolumeGroup.Value[i]->DataAsset->DataInfo->ImportName, VolumeGroup.Value[i]->DataAsset->DataInfo->FdsName));
		}
	}
}

UHorizontalBox* USimControllerUserWidget::ConstructCheckboxRow(const TScriptDelegate<> CheckboxDelegate,
                                                               const FString CheckboxUniqueName,
                                                               const FString CheckboxUIName) const
{
	UHorizontalBox* CheckboxRow = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), FName(*("CheckboxRow" + CheckboxUniqueName)));

	UTextBlock* CheckboxLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
	                                                                    FName(*("Label" + CheckboxUniqueName)));
	FSlateFontInfo FontInfo = CheckboxLabel->Font;
	FontInfo.Size = 10;
	CheckboxLabel->SetFont(FontInfo);
	CheckboxLabel->SetText(FText::FromString(CheckboxUIName));
	CheckboxRow->AddChildToHorizontalBox(CheckboxLabel);

	UCheckBox* Checkbox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(),
	                                                             FName(*("CheckBox" + CheckboxUniqueName)));
	CheckboxRow->AddChildToHorizontalBox(Checkbox);
	Checkbox->OnCheckStateChanged.Add(CheckboxDelegate);

	return CheckboxRow;
}
