#include "Actor/Obst.h"

#include "VRSSConfig.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("StaticMesh'/Game/Meshes/SM_6SurfCube.SM_6SurfCube'"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Obst Static Mesh"));
	// Set basic unit cube properties.
	if (CubeMesh.Succeeded())
	{
		StaticMeshComponent->SetStaticMesh(CubeMesh.Object);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
		StaticMeshComponent->SetRelativeLocation(FVector(0, 0, 0));
		StaticMeshComponent->SetRelativeScale3D(FVector(0, 0, 0));
		StaticMeshComponent->SetupAttachment(RootComponent);
	}

	if (static ConstructorHelpers::FObjectFinder<UMaterial> Material(
		TEXT("Material'/Game/Materials/M_Obst.M_Obst'")); Material.Succeeded())
	{
		ObstDataMaterialBase = Material.Object;
	}


	// Create CubeBorderMeshComponent and find and assign cube border mesh (that's a cube with only edges visible).
	CubeBorderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Obst Cube Border"));
	CubeBorderMeshComponent->SetupAttachment(StaticMeshComponent);
	CubeBorderMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	CubeBorderMeshComponent->SetHiddenInGame(true);
	CubeBorderMeshComponent->SetRelativeScale3D(FVector(100.0f));

	if (static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeBorder(
		TEXT("StaticMesh'/Game/Meshes/SM_Unit_Cube.SM_Unit_Cube'")); CubeBorder.Succeeded())
	{
		// Find and assign cube material.
		CubeBorderMeshComponent->SetStaticMesh(CubeBorder.Object);
		if (static ConstructorHelpers::FObjectFinder<UMaterial> BorderMaterial(
			TEXT("Material'/Game/Materials/M_CubeBorder.M_CubeBorder'")); BorderMaterial.Succeeded())
		{
			CubeBorderMeshComponent->SetMaterial(0, BorderMaterial.Object);
		}
	}
}

void AObst::BeginPlay()
{
	Super::BeginPlay();

	Sim = Cast<ASimulation>(GetParentActor());

	TArray<int> Orientations;
	ObstAsset->ObstInfo.Dimensions.GetKeys(Orientations);

	for (const int Orientation : Orientations)
	{
		DataTexturesT0.Add(Orientation, nullptr);
		DataTexturesT1.Add(Orientation, nullptr);
	}

	Sim->InitUpdateRate("Obst", ObstAsset->ObstInfo.Spacings[Orientations[0]].W);

	// Set some default quantity as active
	TArray<FString> Quantities;
	ObstAsset->ObstInfo.TextureDirs.GetKeys(Quantities);
	if (Quantities.Contains("wall_temperature")) SetActiveQuantity("wall_temperature");
	else SetActiveQuantity(Quantities[0]);

	for (const int Orientation : Orientations)
		if (StaticMeshComponent)
			StaticMeshComponent->SetMaterialByName(*FString::FromInt(Orientation),
			                                       ObstDataMaterials[Orientation]);
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

	if (!NextTexture->PlatformData || NextTexture->GetSizeX() == 0 || NextTexture->GetSizeY() == 0 ||
		NextTexture->GetSizeY() == 0)
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

void AObst::SetActiveQuantity(FString NewQuantity)
{
	const UVRSSGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<UVRSSGameInstanceSubsystem>();
	
	ActiveQuantity = NewQuantity;

	ObstAsset->ObstTextures.Add(NewQuantity, TMap<int, TArray<FAssetData>>());

	TArray<int> Orientations;
	ObstAsset->ObstInfo.Dimensions.GetKeys(Orientations);

	const float CutOffValue = (GI->Config->ObstCutOffValues[ActiveQuantity] - ObstAsset->ObstInfo.
		MinValues[ActiveQuantity]) * ObstAsset->ObstInfo.ScaleFactors[ActiveQuantity] / 255.f;
	for (const int Orientation : Orientations)
	{
		ObstAsset->ObstTextures[NewQuantity].Add(Orientation, TArray<FAssetData>());

		// Registering the automatic async texture loading each timestep
		// Check if the expected amount of data (textures) could be found in the given directory
		if (TOptional<FUpdateDataEvent*> UpdateDataEvent = Sim->RegisterTextureLoad(
			"Obst", ObstAsset->ObstInfo.TextureDirs[ActiveQuantity].FaceDirs[Orientation],
			&ObstAsset->ObstTextures[ActiveQuantity][Orientation], ObstAsset->ObstInfo.Dimensions[Orientation].W); UpdateDataEvent.IsSet())
		{
			UpdateDataEvent.GetValue()->AddUObject(this, &AObst::UpdateTexture, Orientation);
		}
		else
		{
			// If the data has not been loaded yet (or incorrectly loaded), do it again after loading the data
			UpdateDataEvent = Sim->RegisterTextureLoad(
				"Obst", ObstAsset->ObstInfo.TextureDirs[ActiveQuantity].FaceDirs[Orientation],
				&ObstAsset->ObstTextures[ActiveQuantity][Orientation], ObstAsset->ObstInfo.Dimensions[Orientation].W);
			if (UpdateDataEvent.IsSet())
			{
				UpdateDataEvent.GetValue()->AddUObject(this, &AObst::UpdateTexture, Orientation);
			}
			else
			{
				// Todo: Error - Data could not be loaded correctly (at least not the expected amount of data)
			}
		}

		if (ObstDataMaterialBase)
		{
			UMaterialInstanceDynamic* ObstMaterial = UMaterialInstanceDynamic::Create(
				ObstDataMaterialBase, this, *("Obst Mat Dynamic Inst" + FString::FromInt(Orientation)));
			ObstDataMaterials.Add(Orientation, ObstMaterial);

			ObstMaterial->SetScalarParameterValue("CutOffValue", CutOffValue);

			ObstMaterial->SetTextureParameterValue("ColorMap", GI->Config->ColorMaps[ActiveQuantity]);

			Sim->ChangeObstQuantity(this);
		}

		// Initialize resources for timestep t=-1 and t=0 (for time interpolation)
		UpdateTexture(Sim->CurrentTimeSteps["Obst"] - 1, Orientation);
		UpdateTexture(Sim->CurrentTimeSteps["Obst"], Orientation);
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
