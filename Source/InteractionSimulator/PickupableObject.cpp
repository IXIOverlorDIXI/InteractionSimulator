#include "PickupableObject.h"

#include "VectorTypes.h"

APickupableObject::APickupableObject(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
    RootComponent = StaticMeshComponent;

    bAlwaysRelevant = true;

    PrimaryActorTick.bCanEverTick = true;

    TimeSinceLastTick = 0.0f;
    TickFrequency = 1.f;

    TimeSinceLastUpdateTick = 0.0f;
    UpdateTickFrequency = 5.f;

    StaticMeshComponent->SetSimulatePhysics(true);
    SetReplicates(true);

    InterpolationParam = 0.01;
}

void APickupableObject::BeginPlay()
{
    Super::BeginPlay();

    SetReplicateMovement(true);

    CurrentTransform = StaticMeshComponent->GetComponentTransform();
    CurrentVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();

    TrueTransform = StaticMeshComponent->GetComponentTransform();
    TrueVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();
}

void APickupableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    TimeSinceLastTick += DeltaTime;
    TimeSinceLastUpdateTick += DeltaTime;

    if (TimeSinceLastTick >= 1.0f / TickFrequency)
    {
        TimeSinceLastTick = 0.0f;

        if (GetLocalRole() == ROLE_Authority)
        {
            FTransform const transform = StaticMeshComponent->GetComponentTransform();
            FVector const velocity = StaticMeshComponent->GetPhysicsLinearVelocity();

            ObjectTransformReplication(transform, velocity);
        }
    }

    if (GetLocalRole() != ROLE_Authority)
    {
  //      FVector currentPosition = StaticMeshComponent->GetComponentTransform().GetLocation();
  //      FQuat currentRotation = StaticMeshComponent->GetComponentTransform().GetRotation();
  //      FVector currentVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();

  //      FVector truePosition = TrueTransform.GetLocation();
  //      FQuat trueRotation = TrueTransform.GetRotation();

  //      FVector newPosition = InterpolateFVector(currentPosition, truePosition, InterpolationParam);
  //      FQuat newRotation = InterpolateFQuat(currentRotation, trueRotation, InterpolationParam);
  //      FVector newVelocity = InterpolateFVector(currentVelocity, TrueVelocity, InterpolationParam);

  //      //StaticMeshComponent->SetWorldLocationAndRotation(newPosition, newRotation);
  //      StaticMeshComponent->SetWorldRotation(newRotation);

		//newVelocity = VectorSum(
  //          newVelocity,
		//	GetInterpolatedVelocity(currentPosition, truePosition, 1));

  //      if(newVelocity.X <= PrecisionParam 
  //          && newVelocity.Y <= PrecisionParam 
  //          && newVelocity.Z <= PrecisionParam)
  //      {
  //          newVelocity = GetInterpolatedVelocity(currentPosition, truePosition, 1);
  //      }

		//StaticMeshComponent->SetPhysicsLinearVelocity(newVelocity);
  //      
  //      

  //      CurrentTransform = StaticMeshComponent->GetComponentTransform();
  //      CurrentVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();
    }

}

void APickupableObject::ObjectTransformReplication_Implementation(
    FTransform const transform, const FVector velocity)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        return;
    }

    CurrentTransform = StaticMeshComponent->GetComponentTransform();
    CurrentVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();

    TrueTransform = transform;
    TrueVelocity = velocity;

    StaticMeshComponent->SetWorldTransform(transform);
    StaticMeshComponent->SetPhysicsLinearVelocity(velocity);
}

FVector APickupableObject::GetInterpolatedVelocity(FVector point_1, FVector point_2, double interpolation) const
{
	if(!IsVectorEqualWithPrecision(point_1, point_2, true))
	{
        FVector pathVector = VectorSubstract(point_2, point_1);

        FVector InterpolatedVelocity = VectorDivision(pathVector, 1 / interpolation);

        return InterpolatedVelocity;
	}
    else
    {
        return FVector(0, 0, 0);
    }
}

bool APickupableObject::IsVectorEqualWithPrecision(FVector value_1, FVector value_2, bool withPrecision) const
{
    bool result = true;

    result = result && (abs(abs(value_1.X) - abs(value_2.X)) <= (withPrecision ? PrecisionParam : 0));
    result = result && (abs(abs(value_1.Y) - abs(value_2.Y)) <= (withPrecision ? PrecisionParam : 0));
    result = result && (abs(abs(value_1.Z) - abs(value_2.Z)) <= (withPrecision ? PrecisionParam : 0));

    return result;
}

FVector APickupableObject::VectorSum(FVector value_1, FVector value_2) const
{
    FVector newValue = FVector(
        value_1.X + value_2.X,
        value_1.Y + value_2.Y,
        value_1.Z + value_2.Z
    );

    return newValue;
}

FVector APickupableObject::VectorDivision(FVector value_1, double division) const
{
	return FVector(value_1.X / division, value_1.Y / division, value_1.Z / division);
}

FVector APickupableObject::VectorSubstract(FVector value_1, FVector value_2) const
{
    FVector newValue = FVector(
        value_1.X - value_2.X,
        value_1.Y - value_2.Y,
        value_1.Z - value_2.Z
    );

    return newValue;
}

FQuat APickupableObject::InterpolateFQuat(FQuat value_1, FQuat value_2, double interpolation) const
{
    double temp = 0;

    FQuat result = FQuat();

    Interpolate(value_1.X, value_2.X, interpolation, temp);
    result.X = temp;

    Interpolate(value_1.Y, value_2.Y, interpolation, temp);
    result.Y = temp;

    Interpolate(value_1.Z, value_2.Z, interpolation, temp);
    result.Z = temp;

    Interpolate(value_1.W, value_2.W, interpolation, temp);
    result.W = temp;

    return result;
}

FVector APickupableObject::InterpolateFVector(FVector value_1, FVector value_2, double interpolation) const
{
    double temp = 0;
    FVector result = FVector();

    Interpolate(value_1.X, value_2.X, interpolation, temp);
    result.X = temp;

    Interpolate(value_1.Y, value_2.Y, interpolation, temp);
    result.Y = temp;

    Interpolate(value_1.Z, value_2.Z, interpolation, temp);
    result.Z = temp;

    return result;
}

void APickupableObject::Interpolate(double value_1, double value_2, double interpolation, double& result) const
{
    if(abs(value_2 - value_1) < 0.1)
    {
        result = value_2;
    }
    else
    {
        result = (1 - interpolation) * value_1 + value_2 * interpolation;
    }
}


void APickupableObject::OnPickup_Implementation()
{
    SetActorHiddenInGame(true);
    SetActorTickEnabled(false);
    SetActorEnableCollision(false);
}

void APickupableObject::OnThrow_Implementation(FVector Position, FVector ThrowDirection, float ThrowForce)
{
    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);
    SetActorEnableCollision(true);

    FTransform transform = GetTransform();

    StaticMeshComponent->SetWorldLocationAndRotation(Position, ThrowDirection.Rotation());
    StaticMeshComponent->SetPhysicsLinearVelocity(ThrowDirection * ThrowForce);

    CurrentTransform = StaticMeshComponent->GetComponentTransform();
    CurrentVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();

    TrueTransform = StaticMeshComponent->GetComponentTransform();
    TrueVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();
}
