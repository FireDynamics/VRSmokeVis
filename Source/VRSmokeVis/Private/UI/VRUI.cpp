#include "UI/VRUI.h"

#include "Components/WidgetComponent.h"
#include "UI/UserInterfaceUserWidget.h"


AVRUI::AVRUI()
{
	PrimaryActorTick.bCanEverTick = true;
	
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	RootComponent = WidgetComponent;
	WidgetComponent->SetWorldRotation(FRotator(0, 0, -180));
}

void AVRUI::BeginPlay()
{
	Super::BeginPlay();
}

void AVRUI::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVRUI::Init(UUserInterfaceUserWidget* UserInterfaceUserWidget)
{
	WidgetComponent->SetWidget(UserInterfaceUserWidget);
}
