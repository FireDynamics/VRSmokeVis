#pragma once

#include "VRSpectator.generated.h"

UCLASS()
class VRSMOKEVIS_API AVRSpectator : public AActor
{
	GENERATED_BODY()

public:
	AVRSpectator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
