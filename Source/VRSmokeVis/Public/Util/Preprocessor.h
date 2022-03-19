#pragma once

#include "Preprocessor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPreprocessor, Log, All);


UCLASS()
class VRSMOKEVIS_API UPreprocessor : public UObject
{
	GENERATED_BODY()

public:
	UPreprocessor();

	UFUNCTION()
	FString RunFdsreader(const FString InputFile, const FString OutputDir);
	
protected:
	UFUNCTION()
	void InitPythonAlias();
	
	void PythonVersionOutput(const FString Output);
	void PythonVersionCompleted(const int32 ExitCode);

	void FdsreaderOutput(const FString Output);
	void FdsreaderCompleted(const int32 ExitCode);
	void TryPython3();
	
protected:
	UPROPERTY()
	FString PythonAlias;

	UPROPERTY()
	bool SubprocessActive = false;

	UPROPERTY()
	FString LastIntermediateOutputFile;
};
