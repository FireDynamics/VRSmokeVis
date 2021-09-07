// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RaymarchLight.generated.h"

UCLASS()
class VRSMOKEVIS_API ARaymarchLight : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARaymarchLight();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere)
	UCurveFloat *LightIntensityCurve;
	
	UPROPERTY(BlueprintReadOnly)
	class UTimelineComponent *LightIntensityTimelineComponent;

protected:
	UPROPERTY(VisibleAnywhere)
	class UPointLightComponent *PointLightComponent;
};
