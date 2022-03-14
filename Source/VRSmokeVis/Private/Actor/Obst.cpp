#include "Actor/Obst.h"

#include "VRSSConfig.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Util/TextureUtilities.h"
#include "Assets/ObstAsset.h"
#include "VRSSGameInstanceSubsystem.h"
#include "Actor/Simulation.h"

DEFINE_LOG_CATEGORY(LogObst)

#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif

// Sets default values
AObst::AObst() : AActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetActorEnableCollision(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	RootComponent->SetWorldScale3D(FVector(1.0f));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Obst Static Mesh"));
	// Set basic unit cube properties
	StaticMeshComponent->SetStaticMesh(Cube6SurfMesh);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	StaticMeshComponent->SetRelativeLocation(FVector(0, 0, 0));
	StaticMeshComponent->SetRelativeScale3D(FVector(0, 0, 0));
	StaticMeshComponent->SetupAttachment(RootComponent);

	// Create CubeBorderMeshComponent and assign cube border mesh (that's a cube with only edges visible).
	CubeBorderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Obst Cube Border"));
	CubeBorderMeshComponent->SetupAttachment(StaticMeshComponent);
	CubeBorderMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	CubeBorderMeshComponent->SetHiddenInGame(true);
	CubeBorderMeshComponent->SetRelativeScale3D(FVector(100.0f));

	// Find and assign cube material.
	CubeBorderMeshComponent->SetStaticMesh(CubeBorder);
	CubeBorderMeshComponent->SetMaterial(0, BorderMaterial);
}

void AObst::BeginPlay()
{
	Super::BeginPlay();

	Sim = Cast<ASimulation>(GetOwner());

	TArray<int> Orientations;
	ObstAsset->ObstInfo.Dimensions.GetKeys(Orientations);

	for (const int Orientation : Orientations)
	{
		DataTexturesT0.Add(Orientation, nullptr);
		DataTexturesT1.Add(Orientation, nullptr);
	}

	Sim->InitUpdateRate("Obst", ObstAsset->ObstInfo.Spacings[Orientations[0]].W, ObstAsset->ObstInfo.Dimensions[Orientations[0]].W);

	for (const int Orientation : Orientations)
	{
		if (ObstDataMaterialBase)
		{
			UMaterialInstanceDynamic* ObstMaterial = UMaterialInstanceDynamic::Create(
				ObstDataMaterialBase, this, *("Obst Mat Dynamic Inst" + FString::FromInt(Orientation)));
			ObstDataMaterials.Add(Orientation, ObstMaterial);
		}
		
		if (StaticMeshComponent)
			StaticMeshComponent->SetMaterialByName(*FString::FromInt(Orientation), ObstDataMaterials[Orientation]);
	}
}

void AObst::UpdateTexture(const int CurrentTimeStep, const int Orientation)
{
	// Load the texture for the next time step to interpolate between the next and current one
	UTexture2D* NextTexture = Cast<UTexture2D>(
		ObstAsset->ObstTextures[ActiveQuantity][Orientation][(CurrentTimeStep + 1) % ObstAsset->ObstTextures[
			ActiveQuantity][Orientation].Num()].
		GetAsset());

	if (!NextTexture)
	{
		UE_LOG(LogObst, Error, TEXT("Tried to initialize Obst resources with no data textures!"));
		return;
	}

	if (!NextTexture->GetPlatformData() || NextTexture->GetSizeX() == 0 || NextTexture->GetSizeY() == 0)
	{
		// Happens in cooking stage where per-platform data isn't initialized. Return.
		UE_LOG(LogObst, Warning,
		       TEXT(
			       "Following is safe to ignore during cooking :\nTried to initialize Obst resources with an unitialized data "
			       "texture with size 0!\nObst name = %s, Texture name = %s"),
		       *GetName(), *NextTexture->GetName());
		return;
	}

	TimePassedPercentage = 0;
	DataTexturesT0[Orientation] = DataTexturesT1[Orientation];
	DataTexturesT1[Orientation] = NextTexture;

	// Update dynamic material instance
	ObstDataMaterials[Orientation]->SetTextureParameterValue("TextureT0", DataTexturesT0[Orientation]);
	ObstDataMaterials[Orientation]->SetTextureParameterValue("TextureT1", DataTexturesT1[Orientation]);
	ObstDataMaterials[Orientation]->SetScalarParameterValue("TimePassedPercentage", TimePassedPercentage);
}

