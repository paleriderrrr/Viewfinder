#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ViewfinderProjectionActor.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UProceduralMeshComponent;
class UViewfinderPhotoData;

UCLASS()
class VIEWFINDERCORE_API AViewfinderProjectionActor : public AActor
{
	GENERATED_BODY()

public:
	AViewfinderProjectionActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<UProceduralMeshComponent> ProjectionMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	TObjectPtr<UMaterialInterface> PhotoMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	FName TextureParameterName = TEXT("PhotoTexture");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	float PhotoPlaneDistance = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	bool bCreateCollision = true;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<UViewfinderPhotoData> PhotoData = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	void BuildProjection(UViewfinderPhotoData* InPhotoData, const FTransform& PhotoPlaneTransform);

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicPhotoMaterial = nullptr;
};
