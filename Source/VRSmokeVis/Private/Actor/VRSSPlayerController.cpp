

#include "Actor/VRSSPlayerController.h"
#include "Actor/VR/Grabbable.h"
#include "Components/WidgetInteractionComponent.h"
#include "XRMotionControllerBase.h"

DEFINE_LOG_CATEGORY(LogVRSSPlayerController)

AVRSSPlayerController::AVRSSPlayerController()
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
	WidgetInteractor->OnHoveredWidgetChanged.AddDynamic(this, &AVRSSPlayerController::OnWidgetInteractorHoverChanged);

	WidgetInteractorVisualizer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WidgetInteractorVisualizer"));
	WidgetInteractorVisualizer->SetupAttachment(ControllerStaticMeshComponent);
	WidgetInteractorVisualizer->SetVisibility(false);
	WidgetInteractorVisualizer->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	CollisionComponent->SetupAttachment(ControllerStaticMeshComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionProfileName("WorldDynamic");
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AVRSSPlayerController::OnOverlapBegin);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AVRSSPlayerController::OnOverlapEnd);
}

void AVRSSPlayerController::SetupInput(UInputComponent* InInputComponent)
{
	const FString Hand = bIsInRightHand ? "Right" : "Left";
	const FName HandSourceId = bIsInRightHand ? FXRMotionControllerBase::RightHandSourceId : FXRMotionControllerBase::LeftHandSourceId;

	MotionControllerComponent->SetTrackingMotionSource(HandSourceId);
	
	InInputComponent->BindAction(FName(Hand + "_Grip"), IE_Pressed, this, &AVRSSPlayerController::OnGripPressed);
	InInputComponent->BindAction(FName(Hand + "_Grip"), IE_Released, this, &AVRSSPlayerController::OnGripReleased);
	InInputComponent->BindAction(FName(Hand + "_Trigger"), IE_Pressed, this, &AVRSSPlayerController::OnTriggerPressed);
	InInputComponent->BindAction(FName(Hand + "_Trigger"), IE_Released, this, &AVRSSPlayerController::OnTriggerReleased);
	InInputComponent->BindAxis(FName(Hand + "_Grip_Axis"), this, &AVRSSPlayerController::OnGripAxis);
	InInputComponent->BindAxis(FName(Hand + "_Trigger_Axis"), this, &AVRSSPlayerController::OnTriggerAxis);
	InInputComponent->BindAxis(FName(Hand + "_Joystick_Y"), this, &AVRSSPlayerController::OnJoystickYAxis);
}

void AVRSSPlayerController::OnGripPressed()
{
	if (HoveredActor)
	{
		HoveredActor->OnGrabbed(ControllerStaticMeshComponent);
		GrabbedActor = HoveredActor;
		HoveredActor = nullptr;
	}
}

void AVRSSPlayerController::OnGripReleased()
{
	if (GrabbedActor)
	{
		GrabbedActor->OnReleased();
		HoveredActor = GrabbedActor;
		GrabbedActor = nullptr;
	}
}

void AVRSSPlayerController::OnTriggerAxis(float Axis)
{
	// Update animation
}

void AVRSSPlayerController::OnJoystickYAxis(float Axis)
{
	if (WidgetInteractor)
	{
		WidgetInteractor->ScrollWheel(Axis * 50);
	}
}

void AVRSSPlayerController::OnTriggerPressed()
{
	if (WidgetInteractor)
	{
		WidgetInteractor->PressPointerKey(EKeys::LeftMouseButton);
	}
}

void AVRSSPlayerController::OnTriggerReleased()
{
	if (WidgetInteractor)
	{
		WidgetInteractor->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

void AVRSSPlayerController::OnGripAxis(float Axis)
{
	// Update animation.
}

void AVRSSPlayerController::OnOverlapBegin(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<IGrabbable>(OtherActor))
	{
		HoveredActor = Cast<IGrabbable>(OtherActor);
	}
}

void AVRSSPlayerController::OnOverlapEnd(
	class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HoveredActor == Cast<IGrabbable>(OtherActor))
	{
		HoveredActor = nullptr;
	}
}

void AVRSSPlayerController::OnWidgetInteractorHoverChanged(UWidgetComponent* Old, UWidgetComponent* New)
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
