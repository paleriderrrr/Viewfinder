#include "ViewfinderCameraActor.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"
#include "ViewfinderCaptureComponent.h"
#include "ViewfinderPhotoData.h"
#include "ViewfinderPhotoItemActor.h"
#include "ViewfinderProjectionActor.h"
#include "ViewfinderProjectionMath.h"

DEFINE_LOG_CATEGORY_STATIC(LogViewfinderCamera, Log, All);

AViewfinderCameraActor::AViewfinderCameraActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SceneRoot);
	Camera->FieldOfView = 90.0f;

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(Camera);
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->FOVAngle = Camera->FieldOfView;

	PhotoItemClass = AViewfinderPhotoItemActor::StaticClass();
	ProjectionActorClass = AViewfinderProjectionActor::StaticClass();
}

UViewfinderPhotoData* AViewfinderCameraActor::TakePhoto()
{
	if (!GetWorld())
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("TakePhoto failed: world is null."));
		return nullptr;
	}

	if (CaptureResolution.X <= 0 || CaptureResolution.Y <= 0)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("TakePhoto failed: CaptureResolution must be positive. Current value: %d x %d."), CaptureResolution.X, CaptureResolution.Y);
		return nullptr;
	}

	if (NearClipDistance <= KINDA_SMALL_NUMBER || MaxCaptureDistance <= NearClipDistance)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("TakePhoto failed: NearClipDistance and MaxCaptureDistance are invalid. Near=%f Far=%f."), NearClipDistance, MaxCaptureDistance);
		return nullptr;
	}

	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	RenderTarget->ClearColor = FLinearColor::Transparent;
	RenderTarget->InitAutoFormat(CaptureResolution.X, CaptureResolution.Y);
	RenderTarget->UpdateResourceImmediate(true);

	SceneCapture->TextureTarget = RenderTarget;
	SceneCapture->FOVAngle = Camera->FieldOfView;
	SceneCapture->MaxViewDistanceOverride = MaxCaptureDistance;
	SceneCapture->CaptureScene();

	UViewfinderPhotoData* PhotoData = NewObject<UViewfinderPhotoData>(this);
	PhotoData->RenderTarget = RenderTarget;
	PhotoData->CaptureTransform = Camera->GetComponentTransform();
	PhotoData->ProjectionParams.HorizontalFovDegrees = Camera->FieldOfView;
	PhotoData->ProjectionParams.AspectRatio = static_cast<float>(CaptureResolution.X) / static_cast<float>(CaptureResolution.Y);
	PhotoData->ProjectionParams.NearClip = NearClipDistance;
	PhotoData->ProjectionParams.FarClip = MaxCaptureDistance;

	CaptureMarkedGeometry(*PhotoData);
	return PhotoData;
}

AViewfinderPhotoItemActor* AViewfinderCameraActor::SpawnPhotoItem(UViewfinderPhotoData* PhotoData)
{
	if (!GetWorld())
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("SpawnPhotoItem failed: world is null."));
		return nullptr;
	}

	if (!PhotoData)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("SpawnPhotoItem failed: PhotoData is null."));
		return nullptr;
	}

	if (!PhotoItemClass)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("SpawnPhotoItem failed: PhotoItemClass is not assigned."));
		return nullptr;
	}

	if (PhotoPlaneDistance <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("SpawnPhotoItem failed: PhotoPlaneDistance must be positive. Current value: %f."), PhotoPlaneDistance);
		return nullptr;
	}

	const FVector SpawnLocation = Camera->GetComponentLocation() + Camera->GetForwardVector() * HeldPhotoDistance;
	const FRotator SpawnRotation = Camera->GetComponentRotation();
	AViewfinderPhotoItemActor* PhotoItem = GetWorld()->SpawnActor<AViewfinderPhotoItemActor>(PhotoItemClass, SpawnLocation, SpawnRotation);

	if (!PhotoItem)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("SpawnPhotoItem failed: actor spawn returned null."));
		return nullptr;
	}

	PhotoItem->InitializePhoto(PhotoData, PhotoMaterial, TextureParameterName, PhotoPlaneDistance);
	return PhotoItem;
}

