#pragma once

#include "Blueprint/UserWidget.h"
#include "UserInterfaceUserWidget.generated.h"

UCLASS()
class UUserInterfaceUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UUserInterfaceUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void UpdateColorMaps(TArray<FString> ActiveQuantities);

	/** (Re-)Initialize all colormaps (all invisible for now) */
	UFUNCTION(BlueprintCallable)
	void InitColorMaps(class UVRSSConfig* Config, TMap<FString, float> Mins, TMap<FString, float> Maxs);

	void AddSimulationController(TScriptDelegate<> ToggleSimControllerDelegate, const FString& SimName);

	/** Get all quantities for which colormaps are currently shown in the UI */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetActiveColorMapQuantities();

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UTimeUserWidget> TimeUserWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTimeUserWidget* TimeUserWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UColorMapUserWidget> ColorMapUserWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UGridPanel* RootGridPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UVerticalBox* ColorMapsVerticalBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UHorizontalBox* SimulationControllersHorizontalBox;

protected:
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, class UColorMapUserWidget*> ColorMapUserWidgets;
};
