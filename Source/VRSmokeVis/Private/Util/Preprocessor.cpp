#include "Util/Preprocessor.h"
#include "Misc/MonitoredProcess.h"


DEFINE_LOG_CATEGORY(LogPreprocessor)


UPreprocessor::UPreprocessor()
{
}


void UPreprocessor::PythonVersionOutput(const FString Output)
{
	UE_LOG(LogPreprocessor, Warning, TEXT("Preprocessor PythonVersion Output: %s"), *Output)
	if (Output.Contains(TEXT("Python")) && Output.Contains(TEXT("3.")))
	{
		PythonAlias = "python";
	}
	else
	{
		TryPython3();
	}
}

void UPreprocessor::PythonVersionCompleted(const int32 ExitCode)
{
	if (ExitCode != 0)
	{
		TryPython3();
	}
	SubprocessActive = false;
}

void UPreprocessor::TryPython3()
{
	FMonitoredProcess* Proc = new FMonitoredProcess(TEXT("python3"), TEXT("--version"), true, true);

	Proc->OnOutput().BindLambda([&](const FString Output)
	{
		if (Output.Contains(TEXT("Python")) && Output.Contains(TEXT("3.")))
		{
			PythonAlias = "python3";
		}
		else
		{
			// Todo: Error - Python (3.x) could not be found on system
			UE_LOG(LogPreprocessor, Error, TEXT("Python (3.x) could not be found on system"));
		}
	});
	Proc->OnCompleted().BindLambda([&](const int32 ExitCode)
	{
		if (ExitCode != 0)
		{
			// Todo: Error - Python (3.x) could not be found on system
			UE_LOG(LogPreprocessor, Error, TEXT("Python (3.x) could not be found on system"));
		}
		SubprocessActive = false;
	});

	SubprocessActive = true;

	if (Proc->Launch())
	{
		// Todo: Error - Something went wrong with Subprocess
		UE_LOG(LogPreprocessor, Error, TEXT("Something went wrong with Subprocess"));
	}
}

void UPreprocessor::InitPythonAlias()
{
	// Todo: Let the user set a custom python alias or the path to a specific python interpreter (e.g. in an environment)
	// Create a monitored process instance
	FMonitoredProcess* Proc = new FMonitoredProcess(TEXT("python"), TEXT("--version"), true, true);

	// Bind listeners to output and completion events
	Proc->OnOutput().BindUObject(this, &UPreprocessor::PythonVersionOutput);
	Proc->OnCompleted().BindUObject(this, &UPreprocessor::PythonVersionCompleted);
	SubprocessActive = true;

	// Launch the process
	if (!Proc->Launch())
	{
		// Todo: Error - Something went wrong with Subprocess
		// Todo: Prompt python executable path 
		UE_LOG(LogPreprocessor, Error,
		       TEXT("Something went wrong while launching Subprocess. Python Alias could not be determined"))
	}

	// Wait for process to finish executing
	while (SubprocessActive)
	{
		FPlatformProcess::Sleep(0.1f);
	}
}

// Called when the subprocess receives data
void UPreprocessor::FdsreaderOutput(const FString Output)
{
	UE_LOG(LogPreprocessor, Warning, TEXT("Preprocessor FdsReader Output: %s"), *Output)
	if (Output.Contains(".yaml"))
	{
		LastIntermediateOutputFile = Output;
	}
	else
	{
		// Todo: Check for prerequisites when starting the project instead
		
		// Todo: Error - Something went wrong when running the postprocessing script
		// Make sure "fdsreader", "pathos" and "pyyaml" python-packages are installed

		UE_LOG(LogPreprocessor, Error,
		       TEXT(
			       "Something went wrong when running the postprocessing script\nMake sure \"fdsreader\", \"pathos\" and \"pyyaml\" python-packages are installed"
		       ));
	}
}

// Called when the subprocess completes
void UPreprocessor::FdsreaderCompleted(const int32 ExitCode)
{
	if (ExitCode != 0)
	{
		// Todo: Error - Something went wrong when running the postprocessing script
		UE_LOG(LogPreprocessor, Error, TEXT("Something went wrong when running the postprocessing script"));
	}
	SubprocessActive = false;
}

FString UPreprocessor::RunFdsreader(const FString InputFile, const FString OutputDir)
{
	if (PythonAlias.IsEmpty())
		InitPythonAlias();

	// Build the arguments string
	const FString PythonScript = FPaths::Combine(FPaths::GameSourceDir(),
	                                             TEXT("VRSmokeVis/Python/run_fds_postprocessing.py"));
	const FString Args = FString::Printf(TEXT("%s %s %s"), *PythonScript, *InputFile, *OutputDir);

	// Create a monitored process instance
	FMonitoredProcess* Proc = new FMonitoredProcess(PythonAlias, Args, true, true);

	// Bind listeners to output and completion events
	Proc->OnOutput().BindUObject(this, &UPreprocessor::FdsreaderOutput);
	Proc->OnCompleted().BindUObject(this, &UPreprocessor::FdsreaderCompleted);

	SubprocessActive = true;

	// Launch the process
	if (!Proc->Launch())
	{
		// Todo: Error - Something went wrong with Subprocess
		UE_LOG(LogPreprocessor, Error, TEXT("Something went wrong while launching Subprocess"))
	}

	// Wait for process to finish executing
	while (SubprocessActive)
	{
		FPlatformProcess::Sleep(0.1f);
	}
	return LastIntermediateOutputFile;
}