void AObst::UpdateColorMapScale(const float NewMin, const float NewMax) const
{
	if (ActiveQuantity.IsEmpty()) return;
	
	TArray<int> Orientations;
	ObstAsset->ObstInfo.Dimensions.GetKeys(Orientations);

	const float NewRange = NewMax - NewMin;
	const float NewMinScaled = ObstAsset->ObstInfo.MinValues[ActiveQuantity] / NewRange + (ObstAsset->ObstInfo.
			MinValues[ActiveQuantity] - NewMin)
		/ NewRange;
	const float ColorMapRange = (ObstAsset->ObstInfo.MaxValues[ActiveQuantity] - ObstAsset->ObstInfo.MinValues[
		ActiveQuantity]) / NewRange;

	for (const int Orientation : Orientations)
	{
		ObstDataMaterials[Orientation]->SetScalarParameterValue("ColorMapMin", NewMinScaled);
		ObstDataMaterials[Orientation]->SetScalarParameterValue("ColorMapRange", ColorMapRange);
	}
}

void AObst::SetActiveQuantity(FString GlobalObstQuantity)
{
	// Don't change anything if nothing changed
	if (ActiveQuantity == GlobalObstQuantity) return;
	
	ActiveQuantity = GlobalObstQuantity;
	
	const UVRSSGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>();

	ObstAsset->ObstTextures.FindOrAdd(ActiveQuantity, TMap<int, TArray<FAssetData>>());

	TArray<int> Orientations;
	ObstAsset->ObstInfo.Dimensions.GetKeys(Orientations);

	const float CutOffValue = (GI->Config->GetObstCutOffValue(ActiveQuantity) - ObstAsset->ObstInfo.
		MinValues[ActiveQuantity]) * ObstAsset->ObstInfo.ScaleFactors[ActiveQuantity] / 255.f;
	for (const int Orientation : Orientations)
	{
		ObstAsset->ObstTextures[ActiveQuantity].FindOrAdd(Orientation, TArray<FAssetData>());

		ObstDataMaterials[Orientation]->SetScalarParameterValue("CutOffValue", CutOffValue);

		ObstDataMaterials[Orientation]->SetTextureParameterValue("ColorMap", GI->Config->GetColorMap(ActiveQuantity));
	}
}

void AObst::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimePassedPercentage = FMath::Clamp<float>(TimePassedPercentage + DeltaTime / Sim->UpdateRates["Obst"], 0, 1);
}

void AObst::UseSimulationTransform()
{
	if (ObstAsset)
	{
		SetActorScale3D(FVector{1, 1, 1});
		StaticMeshComponent->SetRelativeLocation(FVector{0, 0, 0});

		// Unreal units = cm, FDS has sizes in m. However, the cube mesh is 1x1m already, so no conversion is needed
		const FVector ObstScale = FVector(ObstAsset->BoundingBox[1] - ObstAsset->BoundingBox[0],
		                                  ObstAsset->BoundingBox[3] - ObstAsset->BoundingBox[2],
		                                  ObstAsset->BoundingBox[5] - ObstAsset->BoundingBox[4]);
		StaticMeshComponent->SetRelativeScale3D(ObstScale * 1.01);

		const FVector ObstLocation = FVector(ObstAsset->BoundingBox[0], ObstAsset->BoundingBox[2],
		                                     ObstAsset->BoundingBox[4]) + ObstScale / 2;
		SetActorLocation(ObstLocation * 100);
	}
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif
