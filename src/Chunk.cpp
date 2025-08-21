#include "Chunk.h"
#include "Block.h"

Chunk::Chunk(Vector3 pos) : position(pos) {
    // Initialize chunk with empty blocks
    blocks.resize(CHUNK_WIDTH, std::vector<std::vector<Block>>(
        CHUNK_HEIGHT, std::vector<Block>(CHUNK_DEPTH, Block())));
}

Chunk::~Chunk() {
}

Block& Chunk::getBlock(int x, int y, int z) {
    // Bounds checking
    if (x >= 0 && x < CHUNK_WIDTH && 
        y >= 0 && y < CHUNK_HEIGHT && 
        z >= 0 && z < CHUNK_DEPTH) {
        return blocks[x][y][z];
    }
    
    // Return a default block if out of bounds
    static Block defaultBlock;
    return defaultBlock;
}

void Chunk::setBlock(int x, int y, int z, Block block) {
    // Bounds checking
    if (x >= 0 && x < CHUNK_WIDTH && 
        y >= 0 && y < CHUNK_HEIGHT && 
        z >= 0 && z < CHUNK_DEPTH) {
        blocks[x][y][z] = block;
    }
}

bool Chunk::isBlockSolid(int x, int y, int z) const {
    // Bounds checking
    if (x >= 0 && x < CHUNK_WIDTH && 
        y >= 0 && y < CHUNK_HEIGHT && 
        z >= 0 && z < CHUNK_DEPTH) {
        return blocks[x][y][z].isSolid();
    }
    
    return false; // Assume non-solid outside chunk bounds
}

bool Chunk::isBlockEmpty(int x, int y, int z) const {
    // Bounds checking
    if (x >= 0 && x < CHUNK_WIDTH && 
        y >= 0 && y < CHUNK_HEIGHT && 
        z >= 0 && z < CHUNK_DEPTH) {
        return blocks[x][y][z].isEmpty();
    }
    
    return true; // Assume empty outside chunk bounds
}
