

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

	if (!bIsPaused){
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
