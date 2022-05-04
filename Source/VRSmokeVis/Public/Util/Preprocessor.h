#pragma once

#include "Preprocessor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPreprocessor, Log, All);


/**
 * Toolkit to automatically preprocess the FDS simulation output. This could either be done by hand by the user using
 * the export_all method of the fdsreader or will be done automatically when dragging an .smv-file into the editor
 * or selecting one inside the game. This first checks if python and required modules (such as the fdsreader) are
 * installed on the system and then starts the preprocessing task in a subprocess.
 */
UCLASS()
class VRSMOKEVIS_API UPreprocessor : public UObject
{
	GENERATED_BODY()

public:
	UPreprocessor();

	/** Start the actual preprocessing in the subprocess and return the path to the generated output files */
	UFUNCTION()
	FString RunFdsreader(const FString InputFile, const FString OutputDir);
	
protected:
	/** Check the python alias on the system (python or python3) and its version */
	UFUNCTION()
	void InitPythonAlias();
	
	/** Gets called when output is generated when testing for the python alias and version */
	void PythonVersionOutput(const FString Output);
	/** Gets called when the python alias test completed and gives the exit code of the subprocess call */
	void PythonVersionCompleted(const int32 ExitCode);

	/** Gets called when output is generated for the actual preprocessing task */
	void FdsreaderOutput(const FString Output);
	/** Gets called when the preprocessing completed and gives the exit code of the subprocess call */
	void FdsreaderCompleted(const int32 ExitCode);
	/** Checks if the python alias is not "python", but "python3" instead */
	void TryPython3();
	
protected:
	/** The python alias that was found */
	UPROPERTY()
	FString PythonAlias;

	/** True when a subprocess is currently running. Will be set to false when the execution finishes */
	UPROPERTY()
	bool SubprocessActive = false;

	/** Saves the output of the subprocesses spawned in the RunFdsreader method to return it after execution finished */
	UPROPERTY()
	FString LastIntermediateOutputFile;
};
