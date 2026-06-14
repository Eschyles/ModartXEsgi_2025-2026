#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "InventoryComponent.generated.h"

UENUM(BlueprintType)
enum class EColorEnum : uint8
{
    Cherry       UMETA(DisplayName = "Cherry"),
    MidnightBlue UMETA(DisplayName = "Midnight Blue"),
    Olive        UMETA(DisplayName = "Olive"),
    Yellow       UMETA(DisplayName = "Yellow"),
    Amber        UMETA(DisplayName = "Amber"),
    SkyBlue      UMETA(DisplayName = "Sky Blue"),
    Bubblegum    UMETA(DisplayName = "Bubblegum"),
    BlueishWhite UMETA(DisplayName = "Blueish White"),
    White        UMETA(DisplayName = "White"),
    NavyBlue     UMETA(DisplayName = "Navy Blue")
};

USTRUCT(BlueprintType)
struct FInventoryItem : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 ID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EColorEnum Color = EColorEnum::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TObjectPtr<UTexture2D> Icon = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UPROJECT_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FInventoryItem> Slots;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(const FInventoryItem& Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItem(int32 ItemID);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventoryItem> GetSlots() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(int32 ItemID) const;
};