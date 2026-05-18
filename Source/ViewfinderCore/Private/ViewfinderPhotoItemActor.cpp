#include "ViewfinderPhotoItemActor.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "ViewfinderPhotoData.h"
#include "ViewfinderProjectionMath.h"

DEFINE_LOG_CATEGORY_STATIC(LogViewfinderPhotoItem, Log, All);

AViewfinderPhotoItemActor::AViewfinderPhotoItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	PhotoPlane = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PhotoPlane"));
	SetRootComponent(PhotoPlane);
	PhotoPlane->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AViewfinderPhotoItemActor::InitializePhoto(UViewfinderPhotoData* InPhotoData, UMaterialInterface* InPhotoMaterial, FName InTextureParameterName, float InPhotoPlaneDistance)
{
	PhotoData = InPhotoData;
	PhotoMaterial = InPhotoMaterial;
	TextureParameterName = InTextureParameterName;
	PhotoPlaneDistance = InPhotoPlaneDistance;
	RebuildPhotoPlane();
}

void AViewfinderPhotoItemActor::RebuildPhotoPlane()
{
	PhotoPlane->ClearAllMeshSections();

	if (!PhotoData)
	{
		UE_LOG(LogViewfinderPhotoItem, Error, TEXT("RebuildPhotoPlane failed: PhotoData is null."));
		return;
	}

	if (!PhotoData->RenderTarget)
	{
		UE_LOG(LogViewfinderPhotoItem, Error, TEXT("RebuildPhotoPlane failed: PhotoData has no render target."));
		return;
	}

	if (PhotoPlaneDistance <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogViewfinderPhotoItem, Error, TEXT("RebuildPhotoPlane failed: PhotoPlaneDistance must be positive. Current value: %f."), PhotoPlaneDistance);
		return;
	}

	const float HalfWidth = PhotoPlaneDistance * FViewfinderProjectionMath::GetTanHalfHorizontalFov(PhotoData->ProjectionParams);
	const float HalfHeight = PhotoPlaneDistance * FViewfinderProjectionMath::GetTanHalfVerticalFov(PhotoData->ProjectionParams);

	TArray<FVector> Vertices;
	Vertices.Add(FVector(0.0, -HalfWidth, -HalfHeight));
	Vertices.Add(FVector(0.0, -HalfWidth, HalfHeight));
	Vertices.Add(FVector(0.0, HalfWidth, HalfHeight));
	Vertices.Add(FVector(0.0, HalfWidth, -HalfHeight));

	const TArray<int32> Indices = { 0, 1, 2, 0, 2, 3 };
	const TArray<FVector> Normals =
	{
		FVector(-1.0, 0.0, 0.0),
		FVector(-1.0, 0.0, 0.0),
		FVector(-1.0, 0.0, 0.0),
		FVector(-1.0, 0.0, 0.0)
	};
	const TArray<FVector2D> UVs =
	{
		FVector2D(0.0, 1.0),
		FVector2D(0.0, 0.0),
		FVector2D(1.0, 0.0),
		FVector2D(1.0, 1.0)
	};
	const TArray<FVector2D> EmptyUVs;
	const TArray<FLinearColor> VertexColors =
	{
		FLinearColor::White,
		FLinearColor::White,
		FLinearColor::White,
		FLinearColor::White
	};
	const TArray<FProcMeshTangent> Tangents =
	{
		FProcMeshTangent(0.0f, 1.0f, 0.0f),
		FProcMeshTangent(0.0f, 1.0f, 0.0f),
		FProcMeshTangent(0.0f, 1.0f, 0.0f),
		FProcMeshTangent(0.0f, 1.0f, 0.0f)
	};

	PhotoPlane->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, UVs, EmptyUVs, EmptyUVs, EmptyUVs, VertexColors, Tangents, false, false);

	if (!PhotoMaterial)
	{
		UE_LOG(LogViewfinderPhotoItem, Error, TEXT("RebuildPhotoPlane could not assign texture: PhotoMaterial is not assigned. Assign a material with a Texture2D parameter named %s."), *TextureParameterName.ToString());
		return;
	}

	DynamicPhotoMaterial = UMaterialInstanceDynamic::Create(PhotoMaterial, this);
	DynamicPhotoMaterial->SetTextureParameterValue(TextureParameterName, PhotoData->RenderTarget);
	PhotoPlane->SetMaterial(0, DynamicPhotoMaterial);
}
