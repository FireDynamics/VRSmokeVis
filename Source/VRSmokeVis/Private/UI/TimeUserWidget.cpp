#include "UI/TimeUserWidget.h"


// Sets default values
UTimeUserWidget::UTimeUserWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UTimeUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}


void UTimeUserWidget::NativeTick(const FGeometry& MyGeometry, const float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (!bIsPaused)
	{
		CurrentGameTime += DeltaTime;
		CurrentSimTime += DeltaTime * SimTimeScale;
		UpdateTimeTextBlocks();
	}
}

void UTimeUserWidget::UpdateTimeTextBlocks() const
{
	TextBlockValueGameTime->SetText(FText::AsNumber(CurrentGameTime));
	TextBlockValueSimTime->SetText(FText::AsNumber(CurrentSimTime));
}

UTextBlock* UTimeUserWidget::GetTextBlockValueTimesteps(const FString Type) const
{
	if (Type == "Obst") return TextBlockValueTimestepObsts;
	if (Type == "Slice") return TextBlockValueTimestepSlices;
	if (Type == "Volume") return TextBlockValueTimestepVolumes;
	return nullptr;
}
