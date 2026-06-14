#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::AddItem(const FInventoryItem& Item)
{
    if (HasItem(Item.ID))
    {
        return; // item déjà présent, on n'ajoute pas en double
    }
    Slots.Add(Item);
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::RemoveItem(int32 ItemID)
{
    int32 Removed = Slots.RemoveAll([ItemID](const FInventoryItem& Item)
        {
            return Item.ID == ItemID;
        });

    if (Removed > 0)
    {
        OnInventoryUpdated.Broadcast();
    }
}

TArray<FInventoryItem> UInventoryComponent::GetSlots() const
{
    return Slots;
}

bool UInventoryComponent::HasItem(int32 ItemID) const
{
    return Slots.ContainsByPredicate([ItemID](const FInventoryItem& Item)
        {
            return Item.ID == ItemID;
        });
}