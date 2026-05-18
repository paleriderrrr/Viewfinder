#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ViewfinderPhotoItemActor.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UProceduralMeshComponent;
class UViewfinderPhotoData;

UCLASS()
class VIEWFINDERCORE_API AViewfinderPhotoItemActor : public AActor
{
	GENERATED_BODY()

public:
	AViewfinderPhotoItemActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<UProceduralMeshComponent> PhotoPlane = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<UViewfinderPhotoData> PhotoData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	TObjectPtr<UMaterialInterface> PhotoMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	FName TextureParameterName = TEXT("PhotoTexture");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	float PhotoPlaneDistance = 120.0f;

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	void InitializePhoto(UViewfinderPhotoData* InPhotoData, UMaterialInterface* InPhotoMaterial, FName InTextureParameterName, float InPhotoPlaneDistance);

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	void RebuildPhotoPlane();

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicPhotoMaterial = nullptr;
};
