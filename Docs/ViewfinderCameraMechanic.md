# Viewfinder Camera Mechanic

This project implements the core code path for a Viewfinder-style camera:

1. `AViewfinderCameraActor::TakePhoto` captures a render target through `USceneCaptureComponent2D`.
2. Actors with `UViewfinderCaptureComponent` are filtered against the camera frustum, then sampled from their `UStaticMeshComponent` sections.
3. Source triangles are transformed into camera space, clipped against the photo frustum, and assigned projected UVs.
4. `AViewfinderPhotoItemActor` renders a rectangular preview plane using the captured render target.
5. `AViewfinderProjectionActor::BuildProjection` reconstructs the clipped geometry from the placed photo plane and assigns the same captured render target as projected texture.
6. `AViewfinderCameraActor::FindPhotoPlacementTransform` traces from the camera and creates a placement transform where local `+X` points into the reconstructed scene.
7. By default, `AViewfinderProjectionActor` hides and disables collision on the captured source components when the reconstructed photo is placed. Destroying the projection actor restores those components.

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

The current source-removal behavior is component-level hiding, not mesh boolean subtraction. This mirrors the reference implementations' placement phase at the gameplay level while keeping the first C++ pass explicit: captured source components are hidden and collision-disabled through `bHideCapturedSourceComponentsOnPlace` on the camera and `bHideCapturedSourceComponents` on the projection actor.

## Reference Parity Notes

- `StrangeDS/ViewFinderRe` uses a generated view-frustum component with DynamicMesh/GeometryScript and requires CPU access on static meshes. This project now applies the same frustum-first filtering idea before sampling mesh sections.
- `EllishYe/ViewfinderRemake` cuts capturable meshes by camera frustum, rebuilds a copied photo root, and removes source objects in the placement frustum. This project now records source components in `UViewfinderPhotoData` and suppresses them when a projection is placed.

## Placement Convention

The placed photo actor's local `+X` direction points into the reconstructed scene. Captured depth starts at the photo plane and extends forward.
