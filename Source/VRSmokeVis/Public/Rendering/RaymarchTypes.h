// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/VolumeTexture.h"
#include "Engine/World.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/MessageLog.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#include "RaymarchMaterialParameters.h"
#include "SceneInterface.h"
#include "SceneUtils.h"
#include "UObject/ObjectMacros.h"
#include "VolumeAsset/VolumeInfo.h"

#include <algorithm>	// std::sort
#include <utility>		// std::pair, std::make_pair
#include <vector>		// std::pair, std::make_pair

#include "RaymarchTypes.generated.h"

class UTextureRenderTargetVolume;

// A structure for 4 switchable read-write buffers. Used for one axis. Need 2 pairs for change-light shader.
struct OneAxisReadWriteBufferResources
{
	// 2D Textures whose dimensions match the matching axis in the volume texture.
	FTexture2DRHIRef Buffers[4];
	// UAV refs to the Buffers, when we need to make a RWTexture out of them.
	FUnorderedAccessViewRHIRef UAVs[4];
};

/** A structure holding all resources related to a single raymarchable volume - its texture ref, the
   TF texture ref and TF Range parameters and read-write buffers used for propagating along all axes. */
USTRUCT(BlueprintType)
struct FBasicRaymarchRenderingResources
{
	GENERATED_BODY()

	/// Flag that these Rendering Resources have been initialized and can be used.
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Basic Raymarch Rendering Resources")
	bool bIsInitialized = false;

	/// Pointer to the Data Volume texture.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Basic Raymarch Rendering Resources")
	UVolumeTexture* DataVolumeTextureRef;

	/// Pointer to the Transfer Function Volume texture.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Basic Raymarch Rendering Resources")
	UTexture2D* TFTextureRef;

	// Read-write buffers for all 3 major axes. Used in compute shaders.
	OneAxisReadWriteBufferResources XYZReadWriteBuffers[3];
};