AViewfinderProjectionActor* AViewfinderCameraActor::PlacePhoto(UViewfinderPhotoData* PhotoData, const FTransform& PhotoPlaneTransform)
{
	if (!GetWorld())
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("PlacePhoto failed: world is null."));
		return nullptr;
	}

	if (!PhotoData)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("PlacePhoto failed: PhotoData is null."));
		return nullptr;
	}

	if (!ProjectionActorClass)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("PlacePhoto failed: ProjectionActorClass is not assigned."));
		return nullptr;
	}

	if (PhotoPlaneDistance <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("PlacePhoto failed: PhotoPlaneDistance must be positive. Current value: %f."), PhotoPlaneDistance);
		return nullptr;
	}

	AViewfinderProjectionActor* ProjectionActor = GetWorld()->SpawnActor<AViewfinderProjectionActor>(ProjectionActorClass, PhotoPlaneTransform);
	if (!ProjectionActor)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("PlacePhoto failed: actor spawn returned null."));
		return nullptr;
	}

	ProjectionActor->PhotoMaterial = PhotoMaterial;
	ProjectionActor->TextureParameterName = TextureParameterName;
	ProjectionActor->PhotoPlaneDistance = PhotoPlaneDistance;
	ProjectionActor->bHideCapturedSourceComponents = bHideCapturedSourceComponentsOnPlace;
	ProjectionActor->BuildProjection(PhotoData, PhotoPlaneTransform);
	return ProjectionActor;
}

bool AViewfinderCameraActor::FindPhotoPlacementTransform(float TraceDistance, FTransform& OutPhotoPlaneTransform) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("FindPhotoPlacementTransform failed: world is null."));
		return false;
	}

	if (TraceDistance <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("FindPhotoPlacementTransform failed: TraceDistance must be positive. Current value: %f."), TraceDistance);
		return false;
	}

	const FVector TraceStart = Camera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + Camera->GetForwardVector() * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ViewfinderPhotoPlacement), false, this);
	const bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	if (!bHit)
	{
		return false;
	}

	const FVector PhotoForward = (-Hit.ImpactNormal).GetSafeNormal();
	if (PhotoForward.IsNearlyZero())
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("FindPhotoPlacementTransform failed: hit normal is invalid."));
		return false;
	}

	const FRotator PhotoRotation = FRotationMatrix::MakeFromXZ(PhotoForward, FVector::UpVector).Rotator();
	const FVector PhotoLocation = Hit.ImpactPoint + Hit.ImpactNormal * PlacementSurfaceOffset;
	OutPhotoPlaneTransform = FTransform(PhotoRotation, PhotoLocation);
	return true;
}

void AViewfinderCameraActor::CaptureMarkedGeometry(UViewfinderPhotoData& PhotoData) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogViewfinderCamera, Error, TEXT("CaptureMarkedGeometry failed: world is null."));
		return;
	}

	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (!Actor || Actor == this)
		{
			continue;
		}

		const UViewfinderCaptureComponent* CaptureComponent = Actor->FindComponentByClass<UViewfinderCaptureComponent>();
		const bool bActorIsMarked = CaptureComponent && CaptureComponent->bCaptureEnabled;
		if (bCaptureOnlyMarkedActors && !bActorIsMarked)
		{
			continue;
		}

		TInlineComponentArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents(StaticMeshComponents);

		for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
		{
			if (!MeshComponent || !MeshComponent->IsRegistered() || !MeshComponent->IsVisible())
			{
				continue;
			}

			if (!IsStaticMeshComponentInPhotoFrustum(*MeshComponent, PhotoData))
			{
				continue;
			}

			CaptureStaticMeshComponent(*MeshComponent, PhotoData);
		}
	}
}

