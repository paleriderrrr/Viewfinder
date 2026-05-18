#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ViewfinderCaptureComponent.generated.h"

UCLASS(ClassGroup = (Viewfinder), meta = (BlueprintSpawnableComponent))
class VIEWFINDERCORE_API UViewfinderCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UViewfinderCaptureComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewfinder")
	bool bCaptureEnabled = true;
};
