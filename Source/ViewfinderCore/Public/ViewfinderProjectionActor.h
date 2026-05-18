#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "ViewfinderProjectionActor.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPrimitiveComponent;
class UProceduralMeshComponent;
class UViewfinderPhotoData;

USTRUCT()
struct FViewfinderSourceComponentState
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> Component;

	UPROPERTY()
	bool bWasHiddenInGame = false;

	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> PreviousCollisionEnabled = ECollisionEnabled::NoCollision;
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	bool bHideCapturedSourceComponents = true;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<UViewfinderPhotoData> PhotoData = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	void BuildProjection(UViewfinderPhotoData* InPhotoData, const FTransform& PhotoPlaneTransform);

	UFUNCTION(BlueprintCallable, Category = "Viewfinder")
	void RestoreCapturedSourceComponents();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicPhotoMaterial = nullptr;

	UPROPERTY(Transient)
	TArray<FViewfinderSourceComponentState> HiddenSourceComponents;
};
