#include "Renderer.h"
#include <iostream>

Renderer::Renderer(World* w) : world(w), mode(RenderMode::SOLID),
    cameraPosition(64.0f, 50.0f, 64.0f), velocity(0.0f, 0.0f, 0.0f), 
    cameraYaw(-45.0f), cameraPitch(-20.0f), isOnGround(false), isJumping(false),
    isInWater(false), isSwimming(false), fieldOfView(45.0f),
    showMenu(false), selectedMenuItem(0), showPlayerModel(true), showDebugInfo(false), flightMode(false),
    mouseSensitivity(0.1f), movementSpeed(8.0f), showInventory(false),
    isSwinging(false), swingProgress(0.0f), swingTimer(0.0f), currentElbowAngle(0.0f),
    textureAtlas(0), texturesLoaded(false) {
}

Renderer::~Renderer() {
}

void Renderer::init() {
    // Initialize OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    
    // Enable textures
    glEnable(GL_TEXTURE_2D);
    
    // Load textures
    loadTextures();
    
    // Disable lighting for now to see pure colors
    glDisable(GL_LIGHTING);
    
    // Setup initial camera
    setupCamera();
}

void Renderer::initPlayerPosition() {
    // Find a good starting position on solid ground
    int startX = 64;  // Center of larger world
    int startZ = 64;  // Center of larger world
    
    // Find ground level more thoroughly
    int groundY = 0;
    for (int y = WORLD_HEIGHT * CHUNK_HEIGHT - 1; y >= 0; y--) {
        if (isBlockAt(startX, y, startZ)) {
            groundY = y;
            break;
        }
    }
    
    cameraPosition.x = startX + 0.5f; // Center of block
    cameraPosition.y = groundY + 2.5f; // At least 2 blocks above ground (2.0 + 0.5 for player height)
    cameraPosition.z = startZ + 0.5f; // Center of block
    
    // Ensure clear space for player (need 2 block height clearance)
    bool foundClearSpace = false;
    int maxHeight = groundY + 20; // Don't go too high
    
    for (int testY = groundY + 2; testY < maxHeight; testY++) {
        // Check if we have 2 blocks of clear space
        bool clear1 = !isBlockAt(startX, testY, startZ);
        bool clear2 = !isBlockAt(startX, testY + 1, startZ);
        
        if (clear1 && clear2) {
            cameraPosition.y = testY + 0.5f; // Player eye level
            foundClearSpace = true;
            std::cout << "Found clear spawn space at Y=" << testY << std::endl;
            break;
        }
    }
    
    // If no clear space found, force spawn high above ground
    if (!foundClearSpace) {
        cameraPosition.y = groundY + 10.0f;
        std::cout << "No clear space found, spawning high at Y=" << (groundY + 10.0f) << std::endl;
    }
    
    velocity = Vector3(0, 0, 0);
    isOnGround = false; // Let physics determine ground state
    isJumping = false;
    
    std::cout << "Player spawned at (" << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << ")" << std::endl;
    std::cout << "Ground level found at Y=" << groundY << std::endl;
    std::cout << "Player height above ground: " << (cameraPosition.y - groundY) << " blocks" << std::endl;
}

void Renderer::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    setupCamera();
    renderWorld();
    
    // Only render player model if enabled and menu is not open
    if (showPlayerModel && !showMenu) {
        renderPlayerModel();
    }
    
    // Render inventory if open
    if (showInventory) {
        renderInventory();
    }
    
    // Always render hotbar (unless menu is open)
    if (!showMenu && !showInventory) {
        renderHotbar();
    }
    
    // Render menu on top if open
    if (showMenu) {
        renderMenu();
    }
    
    glutSwapBuffers();
}

