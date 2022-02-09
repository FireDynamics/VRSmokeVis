#include "Actor/Slice.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Util/TextureUtilities.h"
#include "Assets/SliceAsset.h"
#include "Actor/Simulation.h"

DEFINE_LOG_CATEGORY(LogSlice)

#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif

// Sets default values
ASlice::ASlice() : AActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetActorEnableCollision(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	RootComponent->SetWorldScale3D(FVector(1.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("StaticMesh'/Game/Meshes/SM_Plane.SM_Plane'"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Slice Static Mesh"));
	// Set basic unit cube properties.
	if (PlaneMesh.Succeeded())
	{
		StaticMeshComponent->SetStaticMesh(PlaneMesh.Object);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMeshComponent->SetupAttachment(RootComponent);
	}

	if (static ConstructorHelpers::FObjectFinder<UMaterial> Material(
		TEXT("Material'/Game/Materials/M_Slice.M_Slice'")); Material.Succeeded())
	{
		SliceMaterialBase = Material.Object;
	}
}

void ASlice::BeginPlay()
{
	Super::BeginPlay();

	Sim = Cast<ASimulation>(GetParentActor());

	check(SliceAsset)

	if (SliceMaterialBase)
	{
		SliceMaterial = UMaterialInstanceDynamic::Create(SliceMaterialBase, this, "Slice Mat Dynamic Inst");

		const float CutOffValue = (Sim->Config->SliceCutOffValues[SliceAsset->SliceInfo.Quantity] - SliceAsset->SliceInfo
			.
			MinValue) * SliceAsset->SliceInfo.ScaleFactor / 255.f;
		SliceMaterial->SetScalarParameterValue("CutOffValue", CutOffValue);

		SliceMaterial->SetTextureParameterValue("ColorMap", Sim->Config->ColorMaps[SliceAsset->SliceInfo.Quantity]);
	}

	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetMaterial(0, SliceMaterial);
	}

	if (SliceAsset)
	{
		Sim->InitUpdateRate("Slice", SliceAsset->SliceInfo.Spacing.W);
	}
	// Registering the automatic async texture loading each timestep
	// Check if the expected amount of data (textures) could be found in the given directory
	if (TOptional<FUpdateDataEvent*> UpdateDataEvent = Sim->RegisterTextureLoad(
			"Slice", SliceAsset->SliceInfo.TextureDir, &SliceAsset->SliceTextures, SliceAsset->SliceInfo.Dimensions.W);
		UpdateDataEvent.IsSet())
	{
		UpdateDataEvent.GetValue()->AddUObject(this, &ASlice::UpdateTexture);
	}
	else
	{
		// If the data has not been loaded yet (or incorrectly loaded), do it again after loading the data
		UpdateDataEvent = Sim->RegisterTextureLoad("Slice", SliceAsset->SliceInfo.TextureDir, &SliceAsset->SliceTextures,
		                                          SliceAsset->SliceInfo.Dimensions.W);
		if (UpdateDataEvent.IsSet())
		{
			UpdateDataEvent.GetValue()->AddUObject(this, &ASlice::UpdateTexture);
		}
		else
		{
			// Todo: Error - Data could not be loaded correctly (at least not the expected amount of data)
		}
	}

	// Initialize resources for first timesteps (for time interpolation)
	UpdateTexture(Sim->CurrentTimeSteps["Slice"] - 1);
	UpdateTexture(Sim->CurrentTimeSteps["Slice"]);

	// Let the GameInstance know when we spawn a slice
	Sim->AddSlice(this);
}

void ASlice::UpdateTexture(const int CurrentTimeStep)
{
	// Load the texture for the next time step to interpolate between the next and current one
	UTexture2D* NextTexture = Cast<UTexture2D>(
		SliceAsset->SliceTextures[(CurrentTimeStep + 1) % SliceAsset->SliceTextures.Num()].GetAsset());

	if (!NextTexture)
	{
		UE_LOG(LogSlice, Error, TEXT("Tried to initialize Slice resources with no data textures!"));
		return;
	}

	if (!NextTexture->PlatformData || NextTexture->GetSizeX() == 0 || NextTexture->GetSizeY() == 0 ||
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

void ASlice::UpdateColorMapScale(const FString Quantity, const float NewMin, const float NewMax) const
{
	if (Quantity.Equals(SliceAsset->SliceInfo.Quantity))
	{
		const float NewRange = NewMax - NewMin;
		const float NewMinScaled = SliceAsset->SliceInfo.MinValue / NewRange + (SliceAsset->SliceInfo.MinValue - NewMin)
			/ NewRange;

		SliceMaterial->SetScalarParameterValue("ColorMapMin", NewMinScaled);
		SliceMaterial->SetScalarParameterValue("ColorMapRange",
		                                       (SliceAsset->SliceInfo.MaxValue - SliceAsset->SliceInfo.MinValue) /
		                                       NewRange);
	}
}

void ASlice::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimePassedPercentage = FMath::Clamp<float>(TimePassedPercentage + DeltaTime / Sim->UpdateRates["Slice"], 0, 1);
}

void ASlice::UseSimulationTransform()
{
	if (SliceAsset)
	{
		SetActorScale3D(FVector{1, 1, 1});
		StaticMeshComponent->SetRelativeLocation(FVector{0, 0, 0});


		// Planes always use XY as dimensions, adjust scale and rotation accordingly
		const bool SwapX = SliceAsset->SliceInfo.Dimensions.X == 1;
		const bool SwapY = SliceAsset->SliceInfo.Dimensions.Y == 1;
		if (SwapX)
		{
			const float Tmp = SliceAsset->SliceInfo.WorldDimensions.X;
			SliceAsset->SliceInfo.WorldDimensions.X = SliceAsset->SliceInfo.WorldDimensions.Y;
			SliceAsset->SliceInfo.WorldDimensions.Y = SliceAsset->SliceInfo.WorldDimensions.Z;
			SliceAsset->SliceInfo.WorldDimensions.Z = Tmp;
		}
		if (SwapY)
		{
			const float Tmp = SliceAsset->SliceInfo.WorldDimensions.Y;
			SliceAsset->SliceInfo.WorldDimensions.Y = SliceAsset->SliceInfo.WorldDimensions.Z;
			SliceAsset->SliceInfo.WorldDimensions.Z = Tmp;
		}

		// Unreal units = cm, FDS has sizes in m. The default plane however is 1x1m already, so no conversion is needed
		StaticMeshComponent->SetRelativeScale3D(SliceAsset->SliceInfo.WorldDimensions);
		// Rotations for corresponding plane: XY=0,0,0 - XZ=90,0,0 - YZ=0,90,0
		if (SwapX) // YZ
		{
			StaticMeshComponent->SetRelativeRotation(FQuat{0.5f, 0.5f, 0.5f, 0.5f});
		}
		else if (SwapY) // XZ
		{
			StaticMeshComponent->SetRelativeRotation(FQuat{0.7071068f, 0, 0, 0.7071068f});
		}
		// Correct the dimensions again
		if (SwapX)
		{
			const float Tmp = SliceAsset->SliceInfo.WorldDimensions.X;
			SliceAsset->SliceInfo.WorldDimensions.X = SliceAsset->SliceInfo.WorldDimensions.Z;
			SliceAsset->SliceInfo.WorldDimensions.Z = SliceAsset->SliceInfo.WorldDimensions.Y;
			SliceAsset->SliceInfo.WorldDimensions.Y = Tmp;
		}
		if (SwapY)
		{
			const float Tmp = SliceAsset->SliceInfo.WorldDimensions.Z;
			SliceAsset->SliceInfo.WorldDimensions.Z = SliceAsset->SliceInfo.WorldDimensions.Y;
			SliceAsset->SliceInfo.WorldDimensions.Y = Tmp;
		}

		SetActorLocation((SliceAsset->SliceInfo.MeshPos + SliceAsset->SliceInfo.WorldDimensions / 2) * 100);
	}
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif
