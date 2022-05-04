#pragma once

#include "VRSSGameInstanceSubsystem.generated.h"

/**
 * Singleton which is responsible for connecting all ASimulation instances (in case there are more than one).
 */
UCLASS()
class VRSMOKEVIS_API UVRSSGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UVRSSGameInstanceSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION()
	void RegisterSimulation(class ASimulation* Simulation);

	UFUNCTION()
	void ToggleHUDVisibility() const;

	UFUNCTION()
	void OnActiveAssetsChanged() const;
	
	UFUNCTION(BlueprintCallable)
	void ChangeObstQuantity(FString& NewQuantity);
	
protected:
public:
	/** An instance of the configuration for the project which simply uses its default values set in the editor. */
	UPROPERTY(EditAnywhere)
	class UVRSSConfig* Config;

protected:
	UPROPERTY()
	TArray<class ASimulation*> Simulations;
};
