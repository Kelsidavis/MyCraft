#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include <vector>
#include "Vector3.h"

enum BlockType {
    AIR,
    GRASS,
    DIRT,
    STONE,
    WOOD,
    LEAVES,
    WATER,
    SAND,
    COAL_ORE,
    IRON_ORE,
    DIAMOND_ORE
};

struct Block {
    BlockType type;
    Vector3 position;
    
    Block() : type(AIR), position(Vector3(0, 0, 0)) {}
    Block(BlockType t) : type(t), position(Vector3(0, 0, 0)) {}
    Block(BlockType t, Vector3 pos) : type(t), position(pos) {}
    
    bool isSolid() const {
        switch(type) {
            case AIR:
            case WATER:
            case LEAVES:
                return false;
            default:
                return true;
        }
    }
    
    bool isEmpty() const {
        return type == AIR;
    }
    
    std::string toString() const {
        switch(type) {
            case AIR: return "Air";
            case GRASS: return "Grass";
            case DIRT: return "Dirt";
            case STONE: return "Stone";
            case WOOD: return "Wood";
            case LEAVES: return "Leaves";
            case WATER: return "Water";
            case SAND: return "Sand";
            case COAL_ORE: return "Coal Ore";
            case IRON_ORE: return "Iron Ore";
            case DIAMOND_ORE: return "Diamond Ore";
            default: return "Unknown";
        }
    }
};

#endif // BLOCK_H
