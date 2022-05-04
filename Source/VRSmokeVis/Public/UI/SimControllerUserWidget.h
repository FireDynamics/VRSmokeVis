#pragma once

#include "Blueprint/UserWidget.h"

#include "SimControllerUserWidget.generated.h"


/**
 * Widget to control a simulation, e.g. show and hide data assets.
 */
UCLASS(Abstract)
class VRSMOKEVIS_API USimControllerUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USimControllerUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;

	/** Initializes this widget with a simulation */
	UFUNCTION()
	void InitSimulation(class ASimulation* Simulation);
	
protected:
	/** Initializes the checkboxes to show or hide obstructions */
	UFUNCTION()
	void InitObstCheckboxes() const;
	/** Initializes the checkboxes to show or hide slices */
	UFUNCTION()
	void InitSliceCheckboxes() const;
	/** Initializes the checkboxes to show or hide volumes */
	UFUNCTION()
	void InitVolumeCheckboxes() const;

	/** Creates a single checkbox row including the checkbox and the label */
	class UHorizontalBox* ConstructCheckboxRow(TScriptDelegate<> CheckboxDelegate, FString CheckboxName) const;

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UScrollBox* ObstsScrollBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UScrollBox* SlicesScrollBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UScrollBox* VolumesScrollBox;

protected:
	UPROPERTY()
	class ASimulation* Sim;
};
