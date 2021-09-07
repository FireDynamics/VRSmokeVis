// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/VRSSPlayerController.h"
#include "VRSSGameInstance.h"


// Sets default values
AVRSSPlayerController::AVRSSPlayerController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVRSSPlayerController::SetupInputComponent()
{
	// Always call this.
	Super::SetupInputComponent();
	
	// Time controls
	InputComponent->BindAction("Pause", IE_Pressed, this, &AVRSSPlayerController::TogglePauseSimulation).bExecuteWhenPaused = true;
	InputComponent->BindAction("FastForward", IE_Pressed, this, &AVRSSPlayerController::FastForwardSimulation).bExecuteWhenPaused = true;
	InputComponent->BindAction("Rewind", IE_Pressed, this, &AVRSSPlayerController::RewindSimulation).bExecuteWhenPaused = true; 
}

void AVRSSPlayerController::TogglePauseSimulation()
{
	UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->TogglePauseSimulation();
}

void AVRSSPlayerController::FastForwardSimulation()
{
	UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->FastForwardSimulation(25.0f);
}

void AVRSSPlayerController::RewindSimulation()
{
	UVRSSGameInstance *GI = Cast<UVRSSGameInstance>(GetGameInstance());
	GI->RewindSimulation(25.0f);
}
