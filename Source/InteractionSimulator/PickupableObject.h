#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Net/UnrealNetwork.h"
#include "PickupableObject.generated.h"

UCLASS()
class INTERACTIONSIMULATOR_API APickupableObject : public AActor
{
    GENERATED_BODY()

public:
    APickupableObject(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;

    UFUNCTION(NetMulticast, Reliable)
    void OnPickup();

    UFUNCTION(NetMulticast, Reliable)
    void OnThrow(FVector Position, FVector ThrowDirection, float ThrowForce);

    UPROPERTY(EditAnywhere, Category = "Inventory")
    UTexture2D* ImageBrush;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    UStaticMeshComponent* StaticMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tick")
    double InterpolationParam{ 0.01 };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tick")
    double PrecisionParam{ 1 };

    UFUNCTION(NetMulticast, Reliable)
    void ObjectTransformReplication(FTransform transform, FVector velocity);

    void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, Category = "Tick")
    float TimeSinceLastTick;

    UPROPERTY(EditAnywhere, Category = "Tick")
    float TickFrequency;

    UPROPERTY(EditAnywhere, Category = "Tick")
    float TimeSinceLastUpdateTick;
    UPROPERTY(EditAnywhere, Category = "Tick")
    float UpdateTickFrequency;

    UPROPERTY(EditAnywhere, Category = "Tick")
    FTransform TrueTransform;

    UPROPERTY(EditAnywhere, Category = "Tick")
    FVector TrueVelocity;

    UPROPERTY(EditAnywhere, Category = "Tick")
    FTransform CurrentTransform;

    UPROPERTY(EditAnywhere, Category = "Tick")
    FVector CurrentVelocity;

    UFUNCTION()
    void Interpolate(double value_1, double value_2, double interpolation, double& result) const;
    UFUNCTION()
    FVector InterpolateFVector(FVector value_1, FVector value_2, double interpolation) const;
    UFUNCTION()
    FQuat InterpolateFQuat(FQuat value_1, FQuat value_2, double interpolation) const;

    UFUNCTION()
    FVector GetInterpolatedVelocity(FVector point_1, FVector point_2, double interpolation) const;

    UFUNCTION()
    FVector VectorSum(FVector value_1, FVector value_2) const;

    UFUNCTION()
    FVector VectorSubstract(FVector value_1, FVector value_2) const;

    UFUNCTION()
    FVector VectorDivision(FVector value_1, double division) const;

    UFUNCTION()
    bool IsVectorEqualWithPrecision(FVector value_1, FVector value_2, bool withPrecision) const;
};

/*UPROPERTY(VisibleAnywhere, Category = "Components")
UWidgetComponent *PickupWidget;*/
