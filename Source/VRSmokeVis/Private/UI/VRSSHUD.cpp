#include "UI/VRSSHUD.h"

#include "Actor/VRSSPlayerController.h"
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
	if (!UserInterfaceUserWidget){
		UserInterfaceUserWidget = CreateWidget<UUserInterfaceUserWidget>(GetWorld(), UserInterfaceUserWidgetClass);
	}
}
