#include "ViewfinderProjectionMath.h"

float FViewfinderProjectionMath::GetTanHalfHorizontalFov(const FViewfinderProjectionParams& Params)
{
	return FMath::Tan(FMath::DegreesToRadians(Params.HorizontalFovDegrees) * 0.5f);
}

float FViewfinderProjectionMath::GetTanHalfVerticalFov(const FViewfinderProjectionParams& Params)
{
	return GetTanHalfHorizontalFov(Params) / FMath::Max(Params.AspectRatio, KINDA_SMALL_NUMBER);
}

bool FViewfinderProjectionMath::IsPointInsideFrustum(const FVector& CameraSpacePoint, const FViewfinderProjectionParams& Params)
{
	return EvaluateClipPlane(CameraSpacePoint, Params, EClipPlane::Near) >= 0.0f
		&& EvaluateClipPlane(CameraSpacePoint, Params, EClipPlane::Far) >= 0.0f
		&& EvaluateClipPlane(CameraSpacePoint, Params, EClipPlane::Left) >= 0.0f
		&& EvaluateClipPlane(CameraSpacePoint, Params, EClipPlane::Right) >= 0.0f
		&& EvaluateClipPlane(CameraSpacePoint, Params, EClipPlane::Bottom) >= 0.0f
		&& EvaluateClipPlane(CameraSpacePoint, Params, EClipPlane::Top) >= 0.0f;
}

bool FViewfinderProjectionMath::IsSphereInsideOrIntersectingFrustum(const FVector& CameraSpaceCenter, float Radius, const FViewfinderProjectionParams& Params)
{
	const float SafeRadius = FMath::Max(Radius, 0.0f);
	const EClipPlane Planes[] =
	{
		EClipPlane::Near,
		EClipPlane::Far,
		EClipPlane::Left,
		EClipPlane::Right,
		EClipPlane::Bottom,
		EClipPlane::Top
	};

	for (const EClipPlane Plane : Planes)
	{
		const float PlaneDistance = EvaluateClipPlane(CameraSpaceCenter, Params, Plane) / GetClipPlaneNormalLength(Params, Plane);
		if (PlaneDistance < -SafeRadius)
		{
			return false;
		}
	}

	return true;
}

