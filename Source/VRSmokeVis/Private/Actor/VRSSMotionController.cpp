#include "Actor/VRSSMotionController.h"
#include "Components/WidgetInteractionComponent.h"
#include "MotionControllerComponent.h"

DEFINE_LOG_CATEGORY(LogVRSSPlayerController)

AVRSSMotionController::AVRSSMotionController()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	MotionControllerComponent = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	MotionControllerComponent->SetupAttachment(RootComponent);
	MotionControllerComponent->SetRelativeLocation(FVector(0, 0, 0));
	MotionControllerComponent->bDisplayDeviceModel = true;
	
	MotionControllerAimComponent = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerAim"));
	MotionControllerAimComponent->SetupAttachment(RootComponent);
	MotionControllerAimComponent->SetRelativeLocation(FVector(0, 0, 0));
	
	WidgetInteractor = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractor"));
	WidgetInteractor->SetupAttachment(MotionControllerAimComponent);
	WidgetInteractor->TraceChannel = ECC_Pawn; // ECC_GameTraceChannel1;  // = "3DWidget"
}

void AVRSSMotionController::SetupInput(UInputComponent* InInputComponent)
{
	WidgetInteractor->PointerIndex = bIsInRightHand ? 1 : 0;
	
	const FString Hand = bIsInRightHand ? "Right" : "Left";
	MotionControllerComponent->SetTrackingMotionSource(FName(Hand));
	MotionControllerAimComponent->SetTrackingMotionSource(FName(Hand + "Aim"));

	// InInputComponent->BindAxis(FName("MovementAxisY" + Hand), this, &AVRSSPlayerController::OnJoystickYAxis);
}

void AVRSSMotionController::OnJoystickYAxis(float Axis)
{
	if (WidgetInteractor)
	{
		WidgetInteractor->ScrollWheel(Axis * 50);
	}
}