// Copyright 2021 Tomas Bartipan and Technical University of Munich.
// Licensed under MIT license - See License.txt for details.
// Special credits go to : Temaran (compute shader tutorial), TheHugeManatee (original concept, supervision) and Ryan Brucks
// (original raymarching code).

#include "Actor/RaymarchVolume.h"

// #include "GenericPlatform/GenericPlatformTime.h"
#include "Rendering/RaymarchMaterialParameters.h"
#include "Util/TextureUtilities.h"
#include "Util/RaymarchUtils.h"
#include "VolumeAsset/VolumeAsset.h"
#include "TimerManager.h"
#include <Curves/CurveLinearColor.h>

DEFINE_LOG_CATEGORY(LogRaymarchVolume)

#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif

// Sets default values
ARaymarchVolume::ARaymarchVolume() : AActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetActorEnableCollision(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	RootComponent->SetWorldScale3D(FVector(1.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitCubeInsideOut(
		TEXT("StaticMesh'/Game/Meshes/Unit_Cube_Inside_Out.Unit_Cube_Inside_Out'"));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Raymarch Cube Static Mesh"));
	/// Set basic unit cube properties.
	if (UnitCubeInsideOut.Succeeded())
	{
		StaticMeshComponent->SetStaticMesh(UnitCubeInsideOut.Object);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMeshComponent->SetRelativeScale3D(FVector(100.0f));
		StaticMeshComponent->SetupAttachment(RootComponent);
	}

	// Create CubeBorderMeshComponent and find and assign cube border mesh (that's a cube with only edges visible).
	CubeBorderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Raymarch Volume Cube Border"));
	CubeBorderMeshComponent->SetupAttachment(StaticMeshComponent);
	CubeBorderMeshComponent->SetRelativeScale3D(FVector(1.01));
	CubeBorderMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeBorder(TEXT("StaticMesh'/Game/Meshes/Unit_Cube.Unit_Cube'"));

	if (CubeBorder.Succeeded())
	{
		// Find and assign cube material.
		CubeBorderMeshComponent->SetStaticMesh(CubeBorder.Object);
		static ConstructorHelpers::FObjectFinder<UMaterial> BorderMaterial(
			TEXT("Material'/Game/Materials/M_CubeBorder.M_CubeBorder'"));
		if (BorderMaterial.Succeeded())
		{
			CubeBorderMeshComponent->SetMaterial(0, BorderMaterial.Object);
		}
	}

	// Find and assign default raymarch materials.
	static ConstructorHelpers::FObjectFinder<UMaterial> IntensityMaterial(
		TEXT("Material'/Game/Materials/M_Raymarch.M_Raymarch'"));

	if (IntensityMaterial.Succeeded())
	{
		RaymarchMaterialBase = IntensityMaterial.Object;
	}

	// Set default values for steps and half-res.
	RaymarchingSteps = 150;
}

// Called after registering all components. This is the last action performed before editor window is spawned and before BeginPlay.
void ARaymarchVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		// Do not perform this on default class objects or archetype objects
		return;
	}

	if (RaymarchResources.bIsInitialized)
	{
		// Do not perform this if this object already is initialized
		// PostRegisterAllComponents also gets called in every OnPropertyChanged call, so
		// we want to ignore every call to this except the first one.
		return;
	}

	if (RaymarchMaterialBase)
	{
		RaymarchMaterial =
			UMaterialInstanceDynamic::Create(RaymarchMaterialBase, this, "Intensity Raymarch Mat Dynamic Inst");

		RaymarchMaterial->SetScalarParameterValue(RaymarchParams::Steps, RaymarchingSteps);
	}

	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetMaterial(0, RaymarchMaterial);
	}

	if (VolumeAsset)
	{
		SetVolumeAsset(VolumeAsset);
	}
}

void ARaymarchVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

#if WITH_EDITOR

void ARaymarchVolume::OnVolumeAssetChangedTF(UCurveLinearColor* Curve)
{
	// Remove OnUpdateGradient delegate from old curve (if it exists)
	bool bChangedCurve = false;
	if (Curve != CurrentTFCurve)
	{
		bChangedCurve = true;
		CurrentTFCurve->OnUpdateCurve.Remove(CurveGradientUpdateDelegateHandle);
	}

	SetTFCurve(Curve);

	// Add gradient update delegate to new curve.
	if (bChangedCurve)
	{
		CurrentTFCurve->OnUpdateCurve.AddUObject(this, &ARaymarchVolume::OnTFColorCurveUpdated);
	}
}

