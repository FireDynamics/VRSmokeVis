// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "VRSSPlayerController.generated.h"

UCLASS()
class VRSMOKEVIS_API AVRSSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVRSSPlayerController();
	
	/** Time controls **/
	void TogglePauseSimulation();
	void FastForwardSimulation();
	void RewindSimulation();

	virtual void SetupInputComponent() override;
};

