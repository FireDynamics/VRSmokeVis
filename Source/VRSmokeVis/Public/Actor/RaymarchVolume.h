

#pragma once

#include "RaymarchVolume.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRaymarchVolume, Log, All);

UCLASS()
class VRSMOKEVIS_API ARaymarchVolume : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties*/
	ARaymarchVolume();
	
	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(CallInEditor, Category="RaymarchVolume")
	void UseSimulationTransform();

	/** Delegate to update the volume texture after a given amount of time */
	UFUNCTION()
	void UpdateVolume(const int CurrentTimeStep);
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere)
	class ASimulation* Sim;

	/** The loaded Volume asset belonging to this volume. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	class UVolumeAsset* VolumeAsset;

	/** The base material for intensity rendering. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UMaterial* RaymarchMaterialBase;
	
	/** The number of steps to take when raymarching. This is multiplied by the volume thickness in texture space. */
	UPROPERTY(EditAnywhere)
	float RaymarchingSteps = 150;

	UPROPERTY(EditAnywhere)
	UStaticMesh* UnitCubeInsideOut;
	
	UPROPERTY(EditAnywhere)
	UStaticMesh* CubeBorder;
	
	UPROPERTY(EditAnywhere)
	UMaterial* BorderMaterial;

protected:
	/** The % of time that has passed until the next frame is reached. */
	UPROPERTY(VisibleAnywhere)
	float TimePassedPercentage = 0;

	/** Current data volume texture. */
	UPROPERTY(BlueprintReadOnly, Transient)
	UVolumeTexture* DataVolumeTextureT0;
	
	/** Next data volume texture. */
	UPROPERTY(BlueprintReadOnly, Transient)
	UVolumeTexture* DataVolumeTextureT1;

	/** Dynamic material instance for intensity rendering. */
	UPROPERTY(BlueprintReadOnly, Transient)
	UMaterialInstanceDynamic* RaymarchMaterial;
	
	/** Cube border mesh - this is just a cube with wireframe borders. */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CubeBorderMeshComponent;
	
	/** MeshComponent that contains the raymarching cube. */
	UPROPERTY(BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;
};