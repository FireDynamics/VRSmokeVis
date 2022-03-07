#pragma once

#include "Blueprint/UserWidget.h"

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
	void InitObstCheckboxes() const;
	UFUNCTION()
	void InitSliceCheckboxes() const;
	UFUNCTION()
	void InitVolumeCheckboxes() const;
	
	class UHorizontalBox* ConstructCheckboxRow(TScriptDelegate<> CheckboxDelegate, FString CheckboxName) const;

public:	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UScrollBox *ObstsScrollBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UScrollBox *SlicesScrollBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UScrollBox *VolumesScrollBox;
	
protected:
	UPROPERTY()
	class ASimulation* Sim;
};
