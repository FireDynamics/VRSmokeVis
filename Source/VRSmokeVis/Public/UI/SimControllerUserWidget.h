#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"

#include "SimControllerUserWidget.generated.h"


UCLASS(Abstract)
class VRSMOKEVIS_API USimControllerUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USimControllerUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;
	
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;

	UFUNCTION()
	void InitSimulation(class ASimulation* Simulation);
protected:
	UFUNCTION()
	void OnObstCheckboxesStateChanged() const;
	UFUNCTION()
	void OnSliceCheckboxesStateChanged() const;
	UFUNCTION()
	void OnVolumeCheckboxesStateChanged() const;
	
	UFUNCTION()
	void InitObstCheckboxes() const;
	UFUNCTION()
	void InitSliceCheckboxes() const;
	UFUNCTION()
	void InitVolumeCheckboxes() const;
	
	UHorizontalBox* ConstructCheckboxRow(TScriptDelegate<> CheckboxDelegate, FString CheckboxName) const;

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UGridPanel *RootGridPanel;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UVerticalBox *ObstsVerticalBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UVerticalBox *SlicesVerticalBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UVerticalBox *VolumesVerticalBox;
	
protected:
	UPROPERTY()
	class ASimulation* Sim;
};
