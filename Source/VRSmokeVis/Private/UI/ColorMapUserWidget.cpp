

#include "UI/ColorMapUserWidget.h"

#include "VRSSGameInstanceSubsystem.h"


// Sets default values
UColorMapUserWidget::UColorMapUserWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UColorMapUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TextBlockColorMapQuantity->SetText(FText::FromString(ColorMapQuantity));
	const UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	ImageColorMap->SetBrushFromTexture(GI->Config->ColorMaps[ColorMapQuantity]);
}

void UColorMapUserWidget::NativeTick(const FGeometry& MyGeometry, const float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
}

