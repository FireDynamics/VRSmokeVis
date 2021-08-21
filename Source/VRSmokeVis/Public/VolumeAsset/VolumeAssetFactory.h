

#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "VolumeAssetFactory.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogAssetFactory, Log, All);

/**
 * Implements a factory for creating volume texture assets by drag'n'dropping .yaml files into the content browser.
 */
UCLASS(hidecategories = Object)
class UVolumeAssetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	//~ UFactory Interface
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
};
