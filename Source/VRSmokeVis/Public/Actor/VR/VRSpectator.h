#pragma once

#include "VRSpectator.generated.h"


/**
 * Base class for VRSpectator Blueprint.
 */
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
