#pragma once

#include "Blueprint/UserWidget.h"

#include "SimLoadingPromptUserWidget.generated.h"


/**
 * Prompt widget where the user has to enter a path to a simulation which he wants to load.  
 */
UCLASS(Abstract)
class VRSMOKEVIS_API USimLoadingPromptUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USimLoadingPromptUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, const float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ASimulation> SimulationClass;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UEditableText* SimulationFilePathInputText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UEditableText* SimulationOutDirInputText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* CancelButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* OkButton;

protected:
	UFUNCTION()
	void CancelPressed();

	UFUNCTION()
	void OkPressed();
};
