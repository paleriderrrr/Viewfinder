#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ViewfinderCameraActor.generated.h"

class AViewfinderPhotoItemActor;
class AViewfinderProjectionActor;
class UCameraComponent;
class UMaterialInterface;
class USceneCaptureComponent2D;
class UStaticMeshComponent;
class UViewfinderPhotoData;

UCLASS()
class VIEWFINDERCORE_API AViewfinderCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AViewfinderCameraActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<UCameraComponent> Camera = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Capture")
	FIntPoint CaptureResolution = FIntPoint(1024, 768);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Capture")
	float NearClipDistance = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Capture")
	float MaxCaptureDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Capture")
	bool bCaptureOnlyMarkedActors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Placement")
	float HeldPhotoDistance = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Placement")
	float PhotoPlaneDistance = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Placement")
	float PlacementSurfaceOffset = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Placement")
	TSubclassOf<AViewfinderPhotoItemActor> PhotoItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Placement")
	TSubclassOf<AViewfinderProjectionActor> ProjectionActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Materials")
	TObjectPtr<UMaterialInterface> PhotoMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder|Materials")
	FName TextureParameterName = TEXT("PhotoTexture");

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	UViewfinderPhotoData* TakePhoto();

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	AViewfinderPhotoItemActor* SpawnPhotoItem(UViewfinderPhotoData* PhotoData);

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	AViewfinderProjectionActor* PlacePhoto(UViewfinderPhotoData* PhotoData, const FTransform& PhotoPlaneTransform);

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	bool FindPhotoPlacementTransform(float TraceDistance, FTransform& OutPhotoPlaneTransform) const;

private:
	void CaptureMarkedGeometry(UViewfinderPhotoData& PhotoData) const;
	void CaptureStaticMeshComponent(const UStaticMeshComponent& MeshComponent, UViewfinderPhotoData& PhotoData) const;
};
