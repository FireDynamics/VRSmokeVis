

#pragma once

#include "Assets/ObstAsset.h"
#include "VRSSGameInstance.h"
#include "Obst.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogObst, Log, All);

UCLASS()
class VRSMOKEVIS_API AObst : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	AObst();
	
	/** Called every frame. **/
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(CallInEditor, Category="Obst")
	void UseSimulationTransform();
	
	/** Delegate to update the ColorMap in case a obst with higher Max/lower Min has been added. **/
	UFUNCTION()
	void UpdateColorMapScale(const FString Quantity, const float NewMin, const float NewMax) const;

protected:
	virtual void BeginPlay() override;

	/** Delegate to update the texture after a given amount of time. **/
	UFUNCTION()
	void UpdateTexture(const int CurrentTimeStep, const int Orientation);

	UFUNCTION(BlueprintSetter)
	void SetActiveQuantity(FString NewQuantity);
	
public:
	UPROPERTY(VisibleAnywhere)
	UVRSSGameInstance* GI;

	/** The base material for obst rendering. **/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UMaterial* ObstMaterialBase;
	
	/** The loaded obst asset belonging to this obst. **/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UObstAsset* ObstAsset;

	UPROPERTY(BlueprintSetter=SetActiveQuantity, VisibleAnywhere, Transient)
	FString ActiveQuantity;
	
protected:
	/** The % of time that has passed until the next frame is reached. **/
	UPROPERTY(VisibleAnywhere)
	float TimePassedPercentage = 0;
	
	/** Current data texture. **/
	UPROPERTY(BlueprintReadOnly, Transient)
	TMap<int, UTexture2D*> DataTexturesT0;
	
	/** Next data texture. **/
	UPROPERTY(BlueprintReadOnly, Transient)
	TMap<int, UTexture2D*> DataTexturesT1;
	
	/** Dynamic material instance for obst rendering. **/
	UPROPERTY(BlueprintReadOnly, Transient)
	TMap<int, UMaterialInstanceDynamic*> ObstMaterials;
	
	/** MeshComponent that contains the cube mesh. */
	UPROPERTY(BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;
};