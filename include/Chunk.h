#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <memory>
#include "Block.h"
#include "Vector3.h"

const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 128;
const int CHUNK_DEPTH = 16;

class Chunk {
private:
    std::vector<std::vector<std::vector<Block>>> blocks;
    Vector3 position;
    
public:
    Chunk(Vector3 pos);
    ~Chunk();
    
    Block& getBlock(int x, int y, int z);
    void setBlock(int x, int y, int z, Block block);
    
    Vector3 getPosition() const { return position; }
    void setPosition(Vector3 pos) { position = pos; }
    
    bool isBlockSolid(int x, int y, int z) const;
    bool isBlockEmpty(int x, int y, int z) const;
};

#endif // CHUNK_H
