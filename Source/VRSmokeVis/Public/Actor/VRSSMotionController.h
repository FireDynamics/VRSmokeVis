#pragma once

#include "VRSSMotionController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVRSSPlayerController, Log, All);

/**
 * A class for motion controller actors.
 */
UCLASS(Abstract)
class VRSMOKEVIS_API AVRSSMotionController : public AActor
{
	GENERATED_BODY()
public:
	/** Sets default values for this actor's properties */
	AVRSSMotionController();

	/** Sets up this controller's actions to the provided InputComponent */
	virtual void SetupInput(UInputComponent* InInputComponent);

	virtual void OnJoystickYAxis(float Axis);
	
	/** The motion controller component used to drive this MotionController */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionController | Component")
	class UMotionControllerComponent* MotionControllerComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionController | Component")
	class UMotionControllerComponent* MotionControllerAimComponent = nullptr;
	
	/** Widget interactor which allows interacting with VR UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UWidgetInteractionComponent* WidgetInteractor;

	/** If true, this controller should go into the right hand */
	UPROPERTY(EditAnywhere)
	bool bIsInRightHand;
};
