#include "World.h"
#include "Chunk.h"
#include "Block.h"
#include <cstdlib>
#include <cmath>

World::World() : playerPosition(0.0f, 0.0f, 0.0f) {
    // Initialize chunks vector
    chunks.resize(WORLD_WIDTH, std::vector<std::vector<std::shared_ptr<Chunk>>>(
        WORLD_HEIGHT, std::vector<std::shared_ptr<Chunk>>(WORLD_DEPTH)));
}

World::~World() {
    // Clean up chunks
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int z = 0; z < WORLD_DEPTH; z++) {
                chunks[x][y][z].reset();
            }
        }
    }
}

void World::generateWorld() {
    std::cout << "Generating rich Minecraft world with biomes..." << std::endl;
    
    int totalBlocks = 0;
    
    // Generate multiple chunks to create a proper world
    for (int cx = 0; cx < WORLD_WIDTH; cx++) {
        for (int cy = 0; cy < WORLD_HEIGHT; cy++) {
            for (int cz = 0; cz < WORLD_DEPTH; cz++) {
                chunks[cx][cy][cz] = std::make_shared<Chunk>(Vector3(cx, cy, cz));
                
                // Generate terrain for this chunk
                for (int x = 0; x < CHUNK_WIDTH; x++) {
                    for (int z = 0; z < CHUNK_DEPTH; z++) {
                        // Calculate world coordinates
                        int worldX = cx * CHUNK_WIDTH + x;
                        int worldZ = cz * CHUNK_DEPTH + z;
                        
                        // Complex height map with multiple octaves (now randomized)
                        float randomOffset1 = (rand() % 1000) / 10000.0f;
                        float randomOffset2 = (rand() % 1000) / 10000.0f;
                        float randomOffset3 = (rand() % 1000) / 10000.0f;
                        
                        float height = 12.0f + (rand() % 4) - 2; // Base height varies ±2
                        height += 8.0f * sin((worldX + randomOffset1) * 0.03f) * cos((worldZ + randomOffset1) * 0.03f);  // Large hills
                        height += 4.0f * sin((worldX + randomOffset2) * 0.1f) * sin((worldZ + randomOffset2) * 0.1f);   // Medium features
                        height += 2.0f * sin((worldX + randomOffset3) * 0.3f) * cos((worldZ + randomOffset3) * 0.25f); // Small details
                        int terrainHeight = (int)height;
                        
                        // Determine biome based on world coordinates
                        float biomeNoise = sin(worldX * 0.02f) + cos(worldZ * 0.02f);
                        bool isDesert = (biomeNoise > 0.5f);
                        bool isMountain = (height > 18.0f);
                        bool isWater = (terrainHeight < 8);
                        
                        // Generate terrain layers (limit height for performance)
                        for (int y = 0; y < CHUNK_HEIGHT && (cy * CHUNK_HEIGHT + y) <= terrainHeight + 10; y++) {
                            int worldY = cy * CHUNK_HEIGHT + y;
                            BlockType blockType = BlockType::AIR;
                            
                            if (worldY <= terrainHeight) {
                                // Surface blocks based on biome
                                if (worldY == terrainHeight) {
                                    if (isWater) {
                                        blockType = BlockType::SAND; // Beach sand
                                    } else if (isDesert) {
                                        blockType = BlockType::SAND; // Desert sand
                                    } else if (isMountain) {
                                        blockType = BlockType::STONE; // Mountain stone
                                    } else {
                                        blockType = BlockType::GRASS; // Normal grass
                                    }
                                }
                                // Subsurface layers
                                else if (worldY > terrainHeight - 4 && worldY > 4) {
                                    if (isDesert) {
                                        blockType = BlockType::SAND;
                                    } else {
                                        blockType = BlockType::DIRT;
                                    }
                                }
                                // Deep stone with ores
                                else {
                                    blockType = BlockType::STONE;
                                    
                                    // Add random ores
                                    int oreRandom = rand() % 100;
                                    if (worldY < 6 && oreRandom < 2) {
                                        blockType = BlockType::DIAMOND_ORE;
                                    } else if (worldY < 12 && oreRandom < 5) {
                                        blockType = BlockType::IRON_ORE;
                                    } else if (worldY < 20 && oreRandom < 8) {
                                        blockType = BlockType::COAL_ORE;
                                    }
                                }
                                
                                chunks[cx][cy][cz]->setBlock(x, y, z, Block(blockType));
                                totalBlocks++;
                            }
                            // Water level
                            else if (worldY <= 8) {
                                chunks[cx][cy][cz]->setBlock(x, y, z, Block(BlockType::WATER));
                                totalBlocks++;
                            }
                        }
                        
                        // Add vegetation
                        if (!isWater && terrainHeight > 8) {
                            // Trees
                            if ((worldX + worldZ) % 25 == 0 && !isDesert && !isMountain) {
                                // Tree trunk
                                for (int treeY = terrainHeight + 1; treeY < terrainHeight + 6; treeY++) {
                                    if (treeY < cy * CHUNK_HEIGHT + CHUNK_HEIGHT) {
                                        int localY = treeY - cy * CHUNK_HEIGHT;
                                        if (localY >= 0 && localY < CHUNK_HEIGHT) {
                                            chunks[cx][cy][cz]->setBlock(x, localY, z, Block(BlockType::WOOD));
                                            totalBlocks++;
                                        }
                                    }
                                }
                                // Tree leaves
                                for (int lx = -2; lx <= 2; lx++) {
                                    for (int lz = -2; lz <= 2; lz++) {
                                        for (int ly = terrainHeight + 4; ly < terrainHeight + 8; ly++) {
                                            if (abs(lx) + abs(lz) <= 2 && ly < cy * CHUNK_HEIGHT + CHUNK_HEIGHT) {
                                                int leafX = x + lx;
                                                int leafZ = z + lz;
                                                int localY = ly - cy * CHUNK_HEIGHT;
                                                
                                                if (leafX >= 0 && leafX < CHUNK_WIDTH && 
                                                    leafZ >= 0 && leafZ < CHUNK_DEPTH &&
                                                    localY >= 0 && localY < CHUNK_HEIGHT) {
                                                    chunks[cx][cy][cz]->setBlock(leafX, localY, leafZ, Block(BlockType::LEAVES));
                                                    totalBlocks++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    std::cout << "Generated " << totalBlocks << " blocks with biomes in a " 
              << WORLD_WIDTH << "x" << WORLD_HEIGHT << "x" << WORLD_DEPTH << " world" << std::endl;
}

void World::update() {
    // Update world state
}

Chunk* World::getChunkAt(int x, int y, int z) {
    // Bounds checking - x,y,z are chunk indices, not world coordinates
    if (x >= 0 && x < WORLD_WIDTH &&
        y >= 0 && y < WORLD_HEIGHT &&
        z >= 0 && z < WORLD_DEPTH) {
        return chunks[x][y][z].get();
    }
    
    return nullptr; // Chunk not found
}

Block* World::getBlockAt(int x, int y, int z) {
    // Convert world coordinates to chunk indices
    int chunkX = x / CHUNK_WIDTH;
    int chunkY = y / CHUNK_HEIGHT;
    int chunkZ = z / CHUNK_DEPTH;
    
    // Get chunk that contains this block
    Chunk* chunk = getChunkAt(chunkX, chunkY, chunkZ);
    if (chunk) {
        // Convert world coordinates to chunk-relative coordinates
        int localX = x % CHUNK_WIDTH;
        int localY = y % CHUNK_HEIGHT;
        int localZ = z % CHUNK_DEPTH;
        
        // Handle negative coordinates properly
        if (x < 0) localX = CHUNK_WIDTH + localX;
        if (y < 0) localY = CHUNK_HEIGHT + localY;
        if (z < 0) localZ = CHUNK_DEPTH + localZ;
        
        // Return reference to block
        return &chunk->getBlock(localX, localY, localZ);
    }
    
    return nullptr; // Block not found
}
