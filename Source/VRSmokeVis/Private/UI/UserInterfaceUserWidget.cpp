#include "UI/UserInterfaceUserWidget.h"

#include "VRSSConfig.h"
#include "Actor/Simulation.h"
#include "Assets/SimulationAsset.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "UI/ColorMapUserWidget.h"
#include "Components/VerticalBox.h"
#include "UI/SimControllerUserWidget.h"


// Sets default values
UUserInterfaceUserWidget::UUserInterfaceUserWidget(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer)
{
}

bool UUserInterfaceUserWidget::Initialize()
{
	ColorMapUserWidgets = TMap<FString, UColorMapUserWidget*>();

	return Super::Initialize();
}

void UUserInterfaceUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUserInterfaceUserWidget::NativeTick(const FGeometry& MyGeometry, const float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
}

void UUserInterfaceUserWidget::InitColorMaps(UVRSSConfig* Config, TMap<FString, float> Mins, TMap<FString, float> Maxs)
{
	// First remove all old colormaps
	for (const auto ColorMap : ColorMapUserWidgets)
	{
		ColorMap.Value->RemoveFromParent();
	}
	ColorMapUserWidgets.Empty();

	// Now add a colormap for each quantity
	TArray<FString> Quantities = TArray<FString>();
	Mins.GetKeys(Quantities);

	FNumberFormattingOptions LabelFormattingOptions;
	LabelFormattingOptions.SetMaximumFractionalDigits(2);
	LabelFormattingOptions.SetUseGrouping(false);
	for (FString& Quantity : Quantities)
	{
		UColorMapUserWidget* ColorMapUserWidget = WidgetTree->ConstructWidget<UColorMapUserWidget>(
			ColorMapUserWidgetClass, FName(*("ColorMapUserWidget" + Quantity)));
		ColorMapUserWidget->ColorMapQuantity = Quantity;
		ColorMapUserWidgets.Add(Quantity, ColorMapUserWidget);
		ColorMapsVerticalBox->AddChildToVerticalBox(ColorMapUserWidget);
		ColorMapUserWidget->ImageColorMap->SetBrushFromTexture(Config->GetColorMap(Quantity));

		ColorMapUserWidget->TextBlockColorMapMin->SetText(FText::AsNumber(Mins[Quantity], &LabelFormattingOptions));
		ColorMapUserWidget->TextBlockColorMapMax->SetText(FText::AsNumber(Maxs[Quantity], &LabelFormattingOptions));

		ColorMapUserWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUserInterfaceUserWidget::AddSimulationController(ASimulation* Sim) const
{
	const FString SimName = Sim->SimulationAsset->GetName();
	USimControllerUserWidget* SimControllerWidget = WidgetTree->ConstructWidget<USimControllerUserWidget>(
		SimControllerUserWidgetClass, FName(*("SimControllerUserWidget" + SimName)));
	Sim->SimControllerUserWidget = SimControllerWidget;
	SimControllerWidget->InitSimulation(Sim);
	SimulationControllersHorizontalBox->AddChildToHorizontalBox(SimControllerWidget);
}

TArray<FString> UUserInterfaceUserWidget::GetActiveColorMapQuantities()
{
	TArray<FString> Quantities = TArray<FString>();
	for (auto ColorMap : ColorMapUserWidgets)
	{
		if (ColorMap.Value->GetVisibility() != ESlateVisibility::Hidden)
			Quantities.Add(ColorMap.Key);
	}
	return Quantities;
}

void UUserInterfaceUserWidget::UpdateColorMaps(TArray<FString> ActiveQuantities)
{
	TArray<FString> OldQuantities = GetActiveColorMapQuantities();

	// Show currently falsely inactive ColorMaps
	for (FString Quantity : ActiveQuantities)
		if (!OldQuantities.Contains(Quantity))
			ColorMapUserWidgets[Quantity]->SetVisibility(ESlateVisibility::Visible);

	// Hide currently falsely active ColorMaps
	for (FString& Quantity : OldQuantities)
		if (!ActiveQuantities.Contains(Quantity))
			ColorMapUserWidgets[Quantity]->SetVisibility(ESlateVisibility::Hidden);
}
