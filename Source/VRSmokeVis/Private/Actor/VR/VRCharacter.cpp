#include "Actor/VR/VRCharacter.h"
#include "Actor/VRSSPlayerController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "VRSSGameInstanceSubsystem.h"
#include "Actor/Simulation.h"
#include "Kismet/GameplayStatics.h"

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
}

void AVRCharacter::TogglePauseSimulation()
{
	UE_LOG(LogVRSSPlayerController, Warning, TEXT("Paused simulation."))

	TArray<AActor*> Simulations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASimulation::StaticClass(), Simulations);
	for (AActor* Simulation : Simulations)
		Cast<ASimulation>(Simulation)->TogglePauseSimulation();
}

void AVRCharacter::FastForwardSimulation()
{
	TArray<AActor*> Simulations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASimulation::StaticClass(), Simulations);
	for (AActor* Simulation : Simulations)
		Cast<ASimulation>(Simulation)->FastForwardSimulation(25.0f);
}

void AVRCharacter::RewindSimulation()
{TArray<AActor*> Simulations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASimulation::StaticClass(), Simulations);
	for (AActor* Simulation : Simulations)
		Cast<ASimulation>(Simulation)->RewindSimulation(25.0f);
}

void AVRCharacter::ToggleHUDVisibility()
{
	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->ToggleHUDVisibility();
}
