#ifndef INVENTORY_H
#define INVENTORY_H

#include "Block.h"
#include <vector>
#include <string>

struct InventorySlot {
    BlockType itemType;
    int count;
    
    InventorySlot() : itemType(BlockType::AIR), count(0) {}
    InventorySlot(BlockType type, int amount) : itemType(type), count(amount) {}
    
    bool isEmpty() const { return itemType == BlockType::AIR || count <= 0; }
    bool canStack() const { return count < 64; } // Max stack size of 64
    
    std::string toString() const {
        if (isEmpty()) return "Empty";
        Block temp(itemType);
        return temp.toString() + " x" + std::to_string(count);
    }
};

class Inventory {
private:
    static const int HOTBAR_SIZE = 9;
    static const int INVENTORY_ROWS = 3;
    static const int INVENTORY_COLS = 9;
    static const int TOTAL_SLOTS = HOTBAR_SIZE + (INVENTORY_ROWS * INVENTORY_COLS);
    
    std::vector<InventorySlot> slots;
    int selectedSlot; // Currently selected hotbar slot (0-8)
    
public:
    Inventory();
    ~Inventory();
    
    // Basic inventory operations
    bool addItem(BlockType itemType, int amount = 1);
    bool removeItem(BlockType itemType, int amount = 1);
    bool removeItemFromSlot(int slotIndex, int amount = 1);
    int getItemCount(BlockType itemType) const;
    
    // Slot operations
    InventorySlot& getSlot(int index);
    const InventorySlot& getSlot(int index) const;
    InventorySlot& getSelectedSlot();
    const InventorySlot& getSelectedSlot() const;
    
    // Hotbar operations
    void selectSlot(int slotIndex);
    int getSelectedSlotIndex() const { return selectedSlot; }
    BlockType getSelectedItemType() const;
    
    // Inventory management
    void clearInventory();
    bool isSlotEmpty(int slotIndex) const;
    int findEmptySlot() const;
    int findSlotWithItem(BlockType itemType) const;
    
    // Constants
    static int getHotbarSize() { return HOTBAR_SIZE; }
    static int getTotalSlots() { return TOTAL_SLOTS; }
    static int getInventoryRows() { return INVENTORY_ROWS; }
    static int getInventoryCols() { return INVENTORY_COLS; }
    
    // Debug
    void printInventory() const;
};

#endif // INVENTORY_H