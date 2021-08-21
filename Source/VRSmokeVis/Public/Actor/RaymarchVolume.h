

#pragma once

#include "VR/Grabbable.h"
#include "VolumeAsset/VolumeAsset.h"

#include "RaymarchVolume.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRaymarchVolume, Log, All);

UCLASS()
class VRSMOKEVIS_API ARaymarchVolume : public AActor, public IGrabbable
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties*/
	ARaymarchVolume();

	/** Called after the actor is loaded from disk in editor or when spawned in game.
		This is the last action that is performed before BeginPlay.*/
	virtual void PostRegisterAllComponents() override;

	/** MeshComponent that contains the raymarching cube. */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	/** Called every frame. **/
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	/** Delegate to update the volume texture after a given amount of time **/
	UFUNCTION()
	void UpdateVolume(const int CurrentTimeStep);

public:
	/** The loaded Volume asset belonging to this volume. **/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UVolumeAsset* VolumeAsset;

	/** The base material for intensity rendering. **/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UMaterial* RaymarchMaterialBase;

	/** Dynamic material instance for intensity rendering. **/
	UPROPERTY(BlueprintReadOnly, Transient)
	UMaterialInstanceDynamic* RaymarchMaterial = nullptr;

	/** Cube border mesh - this is just a cube with wireframe borders. **/
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CubeBorderMeshComponent = nullptr;

	/** Current data volume texture. **/
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient)
	UVolumeTexture* DataVolumeTextureT0;
	
	/** Next data volume texture. **/
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient)
	UVolumeTexture* DataVolumeTextureT1;
	
	/** The number of steps to take when raymarching. This is multiplied by the volume thickness in texture space, so
	 * can be multiplied by anything from 0 to sqrt(3), Raymarcher will only take exactly this many steps when the path
	 * through the cube is equal to the length of it's side. **/
	UPROPERTY(EditAnywhere)
	float RaymarchingSteps = 150;

protected:
	/** The % of time that has passed  **/
	UPROPERTY(VisibleAnywhere)
	float TimePassedPercentage = 0;
};