#pragma once
#include "FdsActor.h"

#include "RaymarchVolume.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRaymarchVolume, Log, All);


/**
 * The actual volume actor that will be spawned in a level which displays a smoke volume (variable quantity, e.g. soot
 * density or temperature) inside a single FDS mesh.
 */
UCLASS()
class VRSMOKEVIS_API ARaymarchVolume : public AFdsActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties*/
	ARaymarchVolume();

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Update the location and rotation of the actor according to where it was located in FDS */
	UFUNCTION()
	void UseSimulationTransform();

	/** Delegate to update the volume texture after a given amount of time */
	UFUNCTION()
	void UpdateVolume(const int CurrentTimeStep);

protected:
	virtual void BeginPlay() override;

public:
	/** The base material for intensity rendering */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UMaterial* RaymarchMaterialBase;

	UPROPERTY(EditAnywhere)
	UMaterial* BorderMaterial;

protected:
	UPROPERTY()
	class ASimulation* Sim;

	UPROPERTY()
	UStaticMesh* CubeBorder;

	/** The % of time that has passed until the next frame is reached */
	UPROPERTY(VisibleAnywhere)
	float TimePassedPercentage = 0;

	/** Current data volume texture */
	UPROPERTY(BlueprintReadOnly, Transient)
	UVolumeTexture* DataVolumeTextureT0;

	/** Next data volume texture */
	UPROPERTY(BlueprintReadOnly, Transient)
	UVolumeTexture* DataVolumeTextureT1;

	/** Dynamic material instance for intensity rendering */
	UPROPERTY(BlueprintReadOnly, Transient)
	UMaterialInstanceDynamic* RaymarchMaterial;

	/** Cube border mesh - this is just a cube with wireframe borders */
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* CubeBorderMeshComponent;

	/** MeshComponent that contains the raymarching cube */
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComponent;
};
