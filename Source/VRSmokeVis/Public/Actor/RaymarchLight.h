// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RaymarchLight.generated.h"


/**
 * A light source which gets dimmed or illuminated over time due to smoke covering the light source.
 */
UCLASS()
class VRSMOKEVIS_API ARaymarchLight : public AActor
{
	GENERATED_BODY()

public:
	ARaymarchLight();

protected:
	virtual void BeginPlay() override;

	/** Dim or illuminate the light source (e.g. by updating intensity and radius) */
	UFUNCTION()
	void UpdateLight(const float NewValue) const;

public:
	UPROPERTY(EditAnywhere)
	UCurveFloat* LightIntensityCurve;

	UPROPERTY(BlueprintReadOnly)
	class UTimelineComponent* LightIntensityTimelineComponent;

protected:
	UPROPERTY(VisibleAnywhere)
	class UPointLightComponent* PointLightComponent;
};