void Renderer::update(float deltaTime) {
    const float airGravity = -25.0f;
    const float waterGravity = -5.0f;  // Much weaker gravity in water
    const float terminalVelocity = -50.0f;
    const float waterTerminalVelocity = -8.0f;  // Slower fall in water
    const float playerHeight = 1.8f;
    const float waterResistance = 0.7f;  // More friction in water
    const float airFriction = 0.85f;
    const float flightFriction = 0.9f;   // Less friction in flight mode
    
    // In flight mode, skip physics and collision detection
    if (flightMode) {
        // Simple movement with friction in flight mode
        velocity.x *= flightFriction;
        velocity.y *= flightFriction;
        velocity.z *= flightFriction;
        
        // Apply velocity to position
        cameraPosition.x += velocity.x * deltaTime;
        cameraPosition.y += velocity.y * deltaTime;
        cameraPosition.z += velocity.z * deltaTime;
        
        // Keep player within world bounds
        if (cameraPosition.x < 0) cameraPosition.x = 0;
        if (cameraPosition.z < 0) cameraPosition.z = 0;
        if (cameraPosition.x >= WORLD_WIDTH * CHUNK_WIDTH) cameraPosition.x = WORLD_WIDTH * CHUNK_WIDTH - 1;
        if (cameraPosition.z >= WORLD_DEPTH * CHUNK_DEPTH) cameraPosition.z = WORLD_DEPTH * CHUNK_DEPTH - 1;
        if (cameraPosition.y < 0) cameraPosition.y = 0;
        if (cameraPosition.y >= WORLD_HEIGHT * CHUNK_HEIGHT) cameraPosition.y = WORLD_HEIGHT * CHUNK_HEIGHT - 1;
        
        // Update arm swing animation even in flight mode
        if (isSwinging) {
            swingTimer += deltaTime;
            swingProgress = swingTimer / 0.15f; // 0.15 second swing duration (faster)
            if (swingProgress >= 1.0f) {
                isSwinging = false;
                swingProgress = 0.0f;
                swingTimer = 0.0f;
            }
        }
        
        return; // Skip normal physics
    }
    
    // Update arm swing animation
    if (isSwinging) {
        swingTimer += deltaTime;
        swingProgress = swingTimer / 0.15f; // 0.15 second swing duration (faster)
        if (swingProgress >= 1.0f) {
            isSwinging = false;
            swingProgress = 0.0f;
            swingTimer = 0.0f;
        }
    }
    
    // Check if player is in water (normal physics mode)
    int posX = (int)floor(cameraPosition.x);
    int posZ = (int)floor(cameraPosition.z);
    int headY = (int)floor(cameraPosition.y);
    int bodyY = (int)floor(cameraPosition.y - 0.5f);
    int feetY = (int)floor(cameraPosition.y - playerHeight);
    
    // Player is in water if head or body is in water
    isInWater = isWaterAt(posX, headY, posZ) || isWaterAt(posX, bodyY, posZ);
    
    // Apply appropriate gravity
    if (isInWater) {
        velocity.y += waterGravity * deltaTime;
        if (velocity.y < waterTerminalVelocity) {
            velocity.y = waterTerminalVelocity;
        }
        isSwimming = true;
        isOnGround = false;  // Can't be "on ground" while swimming
    } else {
        isSwimming = false;
        if (!isOnGround) {
            velocity.y += airGravity * deltaTime;
            if (velocity.y < terminalVelocity) {
                velocity.y = terminalVelocity;
            }
        }
    }
    
    // Apply X movement with collision (only check solid blocks, not water)
    Vector3 newPos = cameraPosition;
    newPos.x += velocity.x * deltaTime;
    if (!isBlockAt((int)floor(newPos.x), (int)floor(newPos.y - playerHeight), (int)floor(newPos.z))) {
        cameraPosition.x = newPos.x;
    } else {
        velocity.x = 0; // Stop horizontal movement if hitting solid wall
    }
    
    // Apply Z movement with collision (only check solid blocks, not water)
    newPos = cameraPosition;
    newPos.z += velocity.z * deltaTime;
    if (!isBlockAt((int)floor(newPos.x), (int)floor(newPos.y - playerHeight), (int)floor(newPos.z))) {
        cameraPosition.z = newPos.z;
    } else {
        velocity.z = 0; // Stop horizontal movement if hitting solid wall
    }
    
    // Apply Y movement with collision
    newPos = cameraPosition;
    newPos.y += velocity.y * deltaTime;
    
    // Update positions for collision check
    posX = (int)floor(newPos.x);
    posZ = (int)floor(newPos.z);
    feetY = (int)floor(newPos.y - playerHeight);
    headY = (int)floor(newPos.y);
    
    if (!isInWater) {
        // Normal air physics
        if (velocity.y < 0) { // Falling down
            if (isBlockAt(posX, feetY, posZ)) {
                // Hit ground - place player on top of block
                cameraPosition.y = feetY + 1.0f + playerHeight;
                velocity.y = 0;
                isOnGround = true;
                isJumping = false;
            } else {
                cameraPosition.y = newPos.y;
                isOnGround = false;
            }
        } else if (velocity.y > 0) { // Moving up
            if (isBlockAt(posX, headY, posZ)) {
                // Hit ceiling
                velocity.y = 0;
                cameraPosition.y = headY - 0.1f; // Just below the block
            } else {
                cameraPosition.y = newPos.y;
                isOnGround = false;
            }
        } else {
            // Check if still on ground
            if (isBlockAt(posX, feetY, posZ)) {
                isOnGround = true;
            } else {
                isOnGround = false;
            }
        }
    } else {
        // Swimming physics - can move freely in water
        // Only check for solid blocks, not water
        if (velocity.y < 0 && isBlockAt(posX, feetY, posZ)) {
            // Hit solid ground while swimming
            cameraPosition.y = feetY + 1.0f + playerHeight;
            velocity.y = 0;
        } else if (velocity.y > 0 && isBlockAt(posX, headY, posZ)) {
            // Hit solid ceiling while swimming
            velocity.y = 0;
            cameraPosition.y = headY - 0.1f;
        } else {
            cameraPosition.y = newPos.y;
        }
    }
    
    // Apply appropriate friction
    if (isInWater) {
        velocity.x *= waterResistance;
        velocity.z *= waterResistance;
        velocity.y *= 0.95f;  // Also apply resistance to vertical movement in water
    } else {
        velocity.x *= airFriction;
        velocity.z *= airFriction;
    }
    
    // Debug output
    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) { // Every second at 60fps
        std::cout << "Player pos: (" << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << ")" 
                  << " Flying: " << (flightMode ? "true" : "false")
                  << " OnGround: " << (isOnGround ? "true" : "false")
                  << " InWater: " << (isInWater ? "true" : "false")
                  << " Swimming: " << (isSwimming ? "true" : "false")
                  << " Vel: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
    }
}

void Renderer::renderWorld() {
    if (!world) return;
    
    static bool firstRender = true;
    int chunksRendered = 0;
    const float RENDER_DISTANCE = 80.0f; // Only render chunks within this distance
    
    // Get camera chunk position
    int camChunkX = (int)(cameraPosition.x / CHUNK_WIDTH);
    int camChunkZ = (int)(cameraPosition.z / CHUNK_DEPTH);
    
    // Render chunks within render distance
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int z = 0; z < WORLD_DEPTH; z++) {
                // Distance-based culling
                float chunkCenterX = x * CHUNK_WIDTH + CHUNK_WIDTH / 2;
                float chunkCenterZ = z * CHUNK_DEPTH + CHUNK_DEPTH / 2;
                
                float distanceToChunk = sqrt((cameraPosition.x - chunkCenterX) * (cameraPosition.x - chunkCenterX) +
                                           (cameraPosition.z - chunkCenterZ) * (cameraPosition.z - chunkCenterZ));
                
                if (distanceToChunk <= RENDER_DISTANCE) {
                    Chunk* chunk = world->getChunkAt(x, y, z);
                    if (chunk) {
                        renderChunk(chunk);
                        chunksRendered++;
                    }
                }
            }
        }
    }
    
    if (firstRender) {
        std::cout << "First render: " << chunksRendered << " chunks rendered" << std::endl;
        firstRender = false;
    }
}

void Renderer::renderChunk(Chunk* chunk) {
    if (!chunk) return;
    
    // Get chunk world position
    Vector3 chunkPos = chunk->getPosition();
    
    // Convert chunk coordinates to world coordinates
    int worldX = chunkPos.x * CHUNK_WIDTH;
    int worldY = chunkPos.y * CHUNK_HEIGHT;
    int worldZ = chunkPos.z * CHUNK_DEPTH;
    
    static bool firstChunkRender = true;
    int blocksRendered = 0;
    
    // Render blocks in chunk
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                Block& block = chunk->getBlock(x, y, z);
                if (!block.isEmpty()) {
                    renderBlock(block, worldX + x, worldY + y, worldZ + z);
                    blocksRendered++;
                }
            }
        }
    }
    
    if (firstChunkRender) {
        std::cout << "First chunk render: " << blocksRendered << " blocks rendered at chunk pos (" 
                  << chunkPos.x << "," << chunkPos.y << "," << chunkPos.z << ")" << std::endl;
        std::cout << "World offset: (" << worldX << "," << worldY << "," << worldZ << ")" << std::endl;
        firstChunkRender = false;
    }
}

