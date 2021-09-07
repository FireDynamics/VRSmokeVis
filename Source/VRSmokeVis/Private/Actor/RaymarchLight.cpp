// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/RaymarchLight.h"

#include "Components/PointLightComponent.h"
#include "Components/TimelineComponent.h"


ARaymarchLight::ARaymarchLight()
{
	PrimaryActorTick.bCanEverTick = false;

	LightIntensityTimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("Light intensity timeline"));
	
	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("Point Light"));
	PointLightComponent->SetMobility(EComponentMobility::Stationary);
	PointLightComponent->SetIntensityUnits(ELightUnits::Candelas);
	PointLightComponent->SetIntensity(10.0f);
}

void ARaymarchLight::BeginPlay()
{
	Super::BeginPlay();

	if (LightIntensityCurve){
		// Gets called by the timeline each time there is an update in the intensity curve
		FOnTimelineFloat UpdateLightIntensityFunction;
		UpdateLightIntensityFunction.BindUFunction(PointLightComponent, TEXT("SetIntensity"));
		LightIntensityTimelineComponent->AddInterpFloat(LightIntensityCurve, UpdateLightIntensityFunction, NAME_None, TEXT("LightIntensityTrack"));
		LightIntensityTimelineComponent->Play();
	}
}
