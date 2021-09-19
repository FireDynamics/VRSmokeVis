

#pragma once

#include "VR/Grabbable.h"
#include "Assets/VolumeAsset.h"
#include "VRSSGameInstance.h"
#include "RaymarchVolume.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRaymarchVolume, Log, All);

UCLASS()
class VRSMOKEVIS_API ARaymarchVolume : public AActor, public IGrabbable
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties*/
	ARaymarchVolume();
	
	/** Called every frame. **/
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(CallInEditor, Category="RaymarchVolume")
	void UseSimulationTransform();
	
protected:
	virtual void BeginPlay() override;

	/** Delegate to update the volume texture after a given amount of time **/
	UFUNCTION()
	void UpdateVolume(const int CurrentTimeStep);

public:
	UPROPERTY(VisibleAnywhere)
	UVRSSGameInstance* GI;
	
	/** MeshComponent that contains the raymarching cube. */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

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
	
	/** The number of steps to take when raymarching. This is multiplied by the volume thickness in texture space. **/
	UPROPERTY(EditAnywhere)
	float RaymarchingSteps = 150;

protected:
	/** The % of time that has passed until the next frame is reached. **/
	UPROPERTY(VisibleAnywhere)
	float TimePassedPercentage = 0;
};