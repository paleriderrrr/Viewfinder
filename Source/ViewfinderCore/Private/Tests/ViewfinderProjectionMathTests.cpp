#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ViewfinderProjectionMath.h"

namespace
{
	FViewfinderProjectionParams MakeUnitProjectionParams()
	{
		FViewfinderProjectionParams Params;
		Params.HorizontalFovDegrees = 90.0f;
		Params.AspectRatio = 1.0f;
		Params.NearClip = 10.0f;
		Params.FarClip = 1000.0f;
		return Params;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewfinderProjectionCenterUvTest, "Viewfinder.ProjectionMath.ProjectCenterToMiddle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FViewfinderProjectionCenterUvTest::RunTest(const FString& Parameters)
{
	const FViewfinderProjectionParams Params = MakeUnitProjectionParams();
	FVector2D UV;
	const bool bProjected = FViewfinderProjectionMath::ProjectCameraPointToPhotoUV(FVector(100.0, 0.0, 0.0), Params, UV);

	TestTrue(TEXT("center point projects"), bProjected);
	TestEqual(TEXT("center U"), UV.X, 0.5);
	TestEqual(TEXT("center V"), UV.Y, 0.5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewfinderProjectionPlacementTest, "Viewfinder.ProjectionMath.NearPlaneMapsToPhotoPlane", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FViewfinderProjectionPlacementTest::RunTest(const FString& Parameters)
{
	const FViewfinderProjectionParams Params = MakeUnitProjectionParams();
	const FTransform PlaneTransform(FRotator::ZeroRotator, FVector::ZeroVector);

	const FVector NearPoint = FViewfinderProjectionMath::BuildPlacedWorldPosition(FVector(10.0, 0.0, 0.0), Params, PlaneTransform, 100.0f);
	const FVector DeeperPoint = FViewfinderProjectionMath::BuildPlacedWorldPosition(FVector(20.0, 0.0, 0.0), Params, PlaneTransform, 100.0f);

	TestTrue(TEXT("near plane maps to photo plane origin"), NearPoint.Equals(FVector::ZeroVector, UE_SMALL_NUMBER));
	TestTrue(TEXT("depth reconstructs forward from the photo plane"), DeeperPoint.Equals(FVector(100.0, 0.0, 0.0), UE_SMALL_NUMBER));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewfinderProjectionClipInsideTest, "Viewfinder.ProjectionMath.ClipInsideTriangle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FViewfinderProjectionClipInsideTest::RunTest(const FString& Parameters)
{
	const FViewfinderProjectionParams Params = MakeUnitProjectionParams();
	TArray<FVector> Polygon;

	FViewfinderProjectionMath::ClipTriangleToFrustum(
		FVector(100.0, -10.0, -10.0),
		FVector(100.0, 10.0, -10.0),
		FVector(100.0, 0.0, 10.0),
		Params,
		Polygon);

	TestEqual(TEXT("inside triangle remains a triangle"), Polygon.Num(), 3);
	for (const FVector& Point : Polygon)
	{
		TestTrue(TEXT("point remains inside frustum"), FViewfinderProjectionMath::IsPointInsideFrustum(Point, Params));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewfinderProjectionClipPartialTest, "Viewfinder.ProjectionMath.ClipPartialTriangle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FViewfinderProjectionClipPartialTest::RunTest(const FString& Parameters)
{
	const FViewfinderProjectionParams Params = MakeUnitProjectionParams();
	TArray<FVector> Polygon;

	FViewfinderProjectionMath::ClipTriangleToFrustum(
		FVector(100.0, -200.0, 0.0),
		FVector(100.0, 0.0, 50.0),
		FVector(100.0, 0.0, -50.0),
		Params,
		Polygon);

	TestTrue(TEXT("partial triangle leaves a clipped polygon"), Polygon.Num() >= 3);
	for (const FVector& Point : Polygon)
	{
		TestTrue(TEXT("clipped point is inside frustum"), FViewfinderProjectionMath::IsPointInsideFrustum(Point, Params));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewfinderProjectionSphereCullTest, "Viewfinder.ProjectionMath.SphereFrustumCull", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FViewfinderProjectionSphereCullTest::RunTest(const FString& Parameters)
{
	const FViewfinderProjectionParams Params = MakeUnitProjectionParams();

	TestTrue(TEXT("sphere in front intersects frustum"), FViewfinderProjectionMath::IsSphereInsideOrIntersectingFrustum(FVector(100.0, 0.0, 0.0), 10.0f, Params));
	TestFalse(TEXT("sphere behind camera is outside frustum"), FViewfinderProjectionMath::IsSphereInsideOrIntersectingFrustum(FVector(-100.0, 0.0, 0.0), 10.0f, Params));
	TestFalse(TEXT("sphere beyond far clip is outside frustum"), FViewfinderProjectionMath::IsSphereInsideOrIntersectingFrustum(FVector(1200.0, 0.0, 0.0), 10.0f, Params));
	TestTrue(TEXT("sphere touching side plane intersects frustum"), FViewfinderProjectionMath::IsSphereInsideOrIntersectingFrustum(FVector(100.0, 105.0, 0.0), 10.0f, Params));
	return true;
}

#endif
