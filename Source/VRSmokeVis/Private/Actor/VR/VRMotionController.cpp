

#include "Actor/VR/VRSSController.h"
#include "Actor/VR/Grabbable.h"
#include "Components/WidgetInteractionComponent.h"
#include "XRMotionControllerBase.h"


AVRSSController::AVRSSController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	MotionControllerComponent = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	MotionControllerComponent->SetupAttachment(RootComponent);
	MotionControllerComponent->SetRelativeLocation(FVector(0, 0, 0));

	ControllerStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ControllerMesh"));
	ControllerStaticMeshComponent->SetupAttachment(MotionControllerComponent);
	ControllerStaticMeshComponent->SetCanEverAffectNavigation(false);
	ControllerStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WidgetInteractor = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractor"));
	WidgetInteractor->SetupAttachment(ControllerStaticMeshComponent);
	WidgetInteractor->OnHoveredWidgetChanged.AddDynamic(this, &AVRSSController::OnWidgetInteractorHoverChanged);

	WidgetInteractorVisualizer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WidgetInteractorVisualizer"));
	WidgetInteractorVisualizer->SetupAttachment(ControllerStaticMeshComponent);
	WidgetInteractorVisualizer->SetVisibility(false);
	WidgetInteractorVisualizer->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	CollisionComponent->SetupAttachment(ControllerStaticMeshComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionProfileName("WorldDynamic");
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AVRSSController::OnOverlapBegin);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AVRSSController::OnOverlapEnd);
}

void AVRSSController::SetupInput(UInputComponent* InInputComponent)
{
	const FString Hand = bIsInRightHand ? "Right" : "Left";
	const FName HandSourceId = bIsInRightHand ? FXRMotionControllerBase::RightHandSourceId : FXRMotionControllerBase::LeftHandSourceId;

	MotionControllerComponent->SetTrackingMotionSource(HandSourceId);
	
	InInputComponent->BindAction(FName(Hand + "_Grip"), IE_Pressed, this, &AVRSSController::OnGripPressed);
	InInputComponent->BindAction(FName(Hand + "_Grip"), IE_Released, this, &AVRSSController::OnGripReleased);
	InInputComponent->BindAction(FName(Hand + "_Trigger"), IE_Pressed, this, &AVRSSController::OnTriggerPressed);
	InInputComponent->BindAction(FName(Hand + "_Trigger"), IE_Released, this, &AVRSSController::OnTriggerReleased);
	InInputComponent->BindAxis(FName(Hand + "_Grip_Axis"), this, &AVRSSController::OnGripAxis);
	InInputComponent->BindAxis(FName(Hand + "_Trigger_Axis"), this, &AVRSSController::OnTriggerAxis);
	InInputComponent->BindAxis(FName(Hand + "_Joystick_Y"), this, &AVRSSController::OnJoystickYAxis);
}

void AVRSSController::OnGripPressed()
{
	if (HoveredActor)
	{
		HoveredActor->OnGrabbed(ControllerStaticMeshComponent);
		GrabbedActor = HoveredActor;
		HoveredActor = nullptr;
	}
}

void AVRSSController::OnGripReleased()
{
	if (GrabbedActor)
	{
		GrabbedActor->OnReleased();
		HoveredActor = GrabbedActor;
		GrabbedActor = nullptr;
	}
}

void AVRSSController::OnTriggerAxis(float Axis)
{
	// Update animation
}

void AVRSSController::OnJoystickYAxis(float Axis)
{
	if (WidgetInteractor)
	{
		WidgetInteractor->ScrollWheel(Axis * 50);
	}
}

void AVRSSController::OnTriggerPressed()
{
	if (WidgetInteractor)
	{
		WidgetInteractor->PressPointerKey(EKeys::LeftMouseButton);
	}
}

void AVRSSController::OnTriggerReleased()
{
	if (WidgetInteractor)
	{
		WidgetInteractor->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

void AVRSSController::OnGripAxis(float Axis)
{
	// Update animation.
}

void AVRSSController::OnOverlapBegin(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<IGrabbable>(OtherActor))
	{
		HoveredActor = Cast<IGrabbable>(OtherActor);
	}
}

void AVRSSController::OnOverlapEnd(
	class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HoveredActor == Cast<IGrabbable>(OtherActor))
	{
		HoveredActor = nullptr;
	}
}

void AVRSSController::OnWidgetInteractorHoverChanged(UWidgetComponent* Old, UWidgetComponent* New)
{
	// Hide the WidgetInteractorVisualizer when not pointing at a menu.
	if (New)
	{
		WidgetInteractorVisualizer->SetVisibility(false);
	}
	else if (Old)
	{
		WidgetInteractorVisualizer->SetVisibility(true);
	}
}