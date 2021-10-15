

#pragma once

#include "Assets/SliceAsset.h"
#include "VRSSGameInstance.h"
#include "Slice.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSlice, Log, All);

UCLASS()
class VRSMOKEVIS_API ASlice : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	ASlice();
	
	/** Called every frame. **/
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(CallInEditor, Category="Slice")
	void UseSimulationTransform();
	
protected:
	virtual void BeginPlay() override;

	/** Delegate to update the texture after a given amount of time. **/
	UFUNCTION()
	void UpdateTexture(const int CurrentTimeStep);

	/** Delegate to update the ColorMap in case a slice with higher Max/lower Min has been added. **/
	UFUNCTION()
	void UpdateColorMapScale(const FString Quantity, const float NewMin, const float NewMax) const;

public:
	UPROPERTY(VisibleAnywhere)
	UVRSSGameInstance* GI;
	
	/** MeshComponent that contains the raymarching cube. */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	/** The base material for slice rendering. **/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UMaterial* SliceMaterialBase;

	/** Dynamic material instance for slice rendering. **/
	UPROPERTY(BlueprintReadOnly, Transient)
	UMaterialInstanceDynamic* SliceMaterial = nullptr;
	
	/** The loaded slice asset belonging to this slice. **/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	USliceAsset* SliceAsset;

	/** Current data texture. **/
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient)
	UTexture2D* DataTextureT0;
	
	/** Next data texture. **/
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient)
	UTexture2D* DataTextureT1;

protected:
	/** The % of time that has passed until the next frame is reached. **/
	UPROPERTY(VisibleAnywhere)
	float TimePassedPercentage = 0;
};