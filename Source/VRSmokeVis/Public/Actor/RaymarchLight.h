// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RaymarchLight.generated.h"

UCLASS()
class VRSMOKEVIS_API ARaymarchLight : public AActor
{
	GENERATED_BODY()

public:
	ARaymarchLight();

protected:
	virtual void BeginPlay() override;

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
