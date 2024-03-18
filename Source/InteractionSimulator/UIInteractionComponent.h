#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UIInteractionComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INTERACTIONSIMULATOR_API UIInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UIInteractionComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere)
    float InteractionDistance = 200.f;

    UPROPERTY(EditAnywhere)
    FString InteractionText = "Press 'E' to interact";

    UPROPERTY(VisibleAnywhere)
    class AActor* CurrentInteractable = nullptr;

    void CheckForInteractables();

    void ShowInteractionText();

    void HideInteractionText();

    void PerformInteraction();
};