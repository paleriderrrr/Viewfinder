#pragma once

#include "CoreMinimal.h"
#include "ViewfinderProjectionMath.h"
#include "ViewfinderPhotoData.generated.h"

class UTextureRenderTarget2D;

UCLASS(BlueprintType)
class VIEWFINDERCORE_API UViewfinderPhotoData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	FViewfinderProjectionParams ProjectionParams;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	FTransform CaptureTransform = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	TArray<FViewfinderCapturedTriangle> Triangles;

	UFUNCTION(BlueprintPure, Category = "Viewfinder")
	int32 GetTriangleCount() const { return Triangles.Num(); }
};