void Renderer::renderBlock(Block& block, int x, int y, int z) {
    // Simple cube rendering
    glPushMatrix();
    glTranslatef(x, y, z);
    
    // Handle wireframe mode
    if (mode == RenderMode::WIREFRAME) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1.0f, 1.0f, 1.0f); // White wireframe
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    bool useTextures = (mode == RenderMode::TEXTURED && texturesLoaded);
    
    if (useTextures) {
        glBindTexture(GL_TEXTURE_2D, textureAtlas);
        glColor3f(1.0f, 1.0f, 1.0f); // White to show texture colors
        
        float texCoords[8];
        getBlockTexCoords(block.type, texCoords);
        
        // Render cube with textures (face culling)
        glBegin(GL_QUADS);
        
        // Front face (z = 0)
        if (shouldRenderFace(x, y, z, 0)) {
            glNormal3f(0.0f, 0.0f, -1.0f);
            glTexCoord2f(texCoords[0], texCoords[1]); glVertex3f(0, 0, 0);
            glTexCoord2f(texCoords[2], texCoords[3]); glVertex3f(1, 0, 0);
            glTexCoord2f(texCoords[4], texCoords[5]); glVertex3f(1, 1, 0);
            glTexCoord2f(texCoords[6], texCoords[7]); glVertex3f(0, 1, 0);
        }
        
        // Back face (z = 1)
        if (shouldRenderFace(x, y, z, 1)) {
            glNormal3f(0.0f, 0.0f, 1.0f);
            glTexCoord2f(texCoords[2], texCoords[3]); glVertex3f(1, 0, 1);
            glTexCoord2f(texCoords[0], texCoords[1]); glVertex3f(0, 0, 1);
            glTexCoord2f(texCoords[6], texCoords[7]); glVertex3f(0, 1, 1);
            glTexCoord2f(texCoords[4], texCoords[5]); glVertex3f(1, 1, 1);
        }
        
        // Top face (y = 1)
        if (shouldRenderFace(x, y, z, 2)) {
            glNormal3f(0.0f, 1.0f, 0.0f);
            glTexCoord2f(texCoords[0], texCoords[1]); glVertex3f(0, 1, 0);
            glTexCoord2f(texCoords[2], texCoords[3]); glVertex3f(1, 1, 0);
            glTexCoord2f(texCoords[4], texCoords[5]); glVertex3f(1, 1, 1);
            glTexCoord2f(texCoords[6], texCoords[7]); glVertex3f(0, 1, 1);
        }
        
        // Bottom face (y = 0)
        if (shouldRenderFace(x, y, z, 3)) {
            glNormal3f(0.0f, -1.0f, 0.0f);
            glTexCoord2f(texCoords[6], texCoords[7]); glVertex3f(0, 0, 1);
            glTexCoord2f(texCoords[4], texCoords[5]); glVertex3f(1, 0, 1);
            glTexCoord2f(texCoords[2], texCoords[3]); glVertex3f(1, 0, 0);
            glTexCoord2f(texCoords[0], texCoords[1]); glVertex3f(0, 0, 0);
        }
        
        // Right face (x = 1)
        if (shouldRenderFace(x, y, z, 4)) {
            glNormal3f(1.0f, 0.0f, 0.0f);
            glTexCoord2f(texCoords[0], texCoords[1]); glVertex3f(1, 0, 0);
            glTexCoord2f(texCoords[2], texCoords[3]); glVertex3f(1, 0, 1);
            glTexCoord2f(texCoords[4], texCoords[5]); glVertex3f(1, 1, 1);
            glTexCoord2f(texCoords[6], texCoords[7]); glVertex3f(1, 1, 0);
        }
        
        // Left face (x = 0)
        if (shouldRenderFace(x, y, z, 5)) {
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glTexCoord2f(texCoords[2], texCoords[3]); glVertex3f(0, 0, 1);
            glTexCoord2f(texCoords[0], texCoords[1]); glVertex3f(0, 0, 0);
            glTexCoord2f(texCoords[6], texCoords[7]); glVertex3f(0, 1, 0);
            glTexCoord2f(texCoords[4], texCoords[5]); glVertex3f(0, 1, 1);
        }
        
    } else {
        // Set color based on block type (fallback for solid mode)
        switch (block.type) {
            case BlockType::GRASS:
                glColor3f(0.2f, 0.8f, 0.2f);  // Bright Green
                break;
            case BlockType::DIRT:
                glColor3f(0.6f, 0.4f, 0.2f);  // Brown
                break;
            case BlockType::STONE:
                glColor3f(0.6f, 0.6f, 0.6f);  // Gray
                break;
            case BlockType::WOOD:
                glColor3f(0.6f, 0.3f, 0.1f);  // Dark Brown
                break;
            case BlockType::LEAVES:
                glColor3f(0.1f, 0.6f, 0.1f);  // Dark Green
                break;
            case BlockType::WATER:
                glColor3f(0.2f, 0.4f, 0.8f);  // Blue
                break;
            case BlockType::SAND:
                glColor3f(0.9f, 0.8f, 0.6f);  // Sandy Yellow
                break;
            case BlockType::COAL_ORE:
                glColor3f(0.3f, 0.3f, 0.3f);  // Dark Gray with black spots
                break;
            case BlockType::IRON_ORE:
                glColor3f(0.8f, 0.7f, 0.6f);  // Beige with brown spots
                break;
            case BlockType::DIAMOND_ORE:
                glColor3f(0.7f, 0.9f, 0.9f);  // Light Blue
                break;
            default:
                glColor3f(0.8f, 0.8f, 0.8f);  // Light Gray
        }
        
        // Render cube with normals and face culling
        glBegin(GL_QUADS);
        
        // Front face (z = 0)
        if (shouldRenderFace(x, y, z, 0)) {
            glNormal3f(0.0f, 0.0f, -1.0f);
            glVertex3f(0, 0, 0);
            glVertex3f(1, 0, 0);
            glVertex3f(1, 1, 0);
            glVertex3f(0, 1, 0);
        }
        
        // Back face (z = 1)
        if (shouldRenderFace(x, y, z, 1)) {
            glNormal3f(0.0f, 0.0f, 1.0f);
            glVertex3f(1, 0, 1);
            glVertex3f(0, 0, 1);
            glVertex3f(0, 1, 1);
            glVertex3f(1, 1, 1);
        }
        
        // Top face (y = 1)
        if (shouldRenderFace(x, y, z, 2)) {
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0, 1, 0);
            glVertex3f(1, 1, 0);
            glVertex3f(1, 1, 1);
            glVertex3f(0, 1, 1);
        }
        
        // Bottom face (y = 0)
        if (shouldRenderFace(x, y, z, 3)) {
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(0, 0, 1);
            glVertex3f(1, 0, 1);
            glVertex3f(1, 0, 0);
            glVertex3f(0, 0, 0);
        }
        
        // Right face (x = 1)
        if (shouldRenderFace(x, y, z, 4)) {
            glNormal3f(1.0f, 0.0f, 0.0f);
            glVertex3f(1, 0, 0);
            glVertex3f(1, 0, 1);
            glVertex3f(1, 1, 1);
            glVertex3f(1, 1, 0);
        }
        
        // Left face (x = 0)
        if (shouldRenderFace(x, y, z, 5)) {
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f(0, 0, 1);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 1, 0);
            glVertex3f(0, 1, 1);
        }
    }
    
    glEnd();
    
    glPopMatrix();
}

