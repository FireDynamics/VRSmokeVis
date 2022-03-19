#pragma once

#include "Grabbable.generated.h"

UINTERFACE(Blueprintable)
class VRSMOKEVIS_API UGrabbable : public UInterface
{
	GENERATED_BODY()
};

class VRSMOKEVIS_API IGrabbable
{
	GENERATED_BODY()

public:
	// Called when the actor is grabbed by another actor. Provides the SceneComponent this will be attached to.
	virtual void OnGrabbed(USceneComponent* Grabber);

	// Called when the actor is released.
	virtual void OnReleased();
};
