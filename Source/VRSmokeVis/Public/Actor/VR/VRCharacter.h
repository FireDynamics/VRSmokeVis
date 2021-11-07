

#pragma once

#include "Camera/CameraComponent.h"
#include "Actor/VRSSPlayerController.h"
#include "GameFramework/Character.h"

#include "VRCharacter.generated.h"

class AVRSSPlayerController;

UENUM()
enum class EVRPlatform
{
	Oculus = 0,
	Vive = 1,
	Default,
};

// The 2 classes of VRMotionController to spawn for each platform.
USTRUCT()
struct FControllerPlatformClasses
{
	GENERATED_BODY()
	// Class of AVRMotionController to spawn for left hand.
	UPROPERTY(EditAnywhere)
	TSubclassOf<AVRSSPlayerController> LeftControllerClass;

	// Class of AVRMotionController to spawn for right hand.
	UPROPERTY(EditAnywhere)
	TSubclassOf<AVRSSPlayerController> RightControllerClass;
};

/**
 * VR pawn for handling volumetric volumes.
 */
UCLASS() class AVRCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	// Default constructor.
	AVRCharacter();

	// Root component to attach everything to.
	UPROPERTY(VisibleAnywhere)
	USceneComponent* HMDScene;

	// VR Camera component.
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* VRCamera;

	// Contains the classes of controllers to be spawned per each platform.
	UPROPERTY(EditAnywhere)
	TMap<EVRPlatform, FControllerPlatformClasses> PerPlatformControllers;

	// Controller spawned for the left hand.
	UPROPERTY(VisibleAnywhere)
	AVRSSPlayerController* LeftController;

	// Controller spawned for the right hand.
	UPROPERTY(VisibleAnywhere)
	AVRSSPlayerController* RightController;

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;
	
protected:
	/** Time controls **/
	void TogglePauseSimulation();
	void FastForwardSimulation();
	void RewindSimulation();
	
	void ToggleHUDVisibility();
	
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
};
