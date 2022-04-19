#include "VRSSAssetFactory.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Assets/SimulationAsset.h"
#include "Containers/UnrealString.h"
#include "UObject/SavePackage.h"
#include "Util/AssetCreationUtilities.h"
#include "Util/ImportUtilities.h"

DEFINE_LOG_CATEGORY(LogAssetFactory)


UVRSSAssetFactory::UVRSSAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("smv;")) + NSLOCTEXT("UVRSSAssetFactory", "FormatSmv", ".smv File").ToString());
	Formats.Add(FString(TEXT("yaml;")) + NSLOCTEXT("UVRSSAssetFactory", "FormatYaml", ".yaml File").ToString());

	SupportedClass = USimulationAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	ImportPriority = 1;
}

UObject* UVRSSAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                              const FString& FileName, const TCHAR* Params, FFeedbackContext* Warn,
                                              bool& bOutOperationCanceled)
{
	bOutOperationCanceled = false;

	FString PackagePath, Temp;
	FImportUtils::SplitPath(InParent->GetFullName(), PackagePath, Temp);
	// Chop these 8 characters: "Package "
	return FAssetCreationUtils::CreateSimulation(FileName, PackagePath.RightChop(8));
}