#include "Actor/RaymarchVolume.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Util/TextureUtilities.h"
#include "Assets/VolumeAsset.h"
#include "Actor/Simulation.h"

DEFINE_LOG_CATEGORY(LogRaymarchVolume)

#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif

// Sets default values
ARaymarchVolume::ARaymarchVolume() : AActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetActorEnableCollision(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	RootComponent->SetWorldScale3D(FVector(1.0f));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Raymarch Cube Static Mesh"));
	// Set basic unit cube properties
	StaticMeshComponent->SetStaticMesh(UnitCubeInsideOut);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	StaticMeshComponent->SetRelativeScale3D(FVector(100.0f));
	StaticMeshComponent->SetupAttachment(RootComponent);

	// Create CubeBorderMeshComponent and assign cube border mesh (that's a cube with only edges visible)
	CubeBorderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Raymarch Volume Cube Border"));
	CubeBorderMeshComponent->SetupAttachment(StaticMeshComponent);
	CubeBorderMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	CubeBorderMeshComponent->SetHiddenInGame(true);
	CubeBorderMeshComponent->SetStaticMesh(CubeBorder);
	CubeBorderMeshComponent->SetMaterial(0, BorderMaterial);
}

void ARaymarchVolume::BeginPlay()
{
	Super::BeginPlay();

	check(VolumeAsset)

	Sim = Cast<ASimulation>(GetParentActor());

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

	// Unreal units = cm, FDS has sizes in m -> multiply by 100.
	StaticMeshComponent->SetRelativeScale3D(VolumeAsset->VolumeInfo.WorldDimensions * 100);

	Sim->InitUpdateRate("Volume", VolumeAsset->VolumeInfo.Spacing.W);

	// Registering the automatic async texture loading each timestep
	// Check if the expected amount of data (textures) could be found in the given directory
	if (TOptional<FUpdateDataEvent*> UpdateDataEvent = Sim->RegisterTextureLoad(
			"Volume", VolumeAsset->VolumeInfo.TextureDir, &VolumeAsset->VolumeTextures,
			VolumeAsset->VolumeInfo.Dimensions.W);
		UpdateDataEvent.IsSet())
	{
		UpdateDataEvent.GetValue()->AddUObject(this, &ARaymarchVolume::UpdateVolume);
	}
	else
	{
		// If the data has not been loaded yet (or incorrectly loaded), do it again after loading the data
		UpdateDataEvent = Sim->RegisterTextureLoad("Volume", VolumeAsset->VolumeInfo.TextureDir,
		                                          &VolumeAsset->VolumeTextures, VolumeAsset->VolumeInfo.Dimensions.W);
		if (UpdateDataEvent.IsSet())
		{
			UpdateDataEvent.GetValue()->AddUObject(this, &ARaymarchVolume::UpdateVolume);
		}
		else
		{
			// Todo: Error - Data could not be loaded correctly (at least not the expected amount of data)
		}
	}

	// Initialize resources for timestep t=-1 and t=0 (for time interpolation)
	UpdateVolume(-1);
	UpdateVolume(0);
}

void ARaymarchVolume::UpdateVolume(const int CurrentTimeStep)
{
	// Load the texture for the next time step to interpolate between the next and current one
	UVolumeTexture* NextTexture = Cast<UVolumeTexture>(
		VolumeAsset->VolumeTextures[(CurrentTimeStep + 1) % VolumeAsset->VolumeTextures.Num()].GetAsset());

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
	if (VolumeAsset)
	{
		SetActorScale3D(FVector{1, 1, 1});
		StaticMeshComponent->SetRelativeLocation(FVector{0, 0, 0});

		// Unreal units = cm, FDS has sizes in m -> multiply by 100.
		StaticMeshComponent->SetRelativeScale3D(VolumeAsset->VolumeInfo.WorldDimensions * 100);
		SetActorLocation((VolumeAsset->VolumeInfo.MeshPos + VolumeAsset->VolumeInfo.WorldDimensions / 2) * 100);
	}
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif
