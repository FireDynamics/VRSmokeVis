#include "Actor/RaymarchVolume.h"

#include "VRSSGameInstanceSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "VolumeAsset/TextureUtilities.h"

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
	// Set basic unit cube properties.
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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeBorder(
		TEXT("StaticMesh'/Game/Meshes/Unit_Cube.Unit_Cube'"));

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

	if (RaymarchMaterialBase)
	{
		RaymarchMaterial =
			UMaterialInstanceDynamic::Create(RaymarchMaterialBase, this, "Intensity Raymarch Mat Dynamic Inst");

		RaymarchMaterial->SetScalarParameterValue("Steps", RaymarchingSteps);
	}

	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetMaterial(0, RaymarchMaterial);
	}

	if (VolumeAsset)
	{
		// Unreal units = cm, FDS has sizes in m -> multiply by 10.
		StaticMeshComponent->SetRelativeScale3D(VolumeAsset->VolumeInfo.WorldDimensions * 100);
	}
}

void ARaymarchVolume::BeginPlay()
{
	Super::BeginPlay();
	
	UVRSSGameInstanceSubsystem* GameInstanceSubsystem = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>();

	FUpdateVolumeEvent& UpdateVolumeEvent = GameInstanceSubsystem->RegisterTextureLoad(
		VolumeAsset->VolumeInfo.VolumeTextureDir, &VolumeAsset->VolumeTextures);
	UpdateVolumeEvent.AddUObject(this, &ARaymarchVolume::UpdateVolume);

	// Initialize resources for timestep t=-1 and t=0 (for time interpolation)
	UpdateVolume(-1);
	UpdateVolume(0);
}

void ARaymarchVolume::UpdateVolume(const int CurrentTimeStep)
{
	// Load the texture for the next time step to interpolate between the next and current one
	UVolumeTexture* NextTexture = Cast<UVolumeTexture>(VolumeAsset->VolumeTextures[(CurrentTimeStep + 1) % VolumeAsset->VolumeTextures.Num()].GetAsset());

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
	
	UE_LOG(LogRaymarchVolume, Warning, TEXT("Tick called"))
	TimePassedPercentage = FMath::Clamp<float>(TimePassedPercentage + DeltaTime / UPDATERATE, 0, 1);
	RaymarchMaterial->SetScalarParameterValue("TimePassedPercentage", TimePassedPercentage);
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif