#include "Inventory.h"
#include <iostream>
#include <algorithm>

Inventory::Inventory() : selectedSlot(0) {
    slots.resize(TOTAL_SLOTS);
    
    // Initialize with some starting items for testing
    addItem(BlockType::DIRT, 64);
    addItem(BlockType::STONE, 32);
    addItem(BlockType::WOOD, 16);
    addItem(BlockType::GRASS, 8);
}

Inventory::~Inventory() {
}

bool Inventory::addItem(BlockType itemType, int amount) {
    if (itemType == BlockType::AIR || amount <= 0) return false;
    
    int remainingAmount = amount;
    
    // First, try to stack with existing items
    for (int i = 0; i < TOTAL_SLOTS && remainingAmount > 0; i++) {
        if (slots[i].itemType == itemType && slots[i].canStack()) {
            int canAdd = std::min(remainingAmount, 64 - slots[i].count);
            slots[i].count += canAdd;
            remainingAmount -= canAdd;
        }
    }
    
    // If we still have items, try to put them in empty slots
    for (int i = 0; i < TOTAL_SLOTS && remainingAmount > 0; i++) {
        if (slots[i].isEmpty()) {
            int canAdd = std::min(remainingAmount, 64);
            slots[i].itemType = itemType;
            slots[i].count = canAdd;
            remainingAmount -= canAdd;
        }
    }
    
    return remainingAmount == 0; // Returns true if all items were added
}

bool Inventory::removeItem(BlockType itemType, int amount) {
    if (itemType == BlockType::AIR || amount <= 0) return false;
    
    int remainingAmount = amount;
    
    // Find and remove items
    for (int i = 0; i < TOTAL_SLOTS && remainingAmount > 0; i++) {
        if (slots[i].itemType == itemType) {
            int canRemove = std::min(remainingAmount, slots[i].count);
            slots[i].count -= canRemove;
            remainingAmount -= canRemove;
            
            if (slots[i].count <= 0) {
                slots[i].itemType = BlockType::AIR;
                slots[i].count = 0;
            }
        }
    }
    
    return remainingAmount == 0; // Returns true if all requested items were removed
}

bool Inventory::removeItemFromSlot(int slotIndex, int amount) {
    if (slotIndex < 0 || slotIndex >= TOTAL_SLOTS || amount <= 0) return false;
    
    int canRemove = std::min(amount, slots[slotIndex].count);
    slots[slotIndex].count -= canRemove;
    
    if (slots[slotIndex].count <= 0) {
        slots[slotIndex].itemType = BlockType::AIR;
        slots[slotIndex].count = 0;
    }
    
    return canRemove == amount;
}

int Inventory::getItemCount(BlockType itemType) const {
    int total = 0;
    for (const auto& slot : slots) {
        if (slot.itemType == itemType) {
            total += slot.count;
        }
    }
    return total;
}

InventorySlot& Inventory::getSlot(int index) {
    if (index < 0 || index >= TOTAL_SLOTS) {
        static InventorySlot emptySlot;
        return emptySlot;
    }
    return slots[index];
}

const InventorySlot& Inventory::getSlot(int index) const {
    if (index < 0 || index >= TOTAL_SLOTS) {
        static InventorySlot emptySlot;
        return emptySlot;
    }
    return slots[index];
}

InventorySlot& Inventory::getSelectedSlot() {
    return getSlot(selectedSlot);
}

const InventorySlot& Inventory::getSelectedSlot() const {
    return getSlot(selectedSlot);
}

void Inventory::selectSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < HOTBAR_SIZE) {
        selectedSlot = slotIndex;
        std::cout << "Selected hotbar slot " << slotIndex << ": " << getSelectedSlot().toString() << std::endl;
    }
}

BlockType Inventory::getSelectedItemType() const {
    return getSelectedSlot().itemType;
}

void Inventory::clearInventory() {
    for (auto& slot : slots) {
        slot.itemType = BlockType::AIR;
        slot.count = 0;
    }
}

bool Inventory::isSlotEmpty(int slotIndex) const {
    if (slotIndex < 0 || slotIndex >= TOTAL_SLOTS) return true;
    return slots[slotIndex].isEmpty();
}

int Inventory::findEmptySlot() const {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].isEmpty()) {
            return i;
        }
    }
    return -1; // No empty slot found
}

int Inventory::findSlotWithItem(BlockType itemType) const {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].itemType == itemType && slots[i].count > 0) {
            return i;
        }
    }
    return -1; // Item not found
}

void Inventory::printInventory() const {
    std::cout << "\n=== INVENTORY ===" << std::endl;
    std::cout << "Hotbar (0-8):" << std::endl;
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        std::cout << "[" << i << "] " << slots[i].toString();
        if (i == selectedSlot) std::cout << " <-- SELECTED";
        std::cout << std::endl;
    }
    
    std::cout << "\nMain Inventory:" << std::endl;
    for (int row = 0; row < INVENTORY_ROWS; row++) {
        for (int col = 0; col < INVENTORY_COLS; col++) {
            int slotIndex = HOTBAR_SIZE + (row * INVENTORY_COLS) + col;
            std::cout << "[" << slotIndex << "] " << slots[slotIndex].toString() << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << "=================" << std::endl;
}