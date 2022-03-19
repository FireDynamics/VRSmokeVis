#pragma once

#include "GameFramework/HUD.h"
#include "VRSSHUD.generated.h"

UCLASS()
class VRSMOKEVIS_API AVRSSHUD : public AHUD
{
	GENERATED_BODY()

public:
	AVRSSHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

	/** Called manually as soon as other relevant systems are setup. */
	void InitHUD();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserInterfaceUserWidget> UserInterfaceUserWidgetClass;

	UPROPERTY(BlueprintReadOnly)
	class UUserInterfaceUserWidget* UserInterfaceUserWidget;
};