void Renderer::setupCamera() {
    // Setup projection matrix with zoom
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Get current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    double aspectRatio = (double)viewport[2] / viewport[3];
    
    gluPerspective(fieldOfView, aspectRatio, 0.1, 200.0);
    
    // Setup modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Apply camera transformations
    glRotatef(-cameraPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-cameraYaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
}

void Renderer::setupLighting() {
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Set light position (sun-like lighting)
    GLfloat lightPosition[] = { 1.0f, 1.0f, 1.0f, 0.0f }; // Directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    
    // Set light properties with more ambient light
    GLfloat lightAmbient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

void Renderer::moveCamera(float dx, float dy, float dz) {
    cameraPosition.x += dx;
    cameraPosition.y += dy;
    cameraPosition.z += dz;
}

void Renderer::moveCameraPhysics(float forward, float right) {
    const float speed = movementSpeed;
    
    // Calculate movement based on camera yaw
    float radians = cameraYaw * 3.14159f / 180.0f;
    float forwardX = -sin(radians);
    float forwardZ = -cos(radians);
    float rightX = cos(radians);
    float rightZ = -sin(radians);
    
    // Apply movement to velocity
    velocity.x += (forwardX * forward + rightX * right) * speed;
    velocity.z += (forwardZ * forward + rightZ * right) * speed;
}

void Renderer::jump() {
    if (flightMode) {
        // Flight mode - fly up
        velocity.y += 10.0f;
        if (velocity.y > 15.0f) velocity.y = 15.0f; // Cap flight speed
    } else if (isInWater) {
        // Swimming upward - can always swim up in water
        velocity.y += 5.0f; // Swimming velocity boost
        if (velocity.y > 8.0f) velocity.y = 8.0f; // Cap swimming speed
    } else if (isOnGround && !isJumping) {
        // Normal jump on land
        velocity.y = 8.0f; // Jump velocity
        isOnGround = false;
        isJumping = true;
    }
}

void Renderer::swimDown() {
    if (flightMode) {
        // Flight mode - fly down
        velocity.y -= 10.0f;
        if (velocity.y < -15.0f) velocity.y = -15.0f; // Cap flight speed
    } else if (isInWater) {
        // Swimming downward in water
        velocity.y -= 5.0f; // Swimming velocity downward
        if (velocity.y < -8.0f) velocity.y = -8.0f; // Cap downward swimming speed
    } else {
        // Creative mode fly down (legacy)
        cameraPosition.y -= 0.5f;
    }
}

void Renderer::flyUp() {
    if (flightMode) {
        velocity.y += 10.0f;
        if (velocity.y > 15.0f) velocity.y = 15.0f;
    }
}

void Renderer::flyDown() {
    if (flightMode) {
        velocity.y -= 10.0f;
        if (velocity.y < -15.0f) velocity.y = -15.0f;
    }
}

void Renderer::toggleFlightMode() {
    flightMode = !flightMode;
    if (flightMode) {
        // Reset velocity when entering flight mode
        velocity = Vector3(0, 0, 0);
        isOnGround = false;
        isJumping = false;
        std::cout << "Flight mode: ON (Free floating enabled)" << std::endl;
    } else {
        // Reset velocity when exiting flight mode
        velocity = Vector3(0, 0, 0);
        std::cout << "Flight mode: OFF (Physics enabled)" << std::endl;
    }
}

bool Renderer::isBlockAt(int x, int y, int z) {
    if (!world) return false;
    
    // Bounds check - don't go outside world
    if (x < 0 || z < 0 || y < 0) return false;
    if (x >= WORLD_WIDTH * CHUNK_WIDTH || z >= WORLD_DEPTH * CHUNK_DEPTH || y >= WORLD_HEIGHT * CHUNK_HEIGHT) return false;
    
    Block* block = world->getBlockAt(x, y, z);
    if (!block) return false;
    
    return (!block->isEmpty() && block->isSolid());
}

bool Renderer::isWaterAt(int x, int y, int z) {
    if (!world) return false;
    
    // Bounds check - don't go outside world
    if (x < 0 || z < 0 || y < 0) return false;
    if (x >= WORLD_WIDTH * CHUNK_WIDTH || z >= WORLD_DEPTH * CHUNK_DEPTH || y >= WORLD_HEIGHT * CHUNK_HEIGHT) return false;
    
    Block* block = world->getBlockAt(x, y, z);
    if (!block) return false;
    
    return (block->type == BlockType::WATER);
}

int Renderer::findGroundLevel(int x, int z) {
    if (!world) return 10;
    
    // Search from top to bottom for the first solid block
    for (int y = WORLD_HEIGHT * CHUNK_HEIGHT - 1; y >= 0; y--) {
        if (isBlockAt(x, y, z)) {
            return y + 1; // Return the position above the solid block
        }
    }
    return 10; // Default height if no ground found
}

bool Renderer::shouldRenderFace(int x, int y, int z, int faceDirection) {
    // Face directions: 0=front, 1=back, 2=top, 3=bottom, 4=right, 5=left
    int checkX = x, checkY = y, checkZ = z;
    
    switch (faceDirection) {
        case 0: checkZ -= 1; break; // Front face - check block in front
        case 1: checkZ += 1; break; // Back face - check block behind
        case 2: checkY += 1; break; // Top face - check block above
        case 3: checkY -= 1; break; // Bottom face - check block below
        case 4: checkX += 1; break; // Right face - check block to right
        case 5: checkX -= 1; break; // Left face - check block to left
    }
    
    // Don't render face if there's a solid block adjacent to it
    return !isBlockAt(checkX, checkY, checkZ);
}

void Renderer::rotateCamera(float yaw, float pitch) {
    cameraYaw += yaw;
    cameraPitch += pitch;
    
    // Clamp pitch to prevent flipping
    if (cameraPitch > 89.0f) cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;
}

void Renderer::zoom(float factor) {
    fieldOfView *= factor;
    
    // Clamp field of view to reasonable limits
    if (fieldOfView < 10.0f) fieldOfView = 10.0f;   // Maximum zoom in
    if (fieldOfView > 120.0f) fieldOfView = 120.0f; // Maximum zoom out
    
    static int debugCounter = 0;
    if (debugCounter++ % 5 == 0) { // Every 5th zoom operation
        std::cout << "FOV: " << fieldOfView << "°" << std::endl;
    }
}

bool Renderer::loadTextures() {
    const int ATLAS_SIZE = 512;
    const int TEXTURE_SIZE = 64; // Each block texture is 64x64
    const int TEXTURES_PER_ROW = ATLAS_SIZE / TEXTURE_SIZE; // 8 textures per row
    
    ImageData atlas(ATLAS_SIZE, ATLAS_SIZE);
    
    // Block texture filenames
    std::vector<std::string> textureFiles = {
        "assets/textures/air.ppm",         // 0 - AIR
        "assets/textures/grass.ppm",       // 1 - GRASS
        "assets/textures/dirt.ppm",        // 2 - DIRT
        "assets/textures/stone.ppm",       // 3 - STONE
        "assets/textures/wood.ppm",        // 4 - WOOD
        "assets/textures/leaves.ppm",      // 5 - LEAVES
        "assets/textures/water.ppm",       // 6 - WATER
        "assets/textures/sand.ppm",        // 7 - SAND
        "assets/textures/coal_ore.ppm",    // 8 - COAL_ORE
        "assets/textures/iron_ore.ppm",    // 9 - IRON_ORE
        "assets/textures/diamond_ore.ppm"  // 10 - DIAMOND_ORE
    };
    
    // Load individual block textures and place them in atlas
    for (int blockType = 0; blockType < textureFiles.size(); blockType++) {
        ImageData blockTexture;
        
        // Try to load sprite file first
        if (ImageLoader::loadPPM(textureFiles[blockType], blockTexture)) {
            std::cout << "Loaded sprite: " << textureFiles[blockType] << std::endl;
        } else {
            // Fallback to procedural texture if file doesn't exist
            std::cout << "Creating procedural texture for block type " << blockType << std::endl;
            blockTexture = ImageLoader::createPatternTexture(TEXTURE_SIZE, TEXTURE_SIZE, blockType);
        }
        
        // Resize texture to standard size if needed
        if (blockTexture.width != TEXTURE_SIZE || blockTexture.height != TEXTURE_SIZE) {
            std::cout << "Warning: texture " << textureFiles[blockType] << " is " 
                      << blockTexture.width << "x" << blockTexture.height 
                      << " but expected " << TEXTURE_SIZE << "x" << TEXTURE_SIZE << std::endl;
            // For now, just use the procedural texture
            blockTexture = ImageLoader::createPatternTexture(TEXTURE_SIZE, TEXTURE_SIZE, blockType);
        }
        
        // Place texture in atlas
        int row = blockType / TEXTURES_PER_ROW;
        int col = blockType % TEXTURES_PER_ROW;
        
        for (int y = 0; y < TEXTURE_SIZE; y++) {
            for (int x = 0; x < TEXTURE_SIZE; x++) {
                int atlasX = col * TEXTURE_SIZE + x;
                int atlasY = row * TEXTURE_SIZE + y;
                int atlasIndex = (atlasY * ATLAS_SIZE + atlasX) * 3;
                int textureIndex = (y * TEXTURE_SIZE + x) * 3;
                
                if (textureIndex < blockTexture.data.size()) {
                    atlas.data[atlasIndex] = blockTexture.data[textureIndex];
                    atlas.data[atlasIndex + 1] = blockTexture.data[textureIndex + 1];
                    atlas.data[atlasIndex + 2] = blockTexture.data[textureIndex + 2];
                }
            }
        }
    }
    
    // Create OpenGL texture
    glGenTextures(1, &textureAtlas);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, atlas.data.data());
    
    // Save the atlas for debugging
    ImageLoader::savePPM("texture_atlas_debug.ppm", atlas);
    
    texturesLoaded = true;
    std::cout << "Texture atlas loaded successfully (sprite + procedural fallback)!" << std::endl;
    
    return true;
}

void Renderer::getBlockTexCoords(BlockType blockType, float* texCoords) {
    const float TEXTURE_SIZE = 64.0f;
    const float ATLAS_SIZE = 512.0f;
    const int TEXTURES_PER_ROW = 8;
    
    int blockIndex = (int)blockType;
    int row = blockIndex / TEXTURES_PER_ROW;
    int col = blockIndex % TEXTURES_PER_ROW;
    
    float u = col * TEXTURE_SIZE / ATLAS_SIZE;
    float v = row * TEXTURE_SIZE / ATLAS_SIZE;
    float du = TEXTURE_SIZE / ATLAS_SIZE;
    float dv = TEXTURE_SIZE / ATLAS_SIZE;
    
    // UV coordinates for a quad (bottom-left, bottom-right, top-right, top-left)
    texCoords[0] = u;      texCoords[1] = v;       // Bottom-left
    texCoords[2] = u + du; texCoords[3] = v;       // Bottom-right
    texCoords[4] = u + du; texCoords[5] = v + dv;  // Top-right
    texCoords[6] = u;      texCoords[7] = v + dv;  // Top-left
}

void Renderer::renderPlayerModel() {
    // Save current transformation matrix
    glPushMatrix();
    
    // Position relative to camera for first-person view
    glLoadIdentity();
    
    // Apply only pitch rotation (not yaw, as arms should rotate with view)
    glRotatef(-cameraPitch, 1.0f, 0.0f, 0.0f);
    
    // Calculate arm swing angles
    float rightArmAngle = 0.0f;
    float leftArmAngle = 0.0f;
    
    if (isSwinging) {
        // Mining animation: lift up (0-0.5), then swing down (0.5-1.0)
        float shoulderAngle, elbowAngle;
        float leftShoulderAngle = 0.0f, leftElbowAngle = 0.0f;
        
        if (swingProgress < 0.4f) {
            // Phase 1: Lift arm up and back, bend elbow (preparing to swing)
            float liftProgress = swingProgress / 0.4f;
            shoulderAngle = -60.0f * liftProgress; // Lift shoulder back
            elbowAngle = 90.0f * liftProgress;     // Bend elbow
            leftShoulderAngle = shoulderAngle * 0.1f;
        } else {
            // Phase 2: Swing down hard (like striking)
            float swingPhase = (swingProgress - 0.4f) / 0.6f;
            shoulderAngle = -60.0f + (120.0f * swingPhase); // Swing from back to front
            elbowAngle = 90.0f - (60.0f * swingPhase);      // Straighten elbow as we swing
            leftShoulderAngle = shoulderAngle * 0.1f;
        }
        
        rightArmAngle = shoulderAngle;
        leftArmAngle = leftShoulderAngle;
        
        // Store elbow angle for rendering
        this->currentElbowAngle = elbowAngle;
        
        // Debug output
        std::cout << "Mining: progress=" << swingProgress << " shoulder=" << shoulderAngle << " elbow=" << elbowAngle << std::endl;
    } else {
        // Normal walking animation based on movement
        static float armSwingTimer = 0.0f;
        float armSwingSpeed = sqrt(velocity.x * velocity.x + velocity.z * velocity.z) * 0.5f;
        armSwingTimer += armSwingSpeed;
        float walkingSwing = sin(armSwingTimer) * 20.0f; // 20 degree arm swing
        
        rightArmAngle = walkingSwing;
        leftArmAngle = -walkingSwing * 0.7f; // Left arm swings opposite, less pronounced
        currentElbowAngle = 0.0f; // No elbow bend during walking
    }
    
    // Render right arm (holding tool)
    renderArm(false, rightArmAngle, currentElbowAngle);
    
    // Render left arm (empty)
    renderArm(true, leftArmAngle, 0.0f);
    
    // Restore transformation matrix
    glPopMatrix();
}

void Renderer::renderArm(bool leftArm, float shoulderAngle, float elbowAngle) {
    glPushMatrix();
    
    // Position arm relative to camera - much closer for attached feel
    float armX = leftArm ? -0.2f : 0.2f;  // Closer left/right offset
    float armY = -0.3f;                    // Slightly below camera
    float armZ = -0.4f;                    // Much closer to camera
    
    glTranslatef(armX, armY, armZ);
    glRotatef(shoulderAngle, 1.0f, 0.0f, 0.0f); // Shoulder rotation
    
    // Upper arm dimensions (shoulder to elbow)
    float upperArmWidth = 0.1f;
    float upperArmHeight = 0.18f; // Half the original height
    float armDepth = 0.1f;
    
    // Set arm color (skin tone)
    glColor3f(0.9f, 0.7f, 0.6f);
    
    // Render upper arm as a rectangular prism
    glBegin(GL_QUADS);
    
    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-upperArmWidth/2, -upperArmHeight, armDepth/2);
    glVertex3f(upperArmWidth/2, -upperArmHeight, armDepth/2);
    glVertex3f(upperArmWidth/2, 0, armDepth/2);
    glVertex3f(-upperArmWidth/2, 0, armDepth/2);
    
    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(upperArmWidth/2, -upperArmHeight, -armDepth/2);
    glVertex3f(-upperArmWidth/2, -upperArmHeight, -armDepth/2);
    glVertex3f(-upperArmWidth/2, 0, -armDepth/2);
    glVertex3f(upperArmWidth/2, 0, -armDepth/2);
    
    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-upperArmWidth/2, 0, armDepth/2);
    glVertex3f(upperArmWidth/2, 0, armDepth/2);
    glVertex3f(upperArmWidth/2, 0, -armDepth/2);
    glVertex3f(-upperArmWidth/2, 0, -armDepth/2);
    
    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-upperArmWidth/2, -upperArmHeight, -armDepth/2);
    glVertex3f(upperArmWidth/2, -upperArmHeight, -armDepth/2);
    glVertex3f(upperArmWidth/2, -upperArmHeight, armDepth/2);
    glVertex3f(-upperArmWidth/2, -upperArmHeight, armDepth/2);
    
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(upperArmWidth/2, -upperArmHeight, armDepth/2);
    glVertex3f(upperArmWidth/2, -upperArmHeight, -armDepth/2);
    glVertex3f(upperArmWidth/2, 0, -armDepth/2);
    glVertex3f(upperArmWidth/2, 0, armDepth/2);
    
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-upperArmWidth/2, -upperArmHeight, -armDepth/2);
    glVertex3f(-upperArmWidth/2, -upperArmHeight, armDepth/2);
    glVertex3f(-upperArmWidth/2, 0, armDepth/2);
    glVertex3f(-upperArmWidth/2, 0, -armDepth/2);
    
    glEnd();
    
    // Now render the forearm (elbow to hand)
    // Move to the end of upper arm and apply elbow rotation
    glTranslatef(0.0f, -upperArmHeight, 0.0f); // Move to elbow position
    glRotatef(elbowAngle, 1.0f, 0.0f, 0.0f); // Elbow rotation
    
    // Forearm dimensions
    float forearmWidth = 0.08f;
    float forearmHeight = 0.17f;
    float forearmDepth = 0.08f;
    
    // Render forearm
    glBegin(GL_QUADS);
    
    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-forearmWidth/2, -forearmHeight, forearmDepth/2);
    glVertex3f(forearmWidth/2, -forearmHeight, forearmDepth/2);
    glVertex3f(forearmWidth/2, 0, forearmDepth/2);
    glVertex3f(-forearmWidth/2, 0, forearmDepth/2);
    
    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(forearmWidth/2, -forearmHeight, -forearmDepth/2);
    glVertex3f(-forearmWidth/2, -forearmHeight, -forearmDepth/2);
    glVertex3f(-forearmWidth/2, 0, -forearmDepth/2);
    glVertex3f(forearmWidth/2, 0, -forearmDepth/2);
    
    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-forearmWidth/2, 0, forearmDepth/2);
    glVertex3f(forearmWidth/2, 0, forearmDepth/2);
    glVertex3f(forearmWidth/2, 0, -forearmDepth/2);
    glVertex3f(-forearmWidth/2, 0, -forearmDepth/2);
    
    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-forearmWidth/2, -forearmHeight, -forearmDepth/2);
    glVertex3f(forearmWidth/2, -forearmHeight, -forearmDepth/2);
    glVertex3f(forearmWidth/2, -forearmHeight, forearmDepth/2);
    glVertex3f(-forearmWidth/2, -forearmHeight, forearmDepth/2);
    
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(forearmWidth/2, -forearmHeight, forearmDepth/2);
    glVertex3f(forearmWidth/2, -forearmHeight, -forearmDepth/2);
    glVertex3f(forearmWidth/2, 0, -forearmDepth/2);
    glVertex3f(forearmWidth/2, 0, forearmDepth/2);
    
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-forearmWidth/2, -forearmHeight, -forearmDepth/2);
    glVertex3f(-forearmWidth/2, -forearmHeight, forearmDepth/2);
    glVertex3f(-forearmWidth/2, 0, forearmDepth/2);
    glVertex3f(-forearmWidth/2, 0, -forearmDepth/2);
    
    glEnd();
    
    // Render tool in right hand (at end of forearm)
    if (!leftArm) {
        glTranslatef(0.0f, -forearmHeight - 0.05f, 0.0f);
        extern BlockType selectedBlockType; // Access global selected block
        renderTool(selectedBlockType);
    }
    
    glPopMatrix();
}

