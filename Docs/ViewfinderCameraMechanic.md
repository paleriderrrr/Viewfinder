# Viewfinder Camera Mechanic

This project implements the core code path for a Viewfinder-style camera:

1. `AViewfinderCameraActor::TakePhoto` captures a render target through `USceneCaptureComponent2D`.
2. Actors with `UViewfinderCaptureComponent` are sampled from their `UStaticMeshComponent` sections.
3. Source triangles are transformed into camera space, clipped against the photo frustum, and assigned projected UVs.
4. `AViewfinderPhotoItemActor` renders a rectangular preview plane using the captured render target.
5. `AViewfinderProjectionActor::BuildProjection` reconstructs the clipped geometry from the placed photo plane and assigns the same captured render target as projected texture.
6. `AViewfinderCameraActor::FindPhotoPlacementTransform` traces from the camera and creates a placement transform where local `+X` points into the reconstructed scene.

## Required Material

Create one material for both `PhotoItemActor` and `ProjectionActor`:

- Texture parameter name: `PhotoTexture`
- Base Color: `TextureSampleParameter2D(PhotoTexture).RGB`
- Opacity or Mask: optional
- Two Sided: enabled

Assign this material to `AViewfinderCameraActor::PhotoMaterial`. The code intentionally logs an error if this material is missing.

## Capturable Meshes

Add `UViewfinderCaptureComponent` to every actor that should be reconstructed from a photo.

Static mesh assets must have `Allow CPU Access` enabled. Unreal cannot expose section vertex data at runtime without CPU mesh data.

## Placement Convention

The placed photo actor's local `+X` direction points into the reconstructed scene. Captured depth starts at the photo plane and extends forward.
