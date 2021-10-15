

#include "UI/VRSSHUD.h"

#include "UI/UserInterfaceUserWidget.h"


AVRSSHUD::AVRSSHUD()
{
}

void AVRSSHUD::DrawHUD()
{
	Super::DrawHUD();
}

void AVRSSHUD::BeginPlay()
{
	Super::BeginPlay();

	UserInterfaceUserWidget = CreateWidget<UUserInterfaceUserWidget>(GetWorld(), UserInterfaceUserWidgetClass);
	UserInterfaceUserWidget->AddToViewport();
}