void Renderer::renderTool(BlockType toolType) {
    glPushMatrix();
    
    // Tool dimensions (proportional to larger arms)
    float toolSize = 0.08f;
    
    glScalef(toolSize, toolSize, toolSize);
    
    // Set tool color based on block type
    switch (toolType) {
        case BlockType::GRASS:
            glColor3f(0.2f, 0.8f, 0.2f);  // Bright Green
            break;
        case BlockType::DIRT:
            glColor3f(0.6f, 0.4f, 0.2f);  // Brown
            break;
        case BlockType::STONE:
            glColor3f(0.6f, 0.6f, 0.6f);  // Gray
            break;
        case BlockType::WOOD:
            glColor3f(0.6f, 0.3f, 0.1f);  // Dark Brown
            break;
        case BlockType::LEAVES:
            glColor3f(0.1f, 0.6f, 0.1f);  // Dark Green
            break;
        case BlockType::WATER:
            glColor3f(0.2f, 0.4f, 0.8f);  // Blue
            break;
        case BlockType::SAND:
            glColor3f(0.9f, 0.8f, 0.6f);  // Sandy Yellow
            break;
        case BlockType::COAL_ORE:
            glColor3f(0.3f, 0.3f, 0.3f);  // Dark Gray
            break;
        case BlockType::IRON_ORE:
            glColor3f(0.8f, 0.7f, 0.6f);  // Beige
            break;
        case BlockType::DIAMOND_ORE:
            glColor3f(0.7f, 0.9f, 0.9f);  // Light Blue
            break;
        default:
            glColor3f(0.8f, 0.8f, 0.8f);  // Light Gray
    }
    
    // Render tool as a small cube
    glBegin(GL_QUADS);
    
    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1, -1, 1);
    glVertex3f(1, -1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(-1, 1, 1);
    
    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(1, -1, -1);
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, 1, -1);
    glVertex3f(1, 1, -1);
    
    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1, 1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, 1, -1);
    glVertex3f(-1, 1, -1);
    
    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-1, -1, -1);
    glVertex3f(1, -1, -1);
    glVertex3f(1, -1, 1);
    glVertex3f(-1, -1, 1);
    
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1, -1, 1);
    glVertex3f(1, -1, -1);
    glVertex3f(1, 1, -1);
    glVertex3f(1, 1, 1);
    
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, -1, 1);
    glVertex3f(-1, 1, 1);
    glVertex3f(-1, 1, -1);
    
    glEnd();
    
    glPopMatrix();
}

void Renderer::triggerArmSwing() {
    isSwinging = true;
    swingProgress = 0.0f;
    swingTimer = 0.0f;
    std::cout << "Arm swing triggered!" << std::endl;
}

void Renderer::toggleMenu() {
    showMenu = !showMenu;
    selectedMenuItem = 0; // Reset selection
    std::cout << (showMenu ? "Menu opened" : "Menu closed") << std::endl;
}

void Renderer::menuNavigate(int direction) {
    if (!showMenu) return;
    
    const int menuItemCount = 8; // Number of menu items (added flight mode)
    selectedMenuItem += direction;
    
    if (selectedMenuItem < 0) selectedMenuItem = menuItemCount - 1;
    if (selectedMenuItem >= menuItemCount) selectedMenuItem = 0;
}

void Renderer::menuSelect() {
    if (!showMenu) return;
    
    switch (selectedMenuItem) {
        case 0: // Resume Game
            toggleMenu();
            break;
        case 1: // Toggle Player Model
            showPlayerModel = !showPlayerModel;
            std::cout << "Player model: " << (showPlayerModel ? "ON" : "OFF") << std::endl;
            break;
        case 2: // Toggle Debug Info
            showDebugInfo = !showDebugInfo;
            std::cout << "Debug info: " << (showDebugInfo ? "ON" : "OFF") << std::endl;
            break;
        case 3: // Toggle Flight Mode
            toggleFlightMode();
            break;
        case 4: // Render Mode
            if (mode == RenderMode::SOLID) {
                mode = RenderMode::WIREFRAME;
                std::cout << "Render mode: WIREFRAME" << std::endl;
            } else if (mode == RenderMode::WIREFRAME) {
                mode = RenderMode::TEXTURED;
                std::cout << "Render mode: TEXTURED" << std::endl;
            } else {
                mode = RenderMode::SOLID;
                std::cout << "Render mode: SOLID" << std::endl;
            }
            break;
        case 5: // Mouse Sensitivity
            if (mouseSensitivity <= 0.1f) {
                mouseSensitivity = std::min(0.5f, mouseSensitivity + 0.05f);
            } else {
                mouseSensitivity = 0.05f; // Reset to minimum
            }
            std::cout << "Mouse sensitivity: " << mouseSensitivity << std::endl;
            break;
        case 6: // Movement Speed
            if (movementSpeed >= 12.0f) {
                movementSpeed = 4.0f; // Reset to minimum
            } else {
                movementSpeed += 2.0f;
            }
            std::cout << "Movement speed: " << movementSpeed << std::endl;
            break;
        case 7: // Quit Game
            std::cout << "Exiting game..." << std::endl;
            exit(0);
            break;
    }
}