void AViewfinderCameraActor::CaptureStaticMeshComponent(UStaticMeshComponent& MeshComponent, UViewfinderPhotoData& PhotoData) const
{
	UStaticMesh* StaticMesh = MeshComponent.GetStaticMesh();
	if (!StaticMesh)
	{
		return;
	}

	const int32 LODIndex = 0;
	const int32 SectionCount = StaticMesh->GetNumSections(LODIndex);
	if (SectionCount <= 0)
	{
		UE_LOG(LogViewfinderCamera, Warning, TEXT("Static mesh %s has no sections at LOD 0."), *StaticMesh->GetName());
		return;
	}

	const FTransform ComponentTransform = MeshComponent.GetComponentTransform();
	const FTransform CaptureWorldToCamera = PhotoData.CaptureTransform.Inverse();
	const int32 OriginalTriangleCount = PhotoData.Triangles.Num();

	for (int32 SectionIndex = 0; SectionIndex < SectionCount; ++SectionIndex)
	{
		TArray<FVector> MeshVertices;
		TArray<int32> MeshTriangles;
		TArray<FVector> MeshNormals;
		TArray<FVector2D> MeshUVs;
		TArray<FProcMeshTangent> MeshTangents;

		UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(StaticMesh, LODIndex, SectionIndex, MeshVertices, MeshTriangles, MeshNormals, MeshUVs, MeshTangents);

		if (MeshVertices.Num() == 0 || MeshTriangles.Num() == 0)
		{
			UE_LOG(LogViewfinderCamera, Warning, TEXT("Could not read section %d from %s. Enable Allow CPU Access on the static mesh asset."), SectionIndex, *StaticMesh->GetName());
			continue;
		}

		TArray<FVector> CameraSpaceVertices;
		FViewfinderProjectionMath::TransformPoints(MeshVertices, ComponentTransform, CaptureWorldToCamera, CameraSpaceVertices);

		for (int32 TriangleIndex = 0; TriangleIndex + 2 < MeshTriangles.Num(); TriangleIndex += 3)
		{
			const int32 IndexA = MeshTriangles[TriangleIndex];
			const int32 IndexB = MeshTriangles[TriangleIndex + 1];
			const int32 IndexC = MeshTriangles[TriangleIndex + 2];

			if (!CameraSpaceVertices.IsValidIndex(IndexA) || !CameraSpaceVertices.IsValidIndex(IndexB) || !CameraSpaceVertices.IsValidIndex(IndexC))
			{
				UE_LOG(LogViewfinderCamera, Error, TEXT("Static mesh %s section %d contains an invalid triangle index."), *StaticMesh->GetName(), SectionIndex);
				continue;
			}

			const FVector& CameraA = CameraSpaceVertices[IndexA];
			const FVector& CameraB = CameraSpaceVertices[IndexB];
			const FVector& CameraC = CameraSpaceVertices[IndexC];

			TArray<FVector> ClippedPolygon;
			FViewfinderProjectionMath::ClipTriangleToFrustum(CameraA, CameraB, CameraC, PhotoData.ProjectionParams, ClippedPolygon);

			if (ClippedPolygon.Num() < 3)
			{
				continue;
			}

			for (int32 FanIndex = 1; FanIndex + 1 < ClippedPolygon.Num(); ++FanIndex)
			{
				FViewfinderCapturedTriangle CapturedTriangle;
				CapturedTriangle.A = ClippedPolygon[0];
				CapturedTriangle.B = ClippedPolygon[FanIndex];
				CapturedTriangle.C = ClippedPolygon[FanIndex + 1];

				if (!FViewfinderProjectionMath::ProjectCameraPointToPhotoUV(CapturedTriangle.A, PhotoData.ProjectionParams, CapturedTriangle.UVA)
					|| !FViewfinderProjectionMath::ProjectCameraPointToPhotoUV(CapturedTriangle.B, PhotoData.ProjectionParams, CapturedTriangle.UVB)
					|| !FViewfinderProjectionMath::ProjectCameraPointToPhotoUV(CapturedTriangle.C, PhotoData.ProjectionParams, CapturedTriangle.UVC))
				{
					UE_LOG(LogViewfinderCamera, Error, TEXT("Clipped triangle from %s could not be projected to photo UVs."), *StaticMesh->GetName());
					continue;
				}

				PhotoData.Triangles.Add(CapturedTriangle);
			}
		}
	}

	if (PhotoData.Triangles.Num() > OriginalTriangleCount)
	{
		RecordCapturedSourceComponent(MeshComponent, PhotoData);
	}
}

bool AViewfinderCameraActor::IsStaticMeshComponentInPhotoFrustum(const UStaticMeshComponent& MeshComponent, const UViewfinderPhotoData& PhotoData) const
{
	const FTransform CaptureWorldToCamera = PhotoData.CaptureTransform.Inverse();
	const FVector CameraSpaceCenter = CaptureWorldToCamera.TransformPosition(MeshComponent.Bounds.Origin);
	return FViewfinderProjectionMath::IsSphereInsideOrIntersectingFrustum(CameraSpaceCenter, MeshComponent.Bounds.SphereRadius, PhotoData.ProjectionParams);
}

void AViewfinderCameraActor::RecordCapturedSourceComponent(UStaticMeshComponent& MeshComponent, UViewfinderPhotoData& PhotoData) const
{
	for (const FViewfinderCapturedSourceComponent& ExistingComponent : PhotoData.CapturedSourceComponents)
	{
		if (ExistingComponent.Component == &MeshComponent)
		{
			return;
		}
	}

	FViewfinderCapturedSourceComponent CapturedComponent;
	CapturedComponent.Component = &MeshComponent;
	PhotoData.CapturedSourceComponents.Add(CapturedComponent);
}
