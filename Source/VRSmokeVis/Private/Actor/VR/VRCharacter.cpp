#include "Actor/VR/VRCharacter.h"
#include "Actor/VRSSPlayerController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "VRSSGameInstanceSubsystem.h"
#include "Actor/Simulation.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "UI/SimLoadingPromptUserWidget.h"

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

	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;
	
	RightController = GetWorld()->SpawnActor<AVRSSPlayerController>(PlayerControllerClass, SpawnParams);
	LeftController = GetWorld()->SpawnActor<AVRSSPlayerController>(PlayerControllerClass, SpawnParams);
	RightController->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	LeftController->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RightController->SetupInput(InputComponent);
	LeftController->SetupInput(InputComponent);

	// Time controls
	InputComponent->BindAction("Pause", IE_Pressed, this, &AVRCharacter::TogglePauseSimulation)
	              .bExecuteWhenPaused = true;
	InputComponent->BindAction("FastForward", IE_Pressed, this, &AVRCharacter::FastForwardSimulation)
	              .bExecuteWhenPaused = true;
	InputComponent->BindAction("Rewind", IE_Pressed, this, &AVRCharacter::RewindSimulation)
	              .bExecuteWhenPaused = true;
	InputComponent->BindAction("ToggleHUD", IE_Pressed, this, &AVRCharacter::ToggleHUDVisibility)
	              .bExecuteWhenPaused = true;
	InputComponent->BindAction("ShowSimLoadingPrompt", IE_Pressed, this, &AVRCharacter::ToggleSimLoadingPrompt)
	              .bExecuteWhenPaused = true;

	// Start the world paused
	TogglePauseSimulation();

	// Init loading prompt
	SimLoadingPromptUserWidget = CreateWidget<USimLoadingPromptUserWidget>(GetGameInstance(), SimLoadingPromptUserWidgetClass);
	int32 X, Y;
	Cast<APlayerController>(GetController())->GetViewportSize(X, Y);
	// Todo: The widgets size is hardcoded for now as getting the size dynamically did not work
	SimLoadingPromptUserWidget->SetPositionInViewport(FVector2d(X-250.f, Y-100.f)/2);
	
	// Check if there is at least one simulation in the world already, if not show simulation loading UI
	if(!TActorIterator<AActor>(GetWorld(), ASimulation::StaticClass())) ToggleSimLoadingPrompt();
}

void AVRCharacter::ToggleSimLoadingPrompt()
{
	if (SimLoadingPromptUserWidget->IsInViewport()){
		SimLoadingPromptUserWidget->RemoveFromViewport();
		Cast<APlayerController>(GetController())->SetShowMouseCursor(false);
	} else
	{
		SimLoadingPromptUserWidget->AddToViewport();
		Cast<APlayerController>(GetController())->SetShowMouseCursor(true);
	}
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
{
	TArray<AActor*> Simulations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASimulation::StaticClass(), Simulations);
	for (AActor* Simulation : Simulations)
		Cast<ASimulation>(Simulation)->RewindSimulation(25.0f);
}

void AVRCharacter::ToggleHUDVisibility()
{
	GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>()->ToggleHUDVisibility();
}
