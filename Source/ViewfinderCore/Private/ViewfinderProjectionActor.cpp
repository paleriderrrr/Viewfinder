#include "ViewfinderProjectionActor.h"

#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "ViewfinderPhotoData.h"
#include "ViewfinderProjectionMath.h"

DEFINE_LOG_CATEGORY_STATIC(LogViewfinderProjection, Log, All);

AViewfinderProjectionActor::AViewfinderProjectionActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectionMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProjectionMesh"));
	SetRootComponent(ProjectionMesh);
	ProjectionMesh->bUseAsyncCooking = true;
	ProjectionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AViewfinderProjectionActor::BuildProjection(UViewfinderPhotoData* InPhotoData, const FTransform& PhotoPlaneTransform)
{
	RestoreCapturedSourceComponents();
	PhotoData = InPhotoData;

	if (!PhotoData)
	{
		UE_LOG(LogViewfinderProjection, Error, TEXT("BuildProjection failed: PhotoData is null."));
		return;
	}

	if (!PhotoData->RenderTarget)
	{
		UE_LOG(LogViewfinderProjection, Error, TEXT("BuildProjection failed: PhotoData has no render target."));
		return;
	}

	if (!PhotoMaterial)
	{
		UE_LOG(LogViewfinderProjection, Error, TEXT("BuildProjection failed: PhotoMaterial is not assigned. Assign a material with a Texture2D parameter named %s."), *TextureParameterName.ToString());
		return;
	}

	if (PhotoPlaneDistance <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogViewfinderProjection, Error, TEXT("BuildProjection failed: PhotoPlaneDistance must be positive. Current value: %f."), PhotoPlaneDistance);
		return;
	}

	SetActorTransform(PhotoPlaneTransform);
	ProjectionMesh->ClearAllMeshSections();

	TArray<FVector> Vertices;
	TArray<int32> Indices;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FVector2D> EmptyUVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	const int32 TriangleCount = PhotoData->Triangles.Num();
	Vertices.Reserve(TriangleCount * 3);
	Indices.Reserve(TriangleCount * 3);
	Normals.Reserve(TriangleCount * 3);
	UVs.Reserve(TriangleCount * 3);
	VertexColors.Reserve(TriangleCount * 3);
	Tangents.Reserve(TriangleCount * 3);

	const FTransform ActorTransform = GetActorTransform();

	for (const FViewfinderCapturedTriangle& CapturedTriangle : PhotoData->Triangles)
	{
		const FVector WorldA = FViewfinderProjectionMath::BuildPlacedWorldPosition(CapturedTriangle.A, PhotoData->ProjectionParams, PhotoPlaneTransform, PhotoPlaneDistance);
		const FVector WorldB = FViewfinderProjectionMath::BuildPlacedWorldPosition(CapturedTriangle.B, PhotoData->ProjectionParams, PhotoPlaneTransform, PhotoPlaneDistance);
		const FVector WorldC = FViewfinderProjectionMath::BuildPlacedWorldPosition(CapturedTriangle.C, PhotoData->ProjectionParams, PhotoPlaneTransform, PhotoPlaneDistance);

		const FVector LocalA = ActorTransform.InverseTransformPosition(WorldA);
		const FVector LocalB = ActorTransform.InverseTransformPosition(WorldB);
		const FVector LocalC = ActorTransform.InverseTransformPosition(WorldC);
		const FVector FaceNormal = FVector::CrossProduct(LocalB - LocalA, LocalC - LocalA).GetSafeNormal();

		const int32 BaseIndex = Vertices.Num();
		Vertices.Add(LocalA);
		Vertices.Add(LocalB);
		Vertices.Add(LocalC);

		Indices.Add(BaseIndex);
		Indices.Add(BaseIndex + 1);
		Indices.Add(BaseIndex + 2);

		Normals.Add(FaceNormal);
		Normals.Add(FaceNormal);
		Normals.Add(FaceNormal);

		UVs.Add(CapturedTriangle.UVA);
		UVs.Add(CapturedTriangle.UVB);
		UVs.Add(CapturedTriangle.UVC);

		VertexColors.Add(FLinearColor::White);
		VertexColors.Add(FLinearColor::White);
		VertexColors.Add(FLinearColor::White);

		Tangents.Add(FProcMeshTangent(0.0f, 1.0f, 0.0f));
		Tangents.Add(FProcMeshTangent(0.0f, 1.0f, 0.0f));
		Tangents.Add(FProcMeshTangent(0.0f, 1.0f, 0.0f));
	}

	if (Vertices.Num() == 0)
	{
		UE_LOG(LogViewfinderProjection, Warning, TEXT("BuildProjection produced an empty mesh. Check ViewfinderCaptureComponent markers and static mesh CPU access."));
		return;
	}

	ProjectionMesh->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, UVs, EmptyUVs, EmptyUVs, EmptyUVs, VertexColors, Tangents, bCreateCollision, false);
	ProjectionMesh->SetCollisionEnabled(bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

	DynamicPhotoMaterial = UMaterialInstanceDynamic::Create(PhotoMaterial, this);
	DynamicPhotoMaterial->SetTextureParameterValue(TextureParameterName, PhotoData->RenderTarget);
	ProjectionMesh->SetMaterial(0, DynamicPhotoMaterial);

	if (bHideCapturedSourceComponents)
	{
		for (const FViewfinderCapturedSourceComponent& SourceComponent : PhotoData->CapturedSourceComponents)
		{
			UPrimitiveComponent* Component = SourceComponent.Component.Get();
			if (!Component)
			{
				continue;
			}

			FViewfinderSourceComponentState State;
			State.Component = Component;
			State.bWasHiddenInGame = Component->bHiddenInGame;
			State.PreviousCollisionEnabled = Component->GetCollisionEnabled();
			HiddenSourceComponents.Add(State);

			Component->SetHiddenInGame(true);
			Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AViewfinderProjectionActor::RestoreCapturedSourceComponents()
{
	for (const FViewfinderSourceComponentState& State : HiddenSourceComponents)
	{
		UPrimitiveComponent* Component = State.Component.Get();
		if (!Component)
		{
			continue;
		}

		Component->SetHiddenInGame(State.bWasHiddenInGame);
		Component->SetCollisionEnabled(State.PreviousCollisionEnabled);
	}

	HiddenSourceComponents.Reset();
}

void AViewfinderProjectionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreCapturedSourceComponents();
	Super::EndPlay(EndPlayReason);
}
