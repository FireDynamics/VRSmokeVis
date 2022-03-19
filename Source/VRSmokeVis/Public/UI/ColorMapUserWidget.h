#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ColorMapUserWidget.generated.h"

UCLASS(Abstract)
class VRSMOKEVIS_API UColorMapUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UColorMapUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TextBlockColorMapMin;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TextBlockColorMapMax;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TextBlockColorMapQuantity;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* ImageColorMap;

	UPROPERTY(VisibleAnywhere)
	FString ColorMapQuantity;
};
