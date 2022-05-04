#include "Actor/RaymarchVolume.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Util/TextureUtilities.h"
#include "Assets/VolumeAsset.h"
#include "Actor/Simulation.h"
#include "Assets/FdsDataAsset.h"
#include "Assets/VolumeDataInfo.h"

DEFINE_LOG_CATEGORY(LogRaymarchVolume)

#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif

// Sets default values
ARaymarchVolume::ARaymarchVolume() : AFdsActor()
{
	/* Watch out, when changing anything in this function, the child blueprint (BP_RaymarchVolume) will not reflect the
	 * changes immediately. For the derived blueprint to apply the changes, one has to reparent the blueprint to sth.
	 * like Actor and then reparent to RaymarchVolume again. After that all default values in the RaymarchVolume
	* (M_Raymarch and M_CubeBorder in RootComponent) and the StaticMeshes (SM_UnitCubeInsideOut in StaticMeshComponent
	* and SM_UnitCube in CubeBorderMeshComponent) have to be set again. */
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetActorEnableCollision(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	RootComponent->SetWorldScale3D(FVector(1.0f));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Raymarch Cube Static Mesh"));
	// Set basic unit cube properties
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	StaticMeshComponent->SetRelativeScale3D(FVector(100.0f));
	StaticMeshComponent->SetCastShadow(false);
	StaticMeshComponent->SetupAttachment(RootComponent);

	// Create CubeBorderMeshComponent and assign cube border mesh (that's a cube with only edges visible)
	CubeBorderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Raymarch Volume Cube Border"));
	CubeBorderMeshComponent->SetupAttachment(StaticMeshComponent);
	CubeBorderMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	CubeBorderMeshComponent->SetHiddenInGame(true);
	CubeBorderMeshComponent->SetMaterial(0, BorderMaterial);
}

void ARaymarchVolume::BeginPlay()
{
	Super::BeginPlay();

	check(DataAsset)

	Sim = Cast<ASimulation>(GetOwner());

	if (RaymarchMaterialBase)
	{
		RaymarchMaterial =
			UMaterialInstanceDynamic::Create(RaymarchMaterialBase, this, "Intensity Raymarch Mat Dynamic Inst");

		RaymarchMaterial->SetScalarParameterValue("Steps", RaymarchingSteps);
		RaymarchMaterial->SetScalarParameterValue("JitterRadius", Sim->JitterRadius);
	}

	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetMaterial(0, RaymarchMaterial);
	}

	const UVolumeDataInfo *VolumeDataInfo = Cast<UVolumeDataInfo>(DataAsset->DataInfo);
	
	// Unreal units = cm, FDS has sizes in m -> multiply by 100.
	StaticMeshComponent->SetRelativeScale3D(VolumeDataInfo->WorldDimensions * 100);

	Sim->InitUpdateRate("Volume", VolumeDataInfo->Spacing.W, VolumeDataInfo->Dimensions.W);
}

void ARaymarchVolume::UpdateVolume(const int CurrentTimeStep)
{
	// Load the texture for the next time step to interpolate between the next and current one
	UVolumeTexture* NextTexture = Cast<UVolumeTexture>(
		Cast<UVolumeAsset>(DataAsset)->VolumeTextures[(CurrentTimeStep + 1) % Cast<UVolumeAsset>(DataAsset)->VolumeTextures.Num()].GetAsset());

	if (!NextTexture)
	{
		UE_LOG(LogRaymarchVolume, Error, TEXT("Tried to initialize Raymarch resources with no data volume!"));
		return;
	}

	if (!NextTexture->PlatformData || NextTexture->GetSizeX() == 0 || NextTexture->GetSizeY() == 0 ||
		NextTexture->GetSizeY() == 0)
	{
		// Happens in cooking stage where per-platform data isn't initialized. Return.
		UE_LOG(LogRaymarchVolume, Warning,
		       TEXT(
			       "Following is safe to ignore during cooking :\nTried to initialize Raymarch resources with an unitialized data "
			       "volume with size 0!\nRaymarch volume name = %s, VolumeTexture name = %s"),
		       *GetName(), *NextTexture->GetName());
		return;
	}

	TimePassedPercentage = 0;
	DataVolumeTextureT0 = DataVolumeTextureT1;
	DataVolumeTextureT1 = NextTexture;

	// Todo: The TBRaymarch project reported some bugs when not flushing rendering commands here, however it should be
	// checked again if this is really necessary or if it might decrease performance
	FlushRenderingCommands();

	// Update dynamic material instance
	RaymarchMaterial->SetTextureParameterValue("VolumeT0", DataVolumeTextureT0);
	RaymarchMaterial->SetTextureParameterValue("VolumeT1", DataVolumeTextureT1);
	RaymarchMaterial->SetScalarParameterValue("TimePassedPercentage", TimePassedPercentage);
}

void ARaymarchVolume::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimePassedPercentage = FMath::Clamp<float>(TimePassedPercentage + DeltaTime / Sim->UpdateRates["Volume"], 0, 1);
	RaymarchMaterial->SetScalarParameterValue("TimePassedPercentage", TimePassedPercentage);
}

void ARaymarchVolume::UseSimulationTransform()
{
	if (DataAsset)
	{
		SetActorScale3D(FVector{1, 1, 1});
		StaticMeshComponent->SetRelativeLocation(FVector{0, 0, 0});

		// Unreal units = cm, FDS has sizes in m -> multiply by 100.
		StaticMeshComponent->SetRelativeScale3D(Cast<UVolumeDataInfo>(DataAsset->DataInfo)->WorldDimensions * 100);
		SetActorRelativeLocation((Cast<UVolumeDataInfo>(DataAsset->DataInfo)->MeshPos + Cast<UVolumeDataInfo>(DataAsset->DataInfo)->WorldDimensions / 2) * 100);
	}
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif
