#pragma once

#include "Preprocessor.generated.h"

UCLASS()
class VRSMOKEVIS_API UPreprocessor : public UObject
{
	GENERATED_BODY()
	
public:
	UPreprocessor();
	
	UFUNCTION()
	bool InitPythonAlias();
	
	UFUNCTION()
	bool RunFdsreader(const FString InputDir, const FString OutputDir);
	
protected:
	FString PythonAlias;
	
	void PythonVersionOutput(const FString Output);
	void PythonVersionCompleted(const int32 ExitCode);
	
	void FdsreaderOutput(const FString Output);
	void FdsreaderCompleted(const int32 ExitCode);
	void TryPython3();
};
