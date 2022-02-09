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

	void InitSimulation(class ASimulation* Simulation);
protected:
	UPROPERTY()
	class ASimulation* Sim;
	
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
};
