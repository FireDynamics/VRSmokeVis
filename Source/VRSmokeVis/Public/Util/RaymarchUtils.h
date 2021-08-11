// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Rendering/RaymarchTypes.h"

#include "RaymarchUtils.generated.h"

UCLASS()
class URaymarchUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	//
	// Functions for handling transfer functions and color curves follow.
	//

	/** Will create a 1D texture representing a default transfer function. This TF is full opacity,
	going from black at 0 to white at 1.*/
	UFUNCTION(BlueprintCallable, Category = "Raymarcher")
	static VRSMOKEVIS_API void MakeDefaultTFTexture(UTexture2D*& OutTexture);

	/** Will create a 1D texture asset from a ColorCurve. */
	UFUNCTION(BlueprintCallable, Category = "Raymarcher")
	static VRSMOKEVIS_API void ColorCurveToTexture(UCurveLinearColor* Curve, UTexture2D*& OutTexture);

	//
	// Functions for creating parameter collections follow
	//

	/** Gets volume texture dimension. */
	UFUNCTION(BlueprintPure, Category = "Raymarcher")
	static VRSMOKEVIS_API void GetVolumeTextureDimensions(UVolumeTexture* Texture, FIntVector& Dimensions);

	/** Transforms a transform to a matrix. */
	UFUNCTION(BlueprintPure, Category = "Raymarcher")
	static VRSMOKEVIS_API void TransformToMatrix(const FTransform Transform, FMatrix& OutMatrix, bool WithScaling);

	/**
	  Transforms Local (-1 to 1) coords to UV coords (0 to 1) coords. (The values are not clamped to the range).
	*/
	static VRSMOKEVIS_API void LocalToTextureCoords(FVector LocalCoords, FVector& TextureCoords);

	/**
	  Transforms UV coords (0 to 1) to Local (-1 to 1) coords. (The values are not clamped to the range).
	*/
	static VRSMOKEVIS_API void TextureToLocalCoords(FVector TextureCoors, FVector& LocalCoords);

	static VRSMOKEVIS_API void CreateBufferTextures(
		FIntPoint Size, EPixelFormat PixelFormat, OneAxisReadWriteBufferResources& RWBuffers);

	static VRSMOKEVIS_API void ReleaseOneAxisReadWriteBufferResources(OneAxisReadWriteBufferResources& Buffer);
};