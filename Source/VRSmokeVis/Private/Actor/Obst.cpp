#include "Actor/Obst.h"

#include "VRSSConfig.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Util/TextureUtilities.h"
#include "Assets/ObstAsset.h"
#include "VRSSGameInstanceSubsystem.h"
#include "Actor/Simulation.h"
#include "Assets/BoundaryDataInfo.h"

DEFINE_LOG_CATEGORY(LogObst)

#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif

// Sets default values
AObst::AObst() : AFdsActor()
{
	/* Watch out, when changing anything in this function, the child blueprint (BP_Obst) will not reflect the changes
	 * immediately. For the derived blueprint to apply the changes, one has to reparent the blueprint to sth. like
	 * Actor and then reparent to Obst again. After that all default values in the Obst (M_Obst and M_CubeBorder
	 * in RootComponent) and the StaticMeshes (SM_6SurfCube1m in StaticMeshComponent and SM_UnitCube in
	 * CubeBorderMeshComponent) have to be set again. */

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetActorEnableCollision(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	RootComponent->SetWorldScale3D(FVector(1.0f));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Obst Static Mesh"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	StaticMeshComponent->SetRelativeLocation(FVector(0, 0, 0));
	StaticMeshComponent->SetRelativeScale3D(FVector(1, 1, 1));
	StaticMeshComponent->SetCastShadow(false);
	StaticMeshComponent->SetupAttachment(RootComponent);

	// Create CubeBorderMeshComponent and assign cube border mesh (that's a cube with only edges visible).
	CubeBorderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Obst Cube Border"));
	CubeBorderMeshComponent->SetupAttachment(StaticMeshComponent);
	CubeBorderMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	CubeBorderMeshComponent->SetHiddenInGame(true);
	CubeBorderMeshComponent->SetRelativeScale3D(FVector(100.0f));
	CubeBorderMeshComponent->SetMaterial(0, BorderMaterial);
}

void AObst::BeginPlay()
{
	Super::BeginPlay();

	Sim = Cast<ASimulation>(GetOwner());

	UBoundaryDataInfo* ObstDataInfo = Cast<UBoundaryDataInfo>(DataAsset->DataInfo);

	TArray<int> Orientations;
	ObstDataInfo->Dimensions.GetKeys(Orientations);

	for (const int Orientation : Orientations)
	{
		DataTexturesT0.Add(Orientation, nullptr);
		DataTexturesT1.Add(Orientation, nullptr);
	}

	Sim->InitUpdateRate("Obst", ObstDataInfo->Spacings[Orientations[0]].W, ObstDataInfo->Dimensions[Orientations[0]].W);

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
		Cast<UObstAsset>(DataAsset)->ObstTextures[ActiveQuantity][Orientation][(CurrentTimeStep + 1) % Cast<UObstAsset>(DataAsset)->ObstTextures[
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

	UBoundaryDataInfo* ObstDataInfo = Cast<UBoundaryDataInfo>(DataAsset->DataInfo);
	
	TArray<int> Orientations;
	ObstDataInfo->Dimensions.GetKeys(Orientations);

	const float NewRange = NewMax - NewMin;
	const float NewMinScaled = ObstDataInfo->MinValues[ActiveQuantity] / NewRange + (ObstDataInfo->
			MinValues[ActiveQuantity] - NewMin)
		/ NewRange;
	const float ColorMapRange = (ObstDataInfo->MaxValues[ActiveQuantity] - ObstDataInfo->MinValues[
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

	UObstAsset *ObstAsset = Cast<UObstAsset>(DataAsset);
	UBoundaryDataInfo* ObstDataInfo = Cast<UBoundaryDataInfo>(ObstAsset->DataInfo);
	ObstAsset->ObstTextures.FindOrAdd(ActiveQuantity, TMap<int, TArray<FAssetData>>());

	TArray<int> Orientations;
	ObstDataInfo->Dimensions.GetKeys(Orientations);

	const float CutOffValue = (GI->Config->GetObstCutOffValue(ActiveQuantity) - ObstDataInfo->
		MinValues[ActiveQuantity]) * ObstDataInfo->ScaleFactors[ActiveQuantity] / 255.f;
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
	if (DataAsset)
	{
		UObstAsset *ObstAsset = Cast<UObstAsset>(DataAsset);
		SetActorScale3D(FVector{1, 1, 1});
		StaticMeshComponent->SetRelativeLocation(FVector{0, 0, 0});

		// Unreal units = cm, FDS has sizes in m. However, the cube mesh is 1x1m already, so no conversion is needed
		const FVector ObstScale = FVector(ObstAsset->BoundingBox[1] - ObstAsset->BoundingBox[0],
		                                  ObstAsset->BoundingBox[3] - ObstAsset->BoundingBox[2],
		                                  ObstAsset->BoundingBox[5] - ObstAsset->BoundingBox[4]);
		StaticMeshComponent->SetRelativeScale3D(ObstScale + 0.001);
		// The pivot point of the mesh is not in the center, but on the lower side instead. We therefore have to adjust
		// the z-coordinate to re-center the actor
		const FVector Adjustment = FVector(ObstScale.X, ObstScale.Y, -0.001);
		const FVector ObstLocation = FVector(ObstAsset->BoundingBox[0], ObstAsset->BoundingBox[2],
		                                     ObstAsset->BoundingBox[4]) + Adjustment / 2;
		SetActorRelativeLocation(ObstLocation * 100);
	}
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif
