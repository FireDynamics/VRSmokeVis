

#include "Actor/VR/VRPawn.h"
#include "Actor/VRSSPlayerController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "VRSSGameInstance.h"

AVRPawn::AVRPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	HMDScene = CreateDefaultSubobject<USceneComponent>(TEXT("VR HMD Scene"));
	SetRootComponent(HMDScene);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VR Camera"));
	VRCamera->SetupAttachment(HMDScene);
}

void AVRPawn::BeginPlay()
{
	Super::BeginPlay();

	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	const EVRPlatform VRPlatformType = EVRPlatform::Default;
	FControllerPlatformClasses PlatformClasses;

	if (PerPlatformControllers.Contains(VRPlatformType))
	{
		PlatformClasses = PerPlatformControllers[VRPlatformType];
	}

	if (PlatformClasses.LeftControllerClass && PlatformClasses.RightControllerClass)
	{
		RightController = GetWorld()->SpawnActor<AVRSSPlayerController>(PlatformClasses.RightControllerClass, SpawnParams);
		LeftController = GetWorld()->SpawnActor<AVRSSPlayerController>(PlatformClasses.LeftControllerClass, SpawnParams);

		RightController->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		LeftController->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		RightController->SetupInput(InputComponent);
		LeftController->SetupInput(InputComponent);
	}
	
	// Time controls
	InputComponent->BindAction("Pause", IE_Pressed, this, &AVRPawn::TogglePauseSimulation).bExecuteWhenPaused = true;
	InputComponent->BindAction("FastForward", IE_Pressed, this, &AVRPawn::FastForwardSimulation).bExecuteWhenPaused = true;
	InputComponent->BindAction("Rewind", IE_Pressed, this, &AVRPawn::RewindSimulation).bExecuteWhenPaused = true;
	
	InputComponent->BindAction("ToggleHUD", IE_Pressed, this, &AVRPawn::ToggleHUDVisibility).bExecuteWhenPaused = true; 
}

void AVRPawn::TogglePauseSimulation()
{
	UE_LOG(LogVRSSPlayerController, Warning, TEXT("Paused simulation."))
	UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->TogglePauseSimulation();
}

void AVRPawn::FastForwardSimulation()
{
	UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->FastForwardSimulation(25.0f);
}

void AVRPawn::RewindSimulation()
{
	UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->RewindSimulation(25.0f);
}

void AVRPawn::ToggleHUDVisibility()
{
	UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->ToggleHUDVisibility();
}