#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TimeUserWidget.generated.h"


/**
 * Widget to show game and simulation time for each type of data.
 */
UCLASS(Abstract)
class VRSMOKEVIS_API UTimeUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTimeUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;

	/** Updates the time values on the screen */
	UFUNCTION()
	void UpdateTimeTextBlocks() const;

	/** Get the TextBlock widget for the corresponding data type [Slice, Obst, Volume] */
	UFUNCTION()
	UTextBlock* GetTextBlockValueTimesteps(const FString Type) const;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TextBlockValueGameTime;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TextBlockValueSimTime;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlockValueTimestepObsts;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlockValueTimestepSlices;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlockValueTimestepVolumes;

	UPROPERTY(VisibleAnywhere)
	float CurrentSimTime = 0;

	UPROPERTY()
	float SimTimeScale = 1;

	UPROPERTY(VisibleAnywhere)
	bool bIsPaused = false;

	UPROPERTY(VisibleAnywhere)
	float CurrentGameTime = 0;
};
