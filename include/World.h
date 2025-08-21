#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <memory>
#include "Chunk.h"
#include "Vector3.h"

const int WORLD_WIDTH = 8;
const int WORLD_HEIGHT = 4;
const int WORLD_DEPTH = 8;

class World {
private:
    std::vector<std::vector<std::vector<std::shared_ptr<Chunk>>>> chunks;
    Vector3 playerPosition;
    
public:
    World();
    ~World();
    
    void generateWorld();
    void update();
    
    Chunk* getChunkAt(int x, int y, int z);
    Block* getBlockAt(int x, int y, int z);
    
    Vector3 getPlayerPosition() const { return playerPosition; }
    void setPlayerPosition(Vector3 pos) { playerPosition = pos; }
};

#endif // WORLD_H