void Renderer::renderText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

void Renderer::renderMenu() {
    // Switch to 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable depth testing for menu
    glDisable(GL_DEPTH_TEST);
    
    // Semi-transparent background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();
    
    glDisable(GL_BLEND);
    
    // Menu title
    glColor3f(1.0f, 1.0f, 1.0f);
    renderText(0.32f, 0.85f, "MY-CRAFT by Kelsi Davis - SETTINGS");
    
    for (int i = 0; i < 8; i++) {
        float yPos = 0.7f - i * 0.08f;
        
        // Highlight selected item
        if (i == selectedMenuItem) {
            glColor3f(1.0f, 1.0f, 0.0f); // Yellow
            renderText(0.25f, yPos, "> ");
        } else {
            glColor3f(0.8f, 0.8f, 0.8f); // Light gray
        }
        
        char buffer[256];
        switch (i) {
            case 0:
                renderText(0.3f, yPos, "Resume Game");
                break;
            case 1:
                snprintf(buffer, sizeof(buffer), "Player Model: %s", showPlayerModel ? "ON" : "OFF");
                renderText(0.3f, yPos, buffer);
                break;
            case 2:
                snprintf(buffer, sizeof(buffer), "Debug Info: %s", showDebugInfo ? "ON" : "OFF");
                renderText(0.3f, yPos, buffer);
                break;
            case 3:
                snprintf(buffer, sizeof(buffer), "Flight Mode: %s", flightMode ? "ON" : "OFF");
                renderText(0.3f, yPos, buffer);
                break;
            case 4: {
                const char* modeStr = (mode == RenderMode::SOLID) ? "SOLID" : 
                                     (mode == RenderMode::WIREFRAME) ? "WIREFRAME" : "TEXTURED";
                snprintf(buffer, sizeof(buffer), "Render Mode: %s", modeStr);
                renderText(0.3f, yPos, buffer);
                break;
            }
            case 5:
                snprintf(buffer, sizeof(buffer), "Mouse Sensitivity: %.2f", mouseSensitivity);
                renderText(0.3f, yPos, buffer);
                break;
            case 6:
                snprintf(buffer, sizeof(buffer), "Movement Speed: %.1f", movementSpeed);
                renderText(0.3f, yPos, buffer);
                break;
            case 7:
                renderText(0.3f, yPos, "Quit Game");
                break;
        }
    }
    
    // Controls help
    glColor3f(0.6f, 0.6f, 0.6f);
    renderText(0.25f, 0.15f, "UP/DOWN: Navigate");
    renderText(0.25f, 0.10f, "ENTER: Select/Toggle");
    renderText(0.25f, 0.05f, "ESC: Close Menu");
    
    // Restore 3D rendering
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Renderer::toggleInventory() {
    showInventory = !showInventory;
    std::cout << (showInventory ? "Inventory opened" : "Inventory closed") << std::endl;
}

void Renderer::renderHotbar() {
    // Switch to 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    // Render hotbar background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    
    float hotbarWidth = 0.6f;
    float hotbarHeight = 0.08f;
    float hotbarX = (1.0f - hotbarWidth) / 2.0f;
    float hotbarY = 0.02f;
    
    glBegin(GL_QUADS);
    glVertex2f(hotbarX, hotbarY);
    glVertex2f(hotbarX + hotbarWidth, hotbarY);
    glVertex2f(hotbarX + hotbarWidth, hotbarY + hotbarHeight);
    glVertex2f(hotbarX, hotbarY + hotbarHeight);
    glEnd();
    
    // Render hotbar slots
    float slotWidth = hotbarWidth / 9.0f;
    int selectedSlot = inventory.getSelectedSlotIndex();
    
    for (int i = 0; i < 9; i++) {
        float slotX = hotbarX + i * slotWidth;
        
        // Highlight selected slot
        if (i == selectedSlot) {
            glColor4f(1.0f, 1.0f, 0.0f, 0.8f); // Yellow highlight
        } else {
            glColor4f(0.3f, 0.3f, 0.3f, 0.8f); // Gray slot
        }
        
        glBegin(GL_QUADS);
        glVertex2f(slotX + 0.002f, hotbarY + 0.002f);
        glVertex2f(slotX + slotWidth - 0.002f, hotbarY + 0.002f);
        glVertex2f(slotX + slotWidth - 0.002f, hotbarY + hotbarHeight - 0.002f);
        glVertex2f(slotX + 0.002f, hotbarY + hotbarHeight - 0.002f);
        glEnd();
        
        // Render slot number and item info
        glColor3f(1.0f, 1.0f, 1.0f);
        char slotText[64];
        sprintf(slotText, "%d", i + 1);
        renderText(slotX + 0.005f, hotbarY + hotbarHeight + 0.01f, slotText);
        
        // Render item name and count
        const InventorySlot& slot = inventory.getSlot(i);
        if (!slot.isEmpty()) {
            Block tempBlock(slot.itemType);
            sprintf(slotText, "%s x%d", tempBlock.toString().substr(0, 4).c_str(), slot.count);
            renderText(slotX + 0.005f, hotbarY - 0.03f, slotText);
        }
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Renderer::renderInventory() {
    // Switch to 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    // Semi-transparent background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();
    
    glDisable(GL_BLEND);
    
    // Inventory title
    glColor3f(1.0f, 1.0f, 1.0f);
    renderText(0.4f, 0.9f, "INVENTORY");
    
    // Render hotbar section
    glColor3f(0.8f, 0.8f, 0.0f);
    renderText(0.1f, 0.8f, "Hotbar:");
    
    for (int i = 0; i < 9; i++) {
        float x = 0.1f + (i * 0.08f);
        float y = 0.75f;
        
        // Highlight selected slot
        if (i == inventory.getSelectedSlotIndex()) {
            glColor3f(1.0f, 1.0f, 0.0f);
        } else {
            glColor3f(0.7f, 0.7f, 0.7f);
        }
        
        const InventorySlot& slot = inventory.getSlot(i);
        char slotText[64];
        sprintf(slotText, "[%d] %s", i + 1, slot.toString().substr(0, 8).c_str());
        renderText(x, y, slotText);
    }
    
    // Render main inventory
    glColor3f(0.8f, 0.8f, 0.8f);
    renderText(0.1f, 0.65f, "Main Inventory:");
    
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 9; col++) {
            int slotIndex = 9 + (row * 9) + col;
            float x = 0.1f + (col * 0.08f);
            float y = 0.6f - (row * 0.08f);
            
            const InventorySlot& slot = inventory.getSlot(slotIndex);
            if (!slot.isEmpty()) {
                glColor3f(0.9f, 0.9f, 0.9f);
                char slotText[64];
                sprintf(slotText, "%s x%d", slot.toString().substr(0, 6).c_str(), slot.count);
                renderText(x, y, slotText);
            } else {
                glColor3f(0.4f, 0.4f, 0.4f);
                renderText(x, y, "[empty]");
            }
        }
    }
    
    // Controls help
    glColor3f(0.6f, 0.6f, 0.6f);
    renderText(0.1f, 0.15f, "TAB: Close Inventory");
    renderText(0.1f, 0.10f, "1-9: Select Hotbar Slot");
    renderText(0.1f, 0.05f, "I: Toggle Inventory");
    
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Renderer::selectHotbarSlot(int slot) {
    inventory.selectSlot(slot);
}
