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
}

void AVRSSHUD::InitHUD()
{
	UserInterfaceUserWidget = CreateWidget<UUserInterfaceUserWidget>(GetWorld(), UserInterfaceUserWidgetClass);
}
