// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionSimulatorCharacter.h"

#include <string>

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/SphereComponent.h"


//////////////////////////////////////////////////////////////////////////
// AInteractionSimulatorCharacter

AInteractionSimulatorCharacter::AInteractionSimulatorCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetAutoActivate(false);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->bLockToHmd = true;
	FirstPersonCamera->SetAutoActivate(false);
	USkeletalMeshComponent* mesh = GetMesh();
	FirstPersonCamera->SetupAttachment(mesh, "head");


	InventoryWidgetInstance = CreateDefaultSubobject<UWidgetComponent>(TEXT("InventoryWidgetComponent"));
	InventoryWidgetInstance->SetWidgetSpace(EWidgetSpace::Screen);
	InventoryWidgetInstance->SetupAttachment(RootComponent);
	//InventoryWidgetInstance->SetWidget(CreateWidget<UUserWidget>(InventoryWidget));

	
	InventoryUICollision = CreateDefaultSubobject<USphereComponent>(TEXT("InventoryUICollision"));
	InventoryUICollision->SetupAttachment(RootComponent);
	InventoryUICollision->SetSphereRadius(300.f);


	currentItem = -1;
}

void AInteractionSimulatorCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();

			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		UserUIInstance = CreateWidget<UUserWidget>(GetWorld(), UserUI);
		UserUIInstance->AddToViewport();
	}

	Inventory = new std::vector<APickupableObject*>();

	bThirdPersonCurrentCamera = true;
	FollowCamera->SetActive(true);

	if(InventoryWidgetInstance)
	{
		UUserWidget* MainInventoryWidget = InventoryWidgetInstance->GetWidget();

		if(MainInventoryWidget)
		{
			InventoryListView = Cast<UListView>(MainInventoryWidget->GetWidgetFromName("ListView"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Nullptr MainInventoryWidget"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nullptr InventoryWidgetInstance"));
	}
	
}

void AInteractionSimulatorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CheckForInteraction();
}

FVector AInteractionSimulatorCharacter::GetCurrentCameraForwardVector() const
{
	if (bThirdPersonCurrentCamera)
	{
		return FollowCamera->GetForwardVector();
	}
	else 
	{
		return FirstPersonCamera->GetForwardVector();
	}
}

FVector AInteractionSimulatorCharacter::GetCurrentCameraLocation() const
{
	if (bThirdPersonCurrentCamera)
	{
		return FollowCamera->GetComponentLocation();
	}
	else
	{

		return FirstPersonCamera->GetComponentLocation();
	}
}

void AInteractionSimulatorCharacter::CheckForInteraction()
{
	
	if (this->GetLocalRole() != ROLE_AutonomousProxy)
	{
		return;
	}
	
	if (!this->FollowCamera || !GetWorld() || !UserUIInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%hs %hs %hs"), 
			(!this->FollowCamera ? "1" : "0"), 
			(!GetWorld() ? "1" : "0"),
			(!UserUIInstance ? "1" : "0"));
		return;
	}

	if(ItemLimit <= Inventory->size())
	{
		PickupableObject = nullptr;
		ShowInteractionMessage(false);

		return;
	}

	FVector Start = GetActorLocation();
	FVector End = Start + GetCurrentCameraForwardVector() * InteractionDistance;

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
	{
		PickupableObject = Cast<APickupableObject>(HitResult.GetActor());

		//UE_LOG(LogTemp, Warning, TEXT("%s"), (PickupableObject ? TEXT("true") : TEXT("false")));

		ShowInteractionMessage(PickupableObject != nullptr);
	}
	else
	{
		PickupableObject = nullptr;
		ShowInteractionMessage(false);
	}
}

void AInteractionSimulatorCharacter::ShowInteractionMessage(bool bShow) const
{
	if (UserUIInstance)
	{
		UserUIInstance->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("User UI is nullptr"));
	}
}

