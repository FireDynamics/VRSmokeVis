// Fill out your copyright notice in the Description page of Project Settings.


#include "VRSSGameMode.h"

#include "VRSSGameInstance.h"
#include "UI/TimeUserWidget.h"


// Sets default values
AVRSSGameMode::AVRSSGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AVRSSGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Init UI
	Cast<UVRSSGameInstance>(GetGameInstance())->TimeUserWidget->AddToViewport();
}