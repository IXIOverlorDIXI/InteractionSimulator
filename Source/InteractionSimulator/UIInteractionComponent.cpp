#include "UIInteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "PickupableObject.h"

// Конструктор
UIInteractionComponent::UIInteractionComponent()
{
    // Не нужно вызывать TickComponent каждый кадр
    PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UIInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void UIInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Проверяем возможные объекты для взаимодействия
    CheckForInteractables();
}

// Функция для проверки возможности взаимодействия с объектом
void UIInteractionComponent::CheckForInteractables()
{
    // Получаем контроллер игрока
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController) return;

    // Получаем положение и направление игрока
    FVector PlayerLocation;
    FRotator PlayerRotation;
    PlayerController->GetPlayerViewPoint(PlayerLocation, PlayerRotation);

    // Вычисляем конечную точку луча для определения объектов, с которыми можно взаимодействовать
    FVector TraceEnd = PlayerLocation + PlayerRotation.Vector() * InteractionDistance;

    // Отображаем отладочную линию для визуализации луча
    DrawDebugLine(GetWorld(), PlayerLocation, TraceEnd, FColor::Green, false, 0.1f, 0, 2.f);

    // Определяем объекты в области луча
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner()); // Игнорируем самого себя
    if (GetWorld()->LineTraceSingleByChannel(HitResult, PlayerLocation, TraceEnd, ECollisionChannel::ECC_Visibility, Params))
    {
        // Проверяем, является ли объект в области взаимодействия
        AActor* HitActor = HitResult.GetActor();
        if (HitActor && HitActor != CurrentInteractable && HitActor->GetClass()->ImplementsInterface(APickupableObject::StaticClass()))
        {
            // Если объект реализует интерфейс взаимодействия, сохраняем его
            CurrentInteractable = HitActor;
            ShowInteractionText();
        }
        else
        {
            // Если объект не реализует интерфейс взаимодействия, скрываем текст
            CurrentInteractable = nullptr;
            HideInteractionText();
        }
    }
    else
    {
        // Если ничего не попало в область взаимодействия, скрываем текст
        CurrentInteractable = nullptr;
        HideInteractionText();
    }
}

// Функция для отображения текста взаимодействия
void UIInteractionComponent::ShowInteractionText()
{
    // Отображение текста на экране игрока или другую вашу реализацию
    if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PlayerController->ClientMessage(InteractionText);
    }
}

// Функция для скрытия текста взаимодействия
void UIInteractionComponent::HideInteractionText()
{
    // Удаление текста на экране игрока или другая ваша реализация
    if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PlayerController->ClientMessage("");
    }
}

// Функция для выполнения действия взаимодействия
void UIInteractionComponent::PerformInteraction()
{
    // Вызывайте ваше действие взаимодействия здесь
}