bool FViewfinderProjectionMath::ProjectCameraPointToPhotoUV(const FVector& CameraSpacePoint, const FViewfinderProjectionParams& Params, FVector2D& OutUV)
{
	if (CameraSpacePoint.X <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float TanHalfHorizontal = GetTanHalfHorizontalFov(Params);
	const float TanHalfVertical = GetTanHalfVerticalFov(Params);

	OutUV.X = 0.5 + (CameraSpacePoint.Y / (2.0 * CameraSpacePoint.X * TanHalfHorizontal));
	OutUV.Y = 0.5 - (CameraSpacePoint.Z / (2.0 * CameraSpacePoint.X * TanHalfVertical));
	return true;
}

void FViewfinderProjectionMath::ClipTriangleToFrustum(const FVector& A, const FVector& B, const FVector& C, const FViewfinderProjectionParams& Params, TArray<FVector>& OutPolygon)
{
	TArray<FVector> Current;
	TArray<FVector> Next;
	Current.Reserve(8);
	Next.Reserve(8);
	Current.Add(A);
	Current.Add(B);
	Current.Add(C);

	const EClipPlane Planes[] =
	{
		EClipPlane::Near,
		EClipPlane::Far,
		EClipPlane::Left,
		EClipPlane::Right,
		EClipPlane::Bottom,
		EClipPlane::Top
	};

	for (const EClipPlane Plane : Planes)
	{
		ClipPolygonAgainstPlane(Current, Params, Plane, Next);
		Current = Next;
		Next.Reset();

		if (Current.Num() < 3)
		{
			OutPolygon.Reset();
			return;
		}
	}

	OutPolygon = MoveTemp(Current);
}

FVector FViewfinderProjectionMath::BuildPlacedWorldPosition(const FVector& CameraSpacePoint, const FViewfinderProjectionParams& Params, const FTransform& PhotoPlaneTransform, float PhotoPlaneDistance)
{
	const float ReconstructionScale = PhotoPlaneDistance / FMath::Max(Params.NearClip, KINDA_SMALL_NUMBER);
	const FQuat PlaneRotation = PhotoPlaneTransform.GetRotation();
	const FVector PlaneForward = PlaneRotation.GetAxisX();
	const FVector VirtualCameraOrigin = PhotoPlaneTransform.GetLocation() - PlaneForward * PhotoPlaneDistance;
	return VirtualCameraOrigin + PlaneRotation.RotateVector(CameraSpacePoint * ReconstructionScale);
}

void FViewfinderProjectionMath::TransformPoints(const TArray<FVector>& InPoints, const FTransform& FirstTransform, const FTransform& SecondTransform, TArray<FVector>& OutPoints)
{
	OutPoints.Reset(InPoints.Num());

	for (const FVector& Point : InPoints)
	{
		OutPoints.Add(SecondTransform.TransformPosition(FirstTransform.TransformPosition(Point)));
	}
}

float FViewfinderProjectionMath::EvaluateClipPlane(const FVector& Point, const FViewfinderProjectionParams& Params, EClipPlane Plane)
{
	const float TanHalfHorizontal = GetTanHalfHorizontalFov(Params);
	const float TanHalfVertical = GetTanHalfVerticalFov(Params);

	switch (Plane)
	{
	case EClipPlane::Near:
		return Point.X - Params.NearClip;
	case EClipPlane::Far:
		return Params.FarClip - Point.X;
	case EClipPlane::Left:
		return Point.Y + Point.X * TanHalfHorizontal;
	case EClipPlane::Right:
		return Point.X * TanHalfHorizontal - Point.Y;
	case EClipPlane::Bottom:
		return Point.Z + Point.X * TanHalfVertical;
	case EClipPlane::Top:
		return Point.X * TanHalfVertical - Point.Z;
	default:
		return -1.0f;
	}
}

float FViewfinderProjectionMath::GetClipPlaneNormalLength(const FViewfinderProjectionParams& Params, EClipPlane Plane)
{
	switch (Plane)
	{
	case EClipPlane::Near:
	case EClipPlane::Far:
		return 1.0f;
	case EClipPlane::Left:
	case EClipPlane::Right:
	{
		const float TanHalfHorizontal = GetTanHalfHorizontalFov(Params);
		return FMath::Sqrt(1.0f + FMath::Square(TanHalfHorizontal));
	}
	case EClipPlane::Bottom:
	case EClipPlane::Top:
	{
		const float TanHalfVertical = GetTanHalfVerticalFov(Params);
		return FMath::Sqrt(1.0f + FMath::Square(TanHalfVertical));
	}
	default:
		return 1.0f;
	}
}

void FViewfinderProjectionMath::ClipPolygonAgainstPlane(const TArray<FVector>& InPolygon, const FViewfinderProjectionParams& Params, EClipPlane Plane, TArray<FVector>& OutPolygon)
{
	OutPolygon.Reset();

	if (InPolygon.Num() == 0)
	{
		return;
	}

	FVector Previous = InPolygon.Last();
	float PreviousValue = EvaluateClipPlane(Previous, Params, Plane);
	bool bPreviousInside = PreviousValue >= 0.0f;

	for (const FVector& Current : InPolygon)
	{
		const float CurrentValue = EvaluateClipPlane(Current, Params, Plane);
		const bool bCurrentInside = CurrentValue >= 0.0f;

		if (bCurrentInside != bPreviousInside)
		{
			const float Denominator = PreviousValue - CurrentValue;
			if (!FMath::IsNearlyZero(Denominator))
			{
				const float Alpha = PreviousValue / Denominator;
				OutPolygon.Add(FMath::Lerp(Previous, Current, Alpha));
			}
		}

		if (bCurrentInside)
		{
			OutPolygon.Add(Current);
		}

		Previous = Current;
		PreviousValue = CurrentValue;
		bPreviousInside = bCurrentInside;
	}
}
