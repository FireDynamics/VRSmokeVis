// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRUI.generated.h"

UCLASS()
class VRSMOKEVIS_API AVRUI : public AActor
{
	GENERATED_BODY()

public:
	AVRUI();

	UFUNCTION(BlueprintCallable)
	static inline FString KeyToStringBP(UPARAM(ref) FKey& Key)
	{
		return Key.ToString();
	}
	
	virtual void Tick(float DeltaTime) override;
	
	void Init(class UUserInterfaceUserWidget* UserInterfaceUserWidget);

	UFUNCTION(BlueprintImplementableEvent)
	void CloseMenu();
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool ActiveMenuHandRight;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UWidgetComponent* WidgetComponent;
};
