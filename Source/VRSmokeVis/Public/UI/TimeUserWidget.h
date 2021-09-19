#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TimeUserWidget.generated.h"

UCLASS(Abstract)
class UTimeUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTimeUserWidget(const FObjectInitializer& ObjectInitializer);

	// Optionally override the Blueprint "Event Construct" event
	virtual void NativeConstruct() override;

	// Optionally override the tick event
	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;

	UFUNCTION()
	void UpdateTimeTextBlocks();
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock *TextBlockValueGameTime;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock *TextBlockValueSimTime;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock *TextBlockValueTimestep;

	UPROPERTY(VisibleAnywhere)
	float CurrentSimTime = 0;

	UPROPERTY()
	float SimTimeScale = 1;

	UPROPERTY(VisibleAnywhere)
	bool bIsPaused = false;

protected:
	UPROPERTY(VisibleAnywhere)
	float CurrentGameTime = 0;
};