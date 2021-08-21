

#pragma once

#include "Grabbable.generated.h"

UINTERFACE(Blueprintable)
class UGrabbable : public UInterface
{
	GENERATED_BODY()
};

class IGrabbable
{
	GENERATED_BODY()

public:
	// Called when the actor is grabbed by another actor. Provides the SceneComponent this will be attached to.
	virtual void OnGrabbed(USceneComponent* Grabber);

	// Called when the actor is released.
	virtual void OnReleased();
};