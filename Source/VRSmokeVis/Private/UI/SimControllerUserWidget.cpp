

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

	// Obsts
	TScriptDelegate<> ObstsDelegate;
	ObstsDelegate.BindUFunction(Sim, "CheckObstActivations");
	for (int i = 0; i < Sim->Obstructions.Num(); ++i) {
		const AObst* Obst = Sim->Obstructions[i];
		const FString ObstName = Obst->ObstAsset->ObstInfo.DataFileNames.begin().Value();
		UCheckBox* ObstActiveCheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), FName(*("ObstActiveCheckBox" + ObstName)));
		ObstsVerticalBox->AddChildToVerticalBox(ObstActiveCheckBox);
		ObstActiveCheckBox->OnCheckStateChanged.Add(ObstsDelegate);
	}
	
	// Slices
	TScriptDelegate<> SlicesDelegate;
	SlicesDelegate.BindUFunction(Sim, "CheckSliceActivations");
	for (int i = 0; i < Sim->Slices.Num(); ++i) {
		const ASlice* Slice = Sim->Slices[i];
		const FString SliceName = Slice->SliceAsset->SliceInfo.DataFileName;
		UCheckBox* SliceActiveCheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), FName(*("SliceActiveCheckBox" + SliceName)));
		SlicesVerticalBox->AddChildToVerticalBox(SliceActiveCheckBox);
		SliceActiveCheckBox->OnCheckStateChanged.Add(SlicesDelegate);
	}
	
	// Volumes
	TScriptDelegate<> VolumesDelegate;
	VolumesDelegate.BindUFunction(Sim, "CheckVolumeActivations");
	for (int i = 0; i < Sim->Volumes.Num(); ++i) {
		const ARaymarchVolume* Volume = Sim->Volumes[i];
		const FString VolumeName = Volume->VolumeAsset->VolumeInfo.DataFileName;
		UCheckBox* VolumeActiveCheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), FName(*("VolumeActiveCheckBox" + VolumeName)));
		VolumesVerticalBox->AddChildToVerticalBox(VolumeActiveCheckBox);
		VolumeActiveCheckBox->OnCheckStateChanged.Add(VolumesDelegate);
	}
	
}
