// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#include "Util/RaymarchUtils.h"

#include "Containers/UnrealString.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RHIDefinitions.h"
#include "RHI.h"
#include "Rendering/RaymarchTypes.h"
#include "Util/TextureUtilities.h"

#define LOCTEXT_NAMESPACE "RaymarchPlugin"

VRSMOKEVIS_API void URaymarchUtils::MakeDefaultTFTexture(UTexture2D*& OutTexture)
{
	const unsigned SampleCount = 256;

	// Give the texture some height, so it can be inspected in the asset editor.
	const unsigned TextureHeight = 1;

	FFloat16* Samples = new FFloat16[SampleCount * 4 * TextureHeight];
	for (unsigned i = 0; i < SampleCount; i++)
	{
		float Whiteness = static_cast<float>(i) / static_cast<float>(SampleCount - 1);
		Samples[i * 4] = Whiteness;
		Samples[i * 4 + 1] = Whiteness;
		Samples[i * 4 + 2] = Whiteness;
		Samples[i * 4 + 3] = 1;
	}

	for (unsigned i = 1; i < TextureHeight; i++)
	{
		FMemory::Memcpy(Samples + i * SampleCount * 4, Samples, SampleCount * 4 * 2);
	}

	FVolumeTextureToolkit::Create2DTextureTransient(
		OutTexture, PF_FloatRGBA, FIntPoint(SampleCount, TextureHeight), reinterpret_cast<uint8*>(Samples));

	// Don't forget to free the memory here
	delete[] Samples;
}

void URaymarchUtils::ColorCurveToTexture(UCurveLinearColor* Curve, UTexture2D*& OutTexture)
{
	const unsigned sampleCount = 256;

	// Give the texture some height, so it can be inspected in the asset editor. Possibly breaks cache consistency
	const unsigned TextureHeight = 16;

	// Using float16 format because RGBA8 wouldn't be persistent for some reason.
	FFloat16* samples = new FFloat16[sampleCount * 4 * TextureHeight];

	for (unsigned i = 0; i < sampleCount; i++)
	{
		float index = static_cast<float>(i) / (static_cast<float>(sampleCount) - 1);
		FLinearColor picked = Curve->GetLinearColorValue(index);

		samples[i * 4] = picked.R;
		samples[i * 4 + 1] = picked.G;
		samples[i * 4 + 2] = picked.B;
		samples[i * 4 + 3] = picked.A;
	}

	for (unsigned i = 1; i < TextureHeight; i++)
	{
		FMemory::Memcpy(samples + i * sampleCount * 4, samples, sampleCount * 4 * 2);
	}

	FVolumeTextureToolkit::Create2DTextureTransient(
		OutTexture, PF_FloatRGBA, FIntPoint(sampleCount, TextureHeight), (uint8*) samples);

	delete[] samples;	 // Don't forget to free the memory here
}

void URaymarchUtils::CreateBufferTextures(FIntPoint Size, EPixelFormat PixelFormat, OneAxisReadWriteBufferResources& RWBuffers)
{
	if (Size.X == 0 || Size.Y == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Creating Buffer Textures: Size is Zero!"), 3);
		return;
	}
	FRHIResourceCreateInfo CreateInfo(*FString(""), FClearValueBinding::Transparent);
	for (int i = 0; i < 4; i++)
	{
		RWBuffers.Buffers[i] =
			RHICreateTexture2D(Size.X, Size.Y, PixelFormat, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
		RWBuffers.UAVs[i] = RHICreateUnorderedAccessView(RWBuffers.Buffers[i]);
	}
}

void URaymarchUtils::ReleaseOneAxisReadWriteBufferResources(OneAxisReadWriteBufferResources& Buffer)
{
	for (FUnorderedAccessViewRHIRef& UAV : Buffer.UAVs)
	{
		if (UAV)
		{
			UAV.SafeRelease();
		}
		UAV = nullptr;
	}

	for (FTexture2DRHIRef& TextureRef : Buffer.Buffers)
	{
		if (TextureRef)
		{
			TextureRef.SafeRelease();
		}
		TextureRef = nullptr;
	}
}

void URaymarchUtils::GetVolumeTextureDimensions(UVolumeTexture* Texture, FIntVector& Dimensions)
{
	if (Texture)
	{
		// This is slightly retarded...
		Dimensions = FIntVector(Texture->GetSizeX(), Texture->GetSizeY(), Texture->GetSizeZ());
	}
	else
	{
		Dimensions = FIntVector(0, 0, 0);
	}
}

void URaymarchUtils::TransformToMatrix(const FTransform Transform, FMatrix& OutMatrix, bool WithScaling)
{
	if (WithScaling)
	{
		OutMatrix = Transform.ToMatrixWithScale();
	}
	else
	{
		OutMatrix = Transform.ToMatrixNoScale();
	}
}

void URaymarchUtils::LocalToTextureCoords(FVector LocalCoords, FVector& TextureCoords)
{
	TextureCoords = LocalCoords / 2.0f + 0.5f;
}

void URaymarchUtils::TextureToLocalCoords(FVector TextureCoors, FVector& LocalCoords)
{
	LocalCoords = (TextureCoors - 0.5f) * 2.0f;
}

#undef LOCTEXT_NAMESPACE