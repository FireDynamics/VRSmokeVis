#include "VRSSAssetFactory.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Assets/SimulationAsset.h"
#include "Containers/UnrealString.h"
#include "UObject/SavePackage.h"
#include "Util/AssetUtilities.h"
#include "Util/ImportUtilities.h"
#include "Util/Preprocessor.h"

DEFINE_LOG_CATEGORY(LogAssetFactory)


UVRSSAssetFactory::UVRSSAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("smv;")) + NSLOCTEXT("UVRSSAssetFactory", "FormatSmv", ".smv File").ToString());
	Formats.Add(FString(TEXT("yaml;")) + NSLOCTEXT("UVRSSAssetFactory", "FormatYaml", ".yaml File").ToString());

	// Todo: Is this enough, even though we are also using other classes?
	SupportedClass = USimulationAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	ImportPriority = 1;
}

UObject* UVRSSAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                              const FString& FileName, const TCHAR* Params, FFeedbackContext* Warn,
                                              bool& bOutOperationCanceled)
{
	FString SimulationIntermediateFile;
	if (FileName.Contains(".smv"))
	{
		FString SmokeViewDir, Temp;
		FImportUtils::SplitPath(FileName, SmokeViewDir, Temp);
		const FString OutputDir = FPaths::Combine(SmokeViewDir, TEXT("SmokeVisIntermediate"));

		// First check if there already is intermediate data from a previous run
		if (FPaths::DirectoryExists(OutputDir))
		{
			TArray<FString> FoundFiles;
			IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
			FileManager.FindFiles(FoundFiles, *OutputDir, TEXT(".yaml"));
			for (const FString& File : FoundFiles)
			{
				if (File.Contains("-smv.yaml"))
				{
					SimulationIntermediateFile = File;
					break;
				}
			}
		}
		if (SimulationIntermediateFile.IsEmpty())
		{
			FImportUtils::VerifyOrCreateDirectory(OutputDir);
			UPreprocessor *PythonPreprocessor = NewObject<UPreprocessor>();
			SimulationIntermediateFile = PythonPreprocessor->RunFdsreader(FileName, OutputDir);
		}
	} else
	{
		SimulationIntermediateFile = FileName;
	}
	
	bOutOperationCanceled = false;
	return FAssetUtils::CreateSimulation(InParent, SimulationIntermediateFile);
}