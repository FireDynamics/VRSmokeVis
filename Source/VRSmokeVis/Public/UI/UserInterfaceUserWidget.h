#pragma once

#include "Blueprint/UserWidget.h"
#include "UserInterfaceUserWidget.generated.h"


/**
 * Base interface which is responsible for all communication of the user with the underlying systems.
 */
UCLASS()
class VRSMOKEVIS_API UUserInterfaceUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UUserInterfaceUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;

	/** Hide all colormaps that should not be shown anymore and show all active ones */
	UFUNCTION(BlueprintCallable)
	void UpdateColorMaps(TArray<FString> ActiveQuantities);

	/** (Re-)Initialize all colormaps (all invisible at first) */
	UFUNCTION(BlueprintCallable)
	void InitColorMaps(class UVRSSConfig* Config, TMap<FString, float> Mins, TMap<FString, float> Maxs);

	/** Add the simulation controller interface for a simulation */
	UFUNCTION()
	void AddSimulationController(class ASimulation* Sim) const;

	/** Get all quantities for which colormaps are currently shown in the UI */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetActiveColorMapQuantities();

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UTimeUserWidget> TimeUserWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class USimControllerUserWidget> SimControllerUserWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UColorMapUserWidget> ColorMapUserWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTimeUserWidget* TimeUserWidget;

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