void AInteractionSimulatorCharacter::AddToInventory_Implementation(APickupableObject* PickUpAbleObject)
{
	if (InventoryListView)
	{
		UUserWidget* item = CreateWidget<UUserWidget>(GetWorld(), InventorySlotWidget);

		if(UImage* itemImage = Cast<UImage>(item->GetWidgetFromName("ItemImage")))
		{
			if(UTexture2D* const texture = PickUpAbleObject->ImageBrush)
			{
				
				itemImage->SetBrushFromTexture(texture, false);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Nullptr Material"));

				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Nullptr InventoryListView"));

			return;
		}

		//item->AddToViewport();

		InventoryListView->AddItem(item);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nullptr InventoryListView"));
	}
}

void AInteractionSimulatorCharacter::EraseFromInventory_Implementation(int itemIndex)
{
	if (InventoryListView)
	{
		UObject* item = InventoryListView->GetItemAt(itemIndex);
		if(!item)
		{
			UE_LOG(LogTemp, Warning, TEXT("Nullptr Item"));
			return;
		}

		InventoryListView->RemoveItem(item);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nullptr InventoryListView"));
	}
}

void AInteractionSimulatorCharacter::ClearInventory_Implementation()
{
	if (InventoryListView)
	{
		InventoryListView->ClearListItems();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nullptr InventoryListView"));
	}
}



//////////////////////////////////////////////////////////////////////////
// Input

void AInteractionSimulatorCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bad jump action"));
		}

		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AInteractionSimulatorCharacter::Move);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bad move action"));
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AInteractionSimulatorCharacter::Look);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bad look action"));
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AInteractionSimulatorCharacter::Interact);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bad interact action"));
		}

		if (ThrowAction)
		{
			EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Triggered, this, &AInteractionSimulatorCharacter::Throw);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bad throw action"));
		}

		if (SwitchCameraAction)
		{
			EnhancedInputComponent->BindAction(SwitchCameraAction, ETriggerEvent::Triggered, this, &AInteractionSimulatorCharacter::SwitchCamera);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bad switch camera action"));
		}
	}
}

void AInteractionSimulatorCharacter::Interact_Implementation(const FInputActionValue& Value)
{
	if(ItemLimit <= Inventory->size())
	{
		return;
	}
	PerformInteraction();
}

void AInteractionSimulatorCharacter::SwitchCamera_Implementation(const FInputActionValue& Value)
{

	FollowCamera->SetActive(!bThirdPersonCurrentCamera);
	FirstPersonCamera->SetActive(bThirdPersonCurrentCamera);

	bThirdPersonCurrentCamera = !bThirdPersonCurrentCamera;
}

void AInteractionSimulatorCharacter::PerformInteraction_Implementation()
{
	if (PickupableObject)
	{
		Inventory->push_back(PickupableObject);
		if (currentItem == -1)
		{
			currentItem = Inventory->size() - 1;
		}

		CallPickUp(PickupableObject);
	}
}

void AInteractionSimulatorCharacter::CallPickUp_Implementation(APickupableObject* PickUpAbleObject)
{
	if (PickUpAbleObject)
	{
		AddToInventory(PickUpAbleObject);

		PickUpAbleObject->OnPickup();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PickUpAble Object Nullptr"));
	}
}

void AInteractionSimulatorCharacter::Throw_Implementation(const FInputActionValue& Value)
{
	if(Inventory->empty())
	{
		return;
	}
	PerformThrow();
}

void AInteractionSimulatorCharacter::PerformThrow_Implementation()
{
	auto iterator = Inventory->begin() + (currentItem <= 0 ? 0 : currentItem);
	APickupableObject* item = *iterator;

	Inventory->erase(iterator);

	CallOnThrow(item);
}

void AInteractionSimulatorCharacter::CallOnThrow_Implementation(APickupableObject* PickUpAbleObject)
{
	if (PickUpAbleObject)
	{
		FVector const direction = GetCurrentCameraForwardVector();
		FVector const position = FirstPersonCamera->GetComponentLocation() + direction * distanceToThrowPoint;

		EraseFromInventory(currentItem <= 0 ? 0 : currentItem--);

		PickUpAbleObject->OnThrow(position, direction, ThrowForce);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PickUpAble Object Nullptr"));
	}
}



void AInteractionSimulatorCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AInteractionSimulatorCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		bUseControllerRotationYaw = !bThirdPersonCurrentCamera;
		
		//bUseControllerRotationPitch = !bThirdPersonCurrentCamera;

		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}




