#include "Actor/VR/VRCharacter.h"
#include "Actor/VRSSPlayerController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "VRSSGameInstance.h"

AVRCharacter::AVRCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	HMDScene = CreateDefaultSubobject<USceneComponent>(TEXT("VR HMD Scene"));
	SetRootComponent(HMDScene);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VR Camera"));
	VRCamera->SetupAttachment(HMDScene);
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Todo
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	// Todo
	const EVRPlatform VRPlatformType = EVRPlatform::Default;
	FControllerPlatformClasses PlatformClasses;

	if (PerPlatformControllers.Contains(VRPlatformType))
	{
		PlatformClasses = PerPlatformControllers[VRPlatformType];
	}

	if (PlatformClasses.LeftControllerClass && PlatformClasses.RightControllerClass)
	{
		RightController = GetWorld()->SpawnActor<AVRSSPlayerController>(PlatformClasses.RightControllerClass,
		                                                                SpawnParams);
		LeftController = GetWorld()->SpawnActor<AVRSSPlayerController>(PlatformClasses.LeftControllerClass,
		                                                               SpawnParams);

		RightController->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		LeftController->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		RightController->SetupInput(InputComponent);
		LeftController->SetupInput(InputComponent);
	}

	// Time controls
	InputComponent->BindAction("Pause", IE_Pressed, this, &AVRCharacter::TogglePauseSimulation)
	              .bExecuteWhenPaused = true;
	InputComponent->BindAction("FastForward", IE_Pressed, this, &AVRCharacter::FastForwardSimulation)
	              .bExecuteWhenPaused = true;
	InputComponent->BindAction("Rewind", IE_Pressed, this, &AVRCharacter::RewindSimulation)
	              .bExecuteWhenPaused = true;
	InputComponent->BindAction("ToggleHUD", IE_Pressed, this, &AVRCharacter::ToggleHUDVisibility)
	              .bExecuteWhenPaused = true;

	// Bind movement events
	InputComponent->BindAxis("MoveForward", this, &AVRCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AVRCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "TurnRate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &AVRCharacter::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AVRCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &AVRCharacter::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AVRCharacter::LookUpAtRate);
}

void AVRCharacter::MoveForward(const float Value)
{
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AVRCharacter::MoveRight(const float Value)
{
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AVRCharacter::TurnAtRate(const float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * 45.f * GetWorld()->GetDeltaSeconds());
}

void AVRCharacter::LookUpAtRate(const float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * 45.f * GetWorld()->GetDeltaSeconds());
}

void AVRCharacter::TogglePauseSimulation()
{
	UE_LOG(LogVRSSPlayerController, Warning, TEXT("Paused simulation."))
	UVRSSGameInstance* GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->TogglePauseSimulation();
}

void AVRCharacter::FastForwardSimulation()
{
	UVRSSGameInstance* GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->FastForwardSimulation(25.0f);
}

void AVRCharacter::RewindSimulation()
{
	UVRSSGameInstance* GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->RewindSimulation(25.0f);
}

void AVRCharacter::ToggleHUDVisibility()
{
	UVRSSGameInstance* GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->ToggleHUDVisibility();
}
