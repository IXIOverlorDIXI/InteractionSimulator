// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once



#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "Engine/World.h"
#include "PickupableObject.h"
#include <vector>

#include "InteractionSimulatorCharacter.generated.h"


UCLASS(config=Game)
class AInteractionSimulatorCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ThrowAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchCameraAction;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> UserUI;
	UUserWidget* UserUIInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* InventoryWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InventorySlotWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* InventoryUICollision;

public:
	AInteractionSimulatorCharacter();

	UFUNCTION(Client, Reliable)
	void Interact(const FInputActionValue& Value);

	UFUNCTION(Client, Reliable)
	void SwitchCamera(const FInputActionValue& Value);

	UFUNCTION(Client, Reliable)
	void Throw(const FInputActionValue& Value);

protected:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
			
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionDistance = 200.f;

	UFUNCTION()
	void ShowInteractionMessage(bool bShow) const;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION()
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Client, Reliable)
	void PerformInteraction();

	UFUNCTION(Client, Reliable)
	void PerformThrow();

	UFUNCTION(Server, Reliable)
	void CallPickUp(APickupableObject* PickUpAbleObject);

	UFUNCTION(Server, Reliable)
	void CallOnThrow(APickupableObject* PickUpAbleObject);

	UFUNCTION(NetMulticast, Reliable)
	void AddToInventory(APickupableObject* PickUpAbleObject);

	UFUNCTION(NetMulticast, Reliable)
	void EraseFromInventory(int index);

	UFUNCTION(NetMulticast, Reliable)
	void ClearInventory();

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION()
	FVector GetCurrentCameraForwardVector() const;

	UFUNCTION()
	FVector GetCurrentCameraLocation() const;

	std::vector<APickupableObject*>* Inventory;

private:
	int currentItem{ 0 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", meta = (AllowPrivateAccess = "true"))
	int ItemLimit{ 5 };

	APickupableObject* PickupableObject { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", meta = (AllowPrivateAccess = "true"))
	float distanceToThrowPoint{ 150.f };

	bool bThirdPersonCurrentCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", meta = (AllowPrivateAccess = "true"))
	float ThrowForce { 10.f };

	class UListView* InventoryListView{ nullptr };

	UFUNCTION()
	void CheckForInteraction();
};

