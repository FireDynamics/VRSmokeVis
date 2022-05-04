#pragma once

#include "FdsActor.h"
#include "Obst.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogObst, Log, All);


/**
 * The actual obst actor that will be spawned in a level which displays a whole obstruction (even across multiple FDS
 * meshes).
 */
UCLASS()
class VRSMOKEVIS_API AObst : public AFdsActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AObst();

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Update the location and rotation of the actor according to where it was located in FDS */
	UFUNCTION()
	void UseSimulationTransform();

	/** Delegate to update the ColorMap in case a simulation with obsts with higher Max/lower Min has been added */
	UFUNCTION()
	void UpdateColorMapScale(const float NewMin, const float NewMax) const;

	UFUNCTION()
	void SetActiveQuantity(FString GlobalObstQuantity);

	/** Delegate to update the texture after a given amount of time */
	UFUNCTION()
	void UpdateTexture(const int CurrentTimeStep, const int Orientation);

protected:
	virtual void BeginPlay() override;

public:
	/** The base material for obst data */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UMaterial* ObstDataMaterialBase;

	UPROPERTY(EditAnywhere)
	UMaterial* BorderMaterial;
	
protected:
	UPROPERTY(VisibleAnywhere, Transient)
	FString ActiveQuantity;

	UPROPERTY()
	class ASimulation* Sim;

	/** The % of time that has passed until the next frame is reached */
	UPROPERTY(VisibleAnywhere)
	float TimePassedPercentage = 0;

	/** Current data texture */
	UPROPERTY(BlueprintReadOnly, Transient)
	TMap<int, UTexture2D*> DataTexturesT0;

	/** Next data texture */
	UPROPERTY(BlueprintReadOnly, Transient)
	TMap<int, UTexture2D*> DataTexturesT1;

	/** Dynamic material instance for obst data */
	UPROPERTY(BlueprintReadOnly, Transient)
	TMap<int, UMaterialInstanceDynamic*> ObstDataMaterials;

	/** Cube border mesh - this is just a cube with wireframe borders */
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* CubeBorderMeshComponent;
	
	/** MeshComponent that contains the cube mesh */
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComponent;
};
