#include "UI/ColorMapUserWidget.h"


// Sets default values
UColorMapUserWidget::UColorMapUserWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UColorMapUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TextBlockColorMapQuantity->SetText(FText::FromString(ColorMapQuantity));
}

void UColorMapUserWidget::NativeTick(const FGeometry& MyGeometry, const float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
}

