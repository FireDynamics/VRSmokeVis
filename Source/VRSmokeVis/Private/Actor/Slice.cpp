#include "Actor/Slice.h"

#include "VRSSConfig.h"
#include "VRSSGameInstanceSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Util/TextureUtilities.h"
#include "Assets/SliceAsset.h"
#include "Actor/Simulation.h"
#include "Assets/SliceDataInfo.h"

DEFINE_LOG_CATEGORY(LogSlice)

#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif

// Sets default values
ASlice::ASlice() : AFdsActor()
{
	/* Watch out, when changing anything in this function, the child blueprint (BP_Slice) will not reflect the changes
	 * immediately. For the derived blueprint to apply the changes, one has to reparent the blueprint to sth. like
	 * Actor and then reparent to Slice again. After that all default values in the Slice (M_Slice in RootComponent)
	 * and the StaticMesh (SM_Plane) have to be set again. */

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetActorEnableCollision(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	RootComponent->SetWorldScale3D(FVector(1.0f));
	// RootComponent->SetMobility(EComponentMobility::Movable);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Slice Static Mesh"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	StaticMeshComponent->SetCastShadow(false);
	StaticMeshComponent->SetupAttachment(RootComponent);
}

void ASlice::BeginPlay()
{
	Super::BeginPlay();

	Sim = Cast<ASimulation>(GetOwner());

	const UVRSSGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>();

	const USliceDataInfo* SliceDataInfo = Cast<USliceDataInfo>(DataAsset->DataInfo);
	if (SliceMaterialBase)
	{
		SliceMaterial = UMaterialInstanceDynamic::Create(SliceMaterialBase, this, "Slice Mat Dynamic Inst");

		const float CutOffValue = (GI->Config->GetSliceCutOffValue(SliceDataInfo->Quantity) - SliceDataInfo->MinValue) *
			SliceDataInfo->ScaleFactor / 255.f;
		SliceMaterial->SetScalarParameterValue("CutOffValue", CutOffValue);

		SliceMaterial->SetTextureParameterValue("ColorMap", GI->Config->GetColorMap(SliceDataInfo->Quantity));
	}

	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetMaterial(0, SliceMaterial);
	}

	if (DataAsset)
	{
		Sim->InitUpdateRate("Slice", SliceDataInfo->Spacing.W, SliceDataInfo->Dimensions.W);
	}
}

void ASlice::UpdateTexture(const int CurrentTimeStep)
{
	// Load the texture for the next time step to interpolate between the next and current one
	UTexture2D* NextTexture = Cast<UTexture2D>(
		Cast<USliceAsset>(DataAsset)->SliceTextures[(CurrentTimeStep + 1) % Cast<USliceAsset>(DataAsset)->SliceTextures.Num()].GetAsset());

	if (!NextTexture)
	{
		UE_LOG(LogSlice, Error, TEXT("Tried to initialize Slice resources with no data textures!"));
		return;
	}

	if (!NextTexture->GetPlatformData() || NextTexture->GetSizeX() == 0 || NextTexture->GetSizeY() == 0 ||
		NextTexture->GetSizeY() == 0)
	{
		// Happens in cooking stage where per-platform data isn't initialized. Return.
		UE_LOG(LogSlice, Warning,
		       TEXT(
			       "Following is safe to ignore during cooking :\nTried to initialize Slice resources with an unitialized data "
			       "texture with size 0!\nSlice name = %s, Texture name = %s"),
		       *GetName(), *NextTexture->GetName());
		return;
	}

	TimePassedPercentage = 0;
	DataTextureT0 = DataTextureT1;
	DataTextureT1 = NextTexture;

	// Update dynamic material instance
	SliceMaterial->SetTextureParameterValue("TextureT0", DataTextureT0);
	SliceMaterial->SetTextureParameterValue("TextureT1", DataTextureT1);
	SliceMaterial->SetScalarParameterValue("TimePassedPercentage", TimePassedPercentage);
}

