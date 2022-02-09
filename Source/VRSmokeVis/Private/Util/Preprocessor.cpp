#include "Util/Preprocessor.h"
#include "Misc/MonitoredProcess.h"


UPreprocessor::UPreprocessor()
{
	if (!InitPythonAlias())
	{
		// Todo: Error - Something went wrong with Subprocess
	}
}


void UPreprocessor::PythonVersionOutput(const FString Output)
{
     if (Output.Contains(TEXT("Python")) && Output.Contains(TEXT("3.")))
     {
	     PythonAlias = "python";
     } else
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
}

void UPreprocessor::TryPython3()
{
	FMonitoredProcess *Proc = new FMonitoredProcess(TEXT("python3"), TEXT("--version"), true, true);
     
	Proc->OnOutput().BindLambda([&](const FString Output)
	{
		if (Output.Contains(TEXT("Python")) && Output.Contains(TEXT("3.")))
		{
			PythonAlias = "python3";
		} else
		{
			// Todo: Error - Python (3.x) could not be found on system
		}
	});
	Proc->OnCompleted().BindLambda([](const int32 ExitCode)
	{
		if (ExitCode != 0)
		{
			// Todo: Error - Python (3.x) could not be found on system
		}
	});
     
	if(Proc->Launch())
	{
		// Todo: Error - Something went wrong with Subprocess
	}
}

bool UPreprocessor::InitPythonAlias()
{	
	// Create a monitored process instance
	FMonitoredProcess *Proc = new FMonitoredProcess(TEXT("python"), TEXT("--version"), true, true);
     
	// Bind listeners to output and completion events
	Proc->OnOutput().BindUObject(this, &UPreprocessor::PythonVersionOutput);
	Proc->OnCompleted().BindUObject(this, &UPreprocessor::PythonVersionCompleted);
     
	// Launch the process
	return Proc->Launch();
}

// Called when the subprocess receives data
void UPreprocessor::FdsreaderOutput(const FString Output)
{
	if (Output.Equals("Success"))
	{
		
	} else
	{
		// Todo: Error - Something went wrong when running the postprocessing script
		// Make sure fdsreader and yaml python-packages are installed
	}
}

// Called when the subprocess completes
void UPreprocessor::FdsreaderCompleted(const int32 ExitCode)
{
     if (ExitCode != 0)
     {
	     // Todo: Error - Something went wrong when running the postprocessing script
     }
}

bool UPreprocessor::RunFdsreader(const FString InputDir, const FString OutputDir)
{
	// Build the arguments string
	const FString PythonScript = FPaths::Combine(FPaths::GameSourceDir(), TEXT("VRSmokeVis/Python/run_fds_postprocessing.py"));
	const FString Args = FString::Printf(TEXT("%s %s %s"), *PythonScript, *InputDir, *OutputDir);
	
	// Create a monitored process instance
	FMonitoredProcess *Proc = new FMonitoredProcess(PythonAlias, Args, true, true);
     
	// Bind listeners to output and completion events
	Proc->OnOutput().BindUObject(this, &UPreprocessor::FdsreaderOutput);
	Proc->OnCompleted().BindUObject(this, &UPreprocessor::FdsreaderCompleted);
     
	// Launch the process
	return Proc->Launch();
}