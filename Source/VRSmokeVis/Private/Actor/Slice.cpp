#include "Actor/Slice.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Assets/TextureUtilities.h"

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
		// StaticMeshComponent->SetRelativeScale3D(FVector(100.f, 100.f, 0.f));
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

	UGameInstance* GIRaw = GetGameInstance();
	GI = Cast<UVRSSGameInstance>(GIRaw);

	if(SliceMaterialBase)
	{
		SliceMaterial = UMaterialInstanceDynamic::Create(SliceMaterialBase, this, "Slice Mat Dynamic Inst");
	}
	
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetMaterial(0, SliceMaterial);
	}
	
	if (SliceAsset)
	{		
		GI->InitUpdateRate(SliceAsset->VolumeInfo.Spacing.W);
	}
	
	FUpdateVolumeEvent& UpdateVolumeEvent = GI->RegisterTextureLoad(
		SliceAsset->VolumeInfo.VolumeTextureDir, &SliceAsset->SliceTextures);
	UpdateVolumeEvent.AddUObject(this, &ASlice::UpdateTexture);

	// Initialize resources for timestep t=-1 and t=0 (for time interpolation)
	UpdateTexture(-1);
	UpdateTexture(0);
}

void ASlice::UpdateTexture(const int CurrentTimeStep)
{
	// Load the texture for the next time step to interpolate between the next and current one
	UTexture2D* NextTexture = Cast<UTexture2D>(SliceAsset->SliceTextures[(CurrentTimeStep + 1) % SliceAsset->SliceTextures.Num()].GetAsset());

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

void ASlice::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TimePassedPercentage = FMath::Clamp<float>(TimePassedPercentage + DeltaTime / GI->UpdateRate, 0, 1);
}

void ASlice::UseSimulationTransform()
{
	if (SliceAsset)
	{
		SetActorScale3D(FVector{1,1,1});
		StaticMeshComponent->SetRelativeLocation(FVector{0, 0, 0});
		
		
		// Planes always use XY as dimensions, adjust scale and rotation accordingly
		const bool bNeedsSwap = SliceAsset->VolumeInfo.Dimensions.Y == 1;
		if (bNeedsSwap)
		{
			SliceAsset->VolumeInfo.WorldDimensions.Y = SliceAsset->VolumeInfo.WorldDimensions.Z;
			SliceAsset->VolumeInfo.WorldDimensions.Z = 1;
		}
		
		// Unreal units = cm, FDS has sizes in m. The default plane however is 1x1m already, so no conversion is needed
		StaticMeshComponent->SetRelativeScale3D(SliceAsset->VolumeInfo.WorldDimensions);
		// Rotations for corresponding plane: XY=0,0,0 - XZ=90,0,0 - YZ=0,90,0
		if (bNeedsSwap)  // XZ
		{
			StaticMeshComponent->SetRelativeRotation(FQuat{0.7071068f, 0, 0, 0.7071068f});
		} else if (SliceAsset->VolumeInfo.Dimensions.X == 1)  // YZ
		{
			StaticMeshComponent->SetRelativeRotation(FQuat{0, 0.7071068f, 0, 0.7071068f});
		}
		// Correct the dimensions again
		if (bNeedsSwap)
		{
			SliceAsset->VolumeInfo.WorldDimensions.Z = SliceAsset->VolumeInfo.WorldDimensions.Y;
			SliceAsset->VolumeInfo.WorldDimensions.Y = 1;
		}
		
		SetActorLocation((SliceAsset->VolumeInfo.MeshPos + SliceAsset->VolumeInfo.WorldDimensions/2) * 100);
	}
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif
