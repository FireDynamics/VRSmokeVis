#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "VRSSAssetFactory.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogAssetFactory, Log, All);

/**
 * Implements a factory for creating (volume) texture assets by drag'n'dropping .yaml files into the content browser.
 */
UCLASS(hidecategories = Object)
class UVRSSAssetFactory : public UFactory
{
	GENERATED_BODY()

	UVRSSAssetFactory(const FObjectInitializer& ObjectInitializer);

protected:
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	                                   const FString& FileName, const TCHAR* Params, FFeedbackContext* Warn,
	                                   bool& bOutOperationCanceled) override;
};