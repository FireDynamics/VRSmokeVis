#include "Actor/VR/Grabbable.h"

void IGrabbable::OnGrabbed(USceneComponent* Grabber)
{
	AActor* ThisAsActor = Cast<AActor>(this);
	if (ThisAsActor)
	{
		ThisAsActor->AttachToComponent(Grabber, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void IGrabbable::OnReleased()
{
	AActor* ThisAsActor = Cast<AActor>(this);
	if (ThisAsActor)
	{
		ThisAsActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}
