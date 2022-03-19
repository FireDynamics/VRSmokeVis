#pragma once

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "VR/Grabbable.h"
#include "MotionControllerComponent.h"

#include "VRSSPlayerController.generated.h"

class UMotionControllerComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogVRSSPlayerController, Log, All);

/**
 * A class for motion controller actors.
 */
UCLASS(Abstract)
class VRSMOKEVIS_API AVRSSPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AVRSSPlayerController();

	// Collision component to detect overlaps with other scene actors.
	UPROPERTY(EditAnywhere)
	USphereComponent* CollisionComponent;

	// The motion controller component used to drive this MotionController.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionController | Component")
	UMotionControllerComponent* MotionControllerComponent = nullptr;

	// Skeletal mesh of the controller.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionController | Component")
	UStaticMeshComponent* ControllerStaticMeshComponent = nullptr;

	// Widget interactor which allows interacting with VR UI.
	UPROPERTY(EditAnywhere)
	UWidgetInteractionComponent* WidgetInteractor;

	// Skeletal mesh representing the WidgetInteractor ray.
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* WidgetInteractorVisualizer;

	// If true, this controller should go into the right hand.
	UPROPERTY(EditAnywhere)
	bool bIsInRightHand = true;

	// Sets up this controller's actions to the provided InputComponent.
	virtual void SetupInput(UInputComponent* InInputComponent);

	// True if the controller grip is pressed.
	bool bIsGripPressed = false;

	virtual void OnGripPressed();

	virtual void OnGripReleased();

	virtual void OnTriggerAxis(float Axis);

	virtual void OnJoystickYAxis(float Axis);

	virtual void OnTriggerPressed();

	virtual void OnTriggerReleased();

	virtual void OnGripAxis(float Axis);

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp,
	                    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
	                  UPrimitiveComponent* OtherComp,
	                  int32 OtherBodyIndex);

	UFUNCTION()
	void OnWidgetInteractorHoverChanged(UWidgetComponent* Old, UWidgetComponent* New);

	// The actor currently hovered by the sphere collision, if any.
	// Could be remade into an array to allow hovering multiple actors at once.
	IGrabbable* HoveredActor = nullptr;

	// The currently grabbed actor, if any.
	IGrabbable* GrabbedActor = nullptr;
};
