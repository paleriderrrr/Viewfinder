#pragma once

#include "CoreMinimal.h"
#include "ViewfinderProjectionMath.generated.h"

USTRUCT(BlueprintType)
struct FViewfinderProjectionParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	float HorizontalFovDegrees = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	float AspectRatio = 16.0f / 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	float NearClip = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	float FarClip = 5000.0f;
};

USTRUCT(BlueprintType)
struct FViewfinderCapturedTriangle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	FVector A = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	FVector B = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	FVector C = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	FVector2D UVA = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	FVector2D UVB = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Viewfinder")
	FVector2D UVC = FVector2D::ZeroVector;
};

class VIEWFINDERCORE_API FViewfinderProjectionMath
{
public:
	static float GetTanHalfHorizontalFov(const FViewfinderProjectionParams& Params);
	static float GetTanHalfVerticalFov(const FViewfinderProjectionParams& Params);

	static bool IsPointInsideFrustum(const FVector& CameraSpacePoint, const FViewfinderProjectionParams& Params);
	static bool IsSphereInsideOrIntersectingFrustum(const FVector& CameraSpaceCenter, float Radius, const FViewfinderProjectionParams& Params);
	static bool ProjectCameraPointToPhotoUV(const FVector& CameraSpacePoint, const FViewfinderProjectionParams& Params, FVector2D& OutUV);
	static void ClipTriangleToFrustum(const FVector& A, const FVector& B, const FVector& C, const FViewfinderProjectionParams& Params, TArray<FVector>& OutPolygon);
	static FVector BuildPlacedWorldPosition(const FVector& CameraSpacePoint, const FViewfinderProjectionParams& Params, const FTransform& PhotoPlaneTransform, float PhotoPlaneDistance);

private:
	enum class EClipPlane : uint8
	{
		Near,
		Far,
		Left,
		Right,
		Bottom,
		Top
	};

	static float EvaluateClipPlane(const FVector& Point, const FViewfinderProjectionParams& Params, EClipPlane Plane);
	static void ClipPolygonAgainstPlane(const TArray<FVector>& InPolygon, const FViewfinderProjectionParams& Params, EClipPlane Plane, TArray<FVector>& OutPolygon);
};