void ARaymarchVolume::OnTFColorCurveUpdated(UCurveBase* Curve, EPropertyChangeType::Type ChangeType)
{
	SetTFCurve(Cast<UCurveLinearColor>(Curve));
}

void ARaymarchVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	//	RaymarchResources.PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ARaymarchVolume, VolumeAsset))
	{
		SetVolumeAsset(VolumeAsset);
		return;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ARaymarchVolume, RaymarchingSteps))
	{
		if (RaymarchResources.bIsInitialized)
		{
			// Set default values for the raymarcher.
			RaymarchMaterial->SetScalarParameterValue(RaymarchParams::Steps, RaymarchingSteps);
		}
	}
}

bool ARaymarchVolume::ShouldTickIfViewportsOnly() const
{
	return true;
}

#endif	  //#if WITH_EDITOR

// Called every frame
void ARaymarchVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Uncomment to see logs of potentially weird ticking behavior in-editor when dragging sliders in VolumeInfo.
	//
	// 	static int TickFrame = 0;
	//
	// 	FString Log = "Tick :\nInitialized = " + FString::FromInt(RaymarchResources.bIsInitialized) +
	// 				  "\nRecompute = " + FString::FromInt(bRequestedRecompute) + "\nDeltaTime = " +
	// FString::SanitizeFloat(DeltaTime)
	// +
	// 				  "\nTick number = " + FString::FromInt(TickFrame++);
	//
	// 	GEngine->AddOnScreenDebugMessage(0, 0, FColor::Yellow, Log);
}

void ARaymarchVolume::UpdateVolume()
{
	++CurrentTimeStep;

	if (CurrentTimeStep >= VolumeAsset->ImageInfo.Dimensions.W)
	{
		CurrentTimeStep = 0;
	}
	
	InitializeRaymarchResources(VolumeAsset->DataTextures[CurrentTimeStep]);

	if (!RaymarchResources.bIsInitialized)
	{
		UE_LOG(LogRaymarchVolume, Warning,
		       TEXT("Could not initialize raymarching resources when trying to update volume!"), 3);
		return;
	}

	FlushRenderingCommands();

	// Update dynamic material instance
	SetMaterialVolumeParameters();
}

bool ARaymarchVolume::SetVolumeAsset(UVolumeAsset* InVolumeAsset)
{
	if (!InVolumeAsset)
	{
		return false;
	}

#if WITH_EDITOR
	if (!GetWorld() || !GetWorld()->IsGameWorld())
	{
		if (InVolumeAsset != OldVolumeAsset)
		{
			// If we're in editor and we already have an asset loaded before, unbind the delegate
			// from it's color curve change broadcast and also the OnCurve and OnVolumeInfo changed broadcasts.
			if (OldVolumeAsset)
			{
				OldVolumeAsset->TransferFuncCurve->OnUpdateCurve.Remove(CurveGradientUpdateDelegateHandle);
				OldVolumeAsset->OnCurveChanged.Remove(CurveChangedInVolumeDelegateHandle);
			}
			if (InVolumeAsset)
			{
				CurveChangedInVolumeDelegateHandle =
					InVolumeAsset->OnCurveChanged.AddUObject(this, &ARaymarchVolume::OnVolumeAssetChangedTF);
			}
		}
	}
#endif

	// Create the transfer function BEFORE calling initialize resources and assign TF texture AFTER initializing!
	// Initialize resources calls FlushRenderingCommands(), so it ensures the TF is useable by the time we bind it.

	// Generate texture for transfer function from curve or make default (if none provided).
	if (InVolumeAsset->TransferFuncCurve)
	{
		CurrentTFCurve = InVolumeAsset->TransferFuncCurve;
		URaymarchUtils::ColorCurveToTexture(CurrentTFCurve, RaymarchResources.TFTextureRef);

#if WITH_EDITOR
		// Bind a listener to the delegate notifying about color curve changes
		if ((!GetWorld() || !GetWorld()->IsGameWorld()) && InVolumeAsset != OldVolumeAsset)
		{
			CurveGradientUpdateDelegateHandle =
				CurrentTFCurve->OnUpdateCurve.AddUObject(this, &ARaymarchVolume::OnTFColorCurveUpdated);
		}
#endif
	}
	else
	{
		// Create default black-to-white texture if the VolumeAsset doesn't have one.
		URaymarchUtils::MakeDefaultTFTexture(RaymarchResources.TFTextureRef);
	}

	VolumeAsset = InVolumeAsset;
	OldVolumeAsset = InVolumeAsset;

	// Set timer to set next frame (update next volume) after some time
	GetWorldTimerManager().SetTimer(UpdateTimerHandle, this, &ARaymarchVolume::UpdateVolume, .3f, true);

	InitializeRaymarchResources(VolumeAsset->DataTextures[CurrentTimeStep]);

	if (!RaymarchResources.bIsInitialized)
	{
		UE_LOG(LogRaymarchVolume, Warning, TEXT("Could not initialize raymarching resources!"), 3);
		return false;
	}

	FlushRenderingCommands();

	// Set TF Texture in the material
	if (RaymarchMaterial)
	{
		RaymarchMaterial->SetTextureParameterValue(RaymarchParams::TransferFunction, RaymarchResources.TFTextureRef);
	}

	// Unreal units = cm, FDS has sizes in m -> multiply by 10.
	StaticMeshComponent->SetRelativeScale3D(InVolumeAsset->ImageInfo.WorldDimensions * 100);

	// Update world and set all parameters.
	SetMaterialVolumeParameters();

	// Notify listeners that we've loaded a new volume.
	bool _ = OnVolumeLoaded.ExecuteIfBound();
	return true;
}

