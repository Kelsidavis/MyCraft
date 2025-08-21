#ifndef RENDERER_H
#define RENDERER_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <vector>
#include <string>
#include "World.h"
#include "Vector3.h"
#include "ImageLoader.h"
#include "Inventory.h"

// Block rendering modes
enum class RenderMode {
    WIREFRAME,
    SOLID,
    TEXTURED
};

class Renderer {
private:
    RenderMode mode;
    World* world;
    Vector3 cameraPosition;
    Vector3 velocity;
    float cameraYaw;
    float cameraPitch;
    bool isOnGround;
    bool isJumping;
    bool isInWater;
    bool isSwimming;
    
    // Camera zoom
    float fieldOfView;
    
    // Menu system
    bool showMenu;
    int selectedMenuItem;
    bool showPlayerModel;
    bool showDebugInfo;
    bool flightMode;
    float mouseSensitivity;
    float movementSpeed;
    
    // Inventory system
    Inventory inventory;
    bool showInventory;
    
    // Arm swing animation
    bool isSwinging;
    float swingProgress;
    float swingTimer;
    float currentElbowAngle;
    
    // Texture system
    GLuint textureAtlas;
    bool texturesLoaded;
    
public:
    Renderer(World* w);
    ~Renderer();
    
    void init();
    void initPlayerPosition();
    void render();
    void update(float deltaTime);
    
    void setRenderMode(RenderMode m) { mode = m; }
    RenderMode getRenderMode() const { return mode; }
    
    void setCameraPosition(Vector3 pos) { cameraPosition = pos; }
    Vector3 getCameraPosition() const { return cameraPosition; }
    float getCameraYaw() const { return cameraYaw; }
    float getCameraPitch() const { return cameraPitch; }
    float getMouseSensitivity() const { return mouseSensitivity; }
    
    void moveCamera(float dx, float dy, float dz);
    void moveCameraPhysics(float forward, float right);
    void jump();
    void swimDown();
    void flyUp();
    void flyDown();
    void toggleFlightMode();
    bool isFlying() const { return flightMode; }
    void rotateCamera(float yaw, float pitch);
    void zoom(float factor);
    bool isBlockAt(int x, int y, int z);
    bool isWaterAt(int x, int y, int z);
    int findGroundLevel(int x, int z);
    bool shouldRenderFace(int x, int y, int z, int faceDirection);
    
    // Texture methods
    bool loadTextures();
    void getBlockTexCoords(BlockType blockType, float* texCoords);
    
    // Player model methods
    void renderPlayerModel();
    void renderArm(bool leftArm, float shoulderAngle = 0.0f, float elbowAngle = 0.0f);
    void renderTool(BlockType toolType);
    void triggerArmSwing();
    
    // Menu methods
    void toggleMenu();
    bool isMenuOpen() const { return showMenu; }
    void menuNavigate(int direction);
    void menuSelect();
    void renderMenu();
    void renderText(float x, float y, const char* text);
    
    // Inventory methods
    void toggleInventory();
    bool isInventoryOpen() const { return showInventory; }
    void renderInventory();
    void renderHotbar();
    void selectHotbarSlot(int slot);
    Inventory& getInventory() { return inventory; }
    
private:
    void renderWorld();
    void renderChunk(Chunk* chunk);
    void renderBlock(Block& block, int x, int y, int z);
    void setupCamera();
    void setupLighting();
};

#endif // RENDERER_H
