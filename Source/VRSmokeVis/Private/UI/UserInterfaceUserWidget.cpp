

#include "UI/UserInterfaceUserWidget.h"

#include "VRSSGameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "UI/ColorMapUserWidget.h"
#include "Components/VerticalBox.h"


// Sets default values
UUserInterfaceUserWidget::UUserInterfaceUserWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

bool UUserInterfaceUserWidget::Initialize()
{	
	return Super::Initialize();
}

void UUserInterfaceUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateColorMaps();
	
	UVRSSGameInstance* GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->ColorMapUpdateEvent.AddUObject(this, &UUserInterfaceUserWidget::UpdateColorMapRange);

	// Update the ColorMap ranges manually once, in case the HUD's BeginPlay runs after the slice's/obst's BeginPlay
	float Min, Max;
	TArray<FString> Keys;
	ColorMapUserWidgets.GetKeys(Keys);
	for (const FString& Quantity : Keys)
	{
		GI->GetActiveMaxMinForQuantity(Quantity, Min, Max);
		UpdateColorMapRange(Quantity, Min, Max);
	}	
}

void UUserInterfaceUserWidget::NativeTick(const FGeometry& MyGeometry, const float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
}

void UUserInterfaceUserWidget::UpdateColorMaps()
{
	TArray<FString> OldQuantities = TArray<FString>();
	ColorMapUserWidgets.GetKeys(OldQuantities);
	
	const UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	for (FString Quantity : GI->GetActiveSliceQuantities())
	{
		OldQuantities.Remove(Quantity);
		if (!ColorMapUserWidgets.Contains(Quantity)){
			UColorMapUserWidget* ColorMapUserWidget = WidgetTree->ConstructWidget<UColorMapUserWidget>(ColorMapUserWidgetClass, FName(*("ColorMapUserWidget" + Quantity)));
			ColorMapUserWidget->ColorMapQuantity = Quantity;
			ColorMapUserWidgets.Add(Quantity, ColorMapUserWidget);
			ColorMapsVerticalBox->AddChildToVerticalBox(ColorMapUserWidget);
		}
	}
	for (FString Quantity : OldQuantities)
	{
		ColorMapUserWidgets[Quantity]->RemoveFromParent();
	}
}

void UUserInterfaceUserWidget::UpdateColorMapRange(const FString Quantity, const float NewMin, const float NewMax)
{
	FNumberFormattingOptions FormattingOptions;
	FormattingOptions.SetMaximumFractionalDigits(2);
	FormattingOptions.SetUseGrouping(false);
	ColorMapUserWidgets[Quantity]->TextBlockColorMapMin->SetText(FText::AsNumber(NewMin, &FormattingOptions));
	ColorMapUserWidgets[Quantity]->TextBlockColorMapMax->SetText(FText::AsNumber(NewMax, &FormattingOptions));
}