void ARaymarchVolume::SetTFCurve(UCurveLinearColor* InTFCurve)
{
	if (InTFCurve)
	{
		CurrentTFCurve = InTFCurve;
		URaymarchUtils::ColorCurveToTexture(CurrentTFCurve, RaymarchResources.TFTextureRef);

		FlushRenderingCommands();
		// Set TF Texture in the lit material.
		RaymarchMaterial->SetTextureParameterValue(RaymarchParams::TransferFunction, RaymarchResources.TFTextureRef);
	}
}

void ARaymarchVolume::SaveCurrentParamsToVolumeAsset()
{
	if (VolumeAsset)
	{
		VolumeAsset->TransferFuncCurve = CurrentTFCurve;
		UPackage* Package = VolumeAsset->GetOutermost();

		UPackage::SavePackage(Package, VolumeAsset, RF_Public | RF_Standalone,
		                      *Package->GetLoadedPath().GetLocalBaseFilenameWithPath(), GError, nullptr, false, true,
		                      SAVE_NoError);
	}
}

void ARaymarchVolume::SetMaterialVolumeParameters()
{
	if (RaymarchMaterial)
	{
		RaymarchMaterial->SetTextureParameterValue(RaymarchParams::DataVolume, RaymarchResources.DataVolumeTextureRef);
	}
}

void ARaymarchVolume::GetMinMaxValues(float& Min, float& Max)
{
	Min = VolumeAsset->ImageInfo.MinValue;
	Max = VolumeAsset->ImageInfo.MaxValue;
}

void ARaymarchVolume::SetRaymarchSteps(float InRaymarchingSteps)
{
	RaymarchingSteps = InRaymarchingSteps;
	if (RaymarchMaterial)
	{
		RaymarchMaterial->SetScalarParameterValue(RaymarchParams::Steps, RaymarchingSteps);
	}
}

void ARaymarchVolume::InitializeRaymarchResources(UVolumeTexture* VolumeTexture)
{
	if (RaymarchResources.bIsInitialized)
	{
		FreeRaymarchResources();
	}

	if (!VolumeTexture)
	{
		UE_LOG(LogRaymarchVolume, Error, TEXT("Tried to initialize Raymarch resources with no data volume!"));
		return;
	}

	if (!VolumeTexture->PlatformData || VolumeTexture->GetSizeX() == 0 || VolumeTexture->GetSizeY() == 0 ||
		VolumeTexture->GetSizeY() == 0)
	{
		// Happens in cooking stage where per-platform data isn't initalized. Return.
		UE_LOG(LogRaymarchVolume, Warning,
		       TEXT(
			       "Following is safe to ignore during cooking :\nTried to initialize Raymarch resources with an unitialized data "
			       "volume with size 0!\nRaymarch volume name = %s, VolumeTexture name = %s"),
		       *GetName(), *VolumeTexture->GetName());
		return;
	}

	RaymarchResources.DataVolumeTextureRef = VolumeTexture;

	RaymarchResources.bIsInitialized = true;
}

void ARaymarchVolume::FreeRaymarchResources()
{
	RaymarchResources.DataVolumeTextureRef = nullptr;

	for (OneAxisReadWriteBufferResources& Buffer : RaymarchResources.XYZReadWriteBuffers)
	{
		URaymarchUtils::ReleaseOneAxisReadWriteBufferResources(Buffer);
	}

	RaymarchResources.bIsInitialized = false;
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif
