#pragma once

#include "FdsActor.h"
#include "Slice.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSlice, Log, All);

/**
 * The actual slice actor that will be spawned in a level which displays a single subslice.
 */
UCLASS()
class VRSMOKEVIS_API ASlice : public AFdsActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ASlice();

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Update the location and rotation of the actor according to where it was located in FDS */
	UFUNCTION()
	void UseSimulationTransform();

	/** Delegate to update the ColorMap in case a simulation with slices with higher Max/lower Min has been added */
	UFUNCTION()
	void UpdateColorMapScale(const float NewMin, const float NewMax) const;

	/** Delegate to update the texture after a given amount of time */
	UFUNCTION()
	void UpdateTexture(const int CurrentTimeStep);

protected:
	virtual void BeginPlay() override;

public:
	/** The base material for slice rendering */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	class UMaterial* SliceMaterialBase;
	
protected:
	UPROPERTY()
	class ASimulation* Sim;

	/** The % of time that has passed until the next frame is reached */
	UPROPERTY(VisibleAnywhere)
	float TimePassedPercentage = 0;

	/** Dynamic material instance for slice rendering */
	UPROPERTY(BlueprintReadOnly, Transient)
	UMaterialInstanceDynamic* SliceMaterial = nullptr;

	/** Current data texture */
	UPROPERTY(BlueprintReadOnly, Transient)
	UTexture2D* DataTextureT0;

	/** Next data texture */
	UPROPERTY(BlueprintReadOnly, Transient)
	UTexture2D* DataTextureT1;
	
	/** MeshComponent that contains the slice plane */
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComponent;
};
