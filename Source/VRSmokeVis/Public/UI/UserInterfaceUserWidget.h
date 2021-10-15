#pragma once

#include "Blueprint/UserWidget.h"
#include "UserInterfaceUserWidget.generated.h"

UCLASS(Abstract)
class UUserInterfaceUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UUserInterfaceUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;
	
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void UpdateColorMaps();

	UFUNCTION(BlueprintCallable)
	void UpdateColorMapRange(const FString Quantity, const float NewMin, const float NewMax);
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UGridPanel *RootGridPanel;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UTimeUserWidget> TimeUserWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTimeUserWidget* TimeUserWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UVerticalBox *ColorMapsVerticalBox;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UColorMapUserWidget> ColorMapUserWidgetClass;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, class UColorMapUserWidget*> ColorMapUserWidgets;
};
