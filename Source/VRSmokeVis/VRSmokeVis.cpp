// Fill out your copyright notice in the Description page of Project Settings.

#include "VRSmokeVis.h"

#define LOCTEXT_NAMESPACE "FVolumeTextureToolkitModule"

void FVRSmokeVisModule::StartupModule()
{
	const FString ShaderDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("VRSmokeVis"), TEXT("Shaders"));
	// This creates an alias "Raymarcher" for the folder of our shaders, which can be used when calling
	// IMPLEMENT_GLOBAL_SHADER to find our shaders.
	AddShaderSourceDirectoryMapping(TEXT("/Raymarcher"), ShaderDir);
}

void FVRSmokeVisModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_PRIMARY_GAME_MODULE(FVRSmokeVisModule, VRSmokeVis, "VRSmokeVis");