void ASlice::UpdateColorMapScale(const float NewMin, const float NewMax) const
{
	const USliceDataInfo* SliceDataInfo = Cast<USliceDataInfo>(DataAsset->DataInfo);
	const FString Quantity = SliceDataInfo->Quantity;
	const float NewRange = NewMax - NewMin;
	const float NewMinScaled = SliceDataInfo->MinValue / NewRange + (SliceDataInfo->MinValue - NewMin) /
		NewRange;

	SliceMaterial->SetScalarParameterValue("ColorMapMin", NewMinScaled);
	SliceMaterial->SetScalarParameterValue("ColorMapRange",
	                                       (SliceDataInfo->MaxValue - SliceDataInfo->MinValue) /
	                                       NewRange);
}

void ASlice::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimePassedPercentage = FMath::Clamp<float>(TimePassedPercentage + DeltaTime / Sim->UpdateRates["Slice"], 0, 1);
}

void ASlice::UseSimulationTransform()
{
	if (DataAsset)
	{
		USliceDataInfo* SliceDataInfo = Cast<USliceDataInfo>(DataAsset->DataInfo);
		SetActorScale3D(FVector{1, 1, 1});
		StaticMeshComponent->SetRelativeLocation(FVector{0, 0, 0});

		// Planes always use XY as dimensions, adjust scale and rotation accordingly
		const bool SwapX = SliceDataInfo->Dimensions.X == 1;
		const bool SwapY = SliceDataInfo->Dimensions.Y == 1;
		if (SwapX)
		{
			SliceDataInfo->WorldDimensions.X = SliceDataInfo->WorldDimensions.Y;
			SliceDataInfo->WorldDimensions.Y = SliceDataInfo->WorldDimensions.Z;
			SliceDataInfo->WorldDimensions.Z = 1;
		}
		else if (SwapY)
		{
			SliceDataInfo->WorldDimensions.Y = SliceDataInfo->WorldDimensions.Z;
			SliceDataInfo->WorldDimensions.Z = 1;
		}

		// Unreal units = cm, FDS has sizes in m. The default plane however is 1x1m already, so no conversion is needed
		StaticMeshComponent->SetRelativeScale3D(SliceDataInfo->WorldDimensions);

		// Correct the dimensions again
		if (SwapX)
		{
			SliceDataInfo->WorldDimensions.Z = SliceDataInfo->WorldDimensions.Y;
			SliceDataInfo->WorldDimensions.Y = SliceDataInfo->WorldDimensions.X;

			// Slices don't have a thickness, we therefore shouldn't account for that when repositioning the slice
			SliceDataInfo->WorldDimensions.X = 0;
			SetActorRelativeLocation((SliceDataInfo->MeshPos + SliceDataInfo->WorldDimensions / 2) * 100);
			SliceDataInfo->WorldDimensions.X = 1;
		}
		else if (SwapY)
		{
			SliceDataInfo->WorldDimensions.Z = SliceDataInfo->WorldDimensions.Y;

			SliceDataInfo->WorldDimensions.Y = 0;
			SetActorRelativeLocation((SliceDataInfo->MeshPos + SliceDataInfo->WorldDimensions / 2) * 100);
			SliceDataInfo->WorldDimensions.Y = 1;
		}
		else
		{
			SliceDataInfo->WorldDimensions.Z = 0;
			SetActorRelativeLocation((SliceDataInfo->MeshPos + SliceDataInfo->WorldDimensions / 2) * 100);
			SliceDataInfo->WorldDimensions.Z = 1;
		}

		// Rotations for corresponding plane: XY=0,0,0 - XZ=90,0,0 - YZ=0,90,0
		if (SwapX) // YZ
		{
			StaticMeshComponent->SetRelativeRotation(FQuat{0.5f, 0.5f, 0.5f, 0.5f});
		}
		else if (SwapY) // XZ
		{
			StaticMeshComponent->SetRelativeRotation(FQuat{0.7071068f, 0, 0, 0.7071068f});
		}
	}
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif
