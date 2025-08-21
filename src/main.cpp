#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Renderer.h"
#include "World.h"

// Global objects
Renderer* renderer = nullptr;
World* world = nullptr;

// GLUT callbacks
void display() {
    if (renderer) {
        renderer->render();
    }
}

void idle() {
    if (renderer) {
        renderer->update(0.016f); // ~60 FPS
    }
    glutPostRedisplay();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    // The perspective will be handled by the renderer's setupCamera method
}

// Global selected block type
BlockType selectedBlockType = BlockType::GRASS;

void keyboard(unsigned char key, int x, int y) {
    // Handle menu navigation first
    if (renderer && renderer->isMenuOpen()) {
        switch (key) {
            case 27: // ESC - close menu
                renderer->toggleMenu();
                return;
            case '\r': // Enter - select menu item
            case '\n':
                renderer->menuSelect();
                return;
            default:
                return; // Block other keys when menu is open
        }
    }
    
    switch (key) {
        case 'w':
            renderer->moveCameraPhysics(1.0f, 0.0f);  // Forward
            break;
        case 's':
            renderer->moveCameraPhysics(-1.0f, 0.0f); // Backward
            break;
        case 'a':
            renderer->moveCameraPhysics(0.0f, -1.0f); // Left
            break;
        case 'd':
            renderer->moveCameraPhysics(0.0f, 1.0f);  // Right
            break;
        case ' ':  // Space for jump/swim up
            renderer->jump();
            break;
        case 'c':  // C for swim down/creative fly down
            renderer->swimDown();
            break;
        case 'r':
            renderer->setRenderMode(RenderMode::WIREFRAME);
            break;
        case 't':
            renderer->setRenderMode(RenderMode::SOLID);
            break;
        case 'y':
            renderer->setRenderMode(RenderMode::TEXTURED);
            break;
        case 'f':
            renderer->toggleFlightMode();
            break;
        case 'g':
            std::cout << "Manual arm swing test" << std::endl;
            renderer->triggerArmSwing();
            break;
        // Block selection
        case '1':
            selectedBlockType = BlockType::GRASS;
            std::cout << "Selected: Grass" << std::endl;
            break;
        case '2':
            selectedBlockType = BlockType::DIRT;
            std::cout << "Selected: Dirt" << std::endl;
            break;
        case '3':
            selectedBlockType = BlockType::STONE;
            std::cout << "Selected: Stone" << std::endl;
            break;
        case '4':
            selectedBlockType = BlockType::WOOD;
            std::cout << "Selected: Wood" << std::endl;
            break;
        case '5':
            selectedBlockType = BlockType::LEAVES;
            std::cout << "Selected: Leaves" << std::endl;
            break;
        case '6':
            selectedBlockType = BlockType::SAND;
            std::cout << "Selected: Sand" << std::endl;
            break;
        case '7':
            selectedBlockType = BlockType::WATER;
            std::cout << "Selected: Water" << std::endl;
            break;
        case '8':
            selectedBlockType = BlockType::COAL_ORE;
            std::cout << "Selected: Coal Ore" << std::endl;
            break;
        case '9':
            selectedBlockType = BlockType::IRON_ORE;
            std::cout << "Selected: Iron Ore" << std::endl;
            break;
        case '0':
            selectedBlockType = BlockType::DIAMOND_ORE;
            std::cout << "Selected: Diamond Ore" << std::endl;
            break;
        case 27: // ESC - open menu
            renderer->toggleMenu();
            break;
    }
}

void specialKeys(int key, int x, int y) {
    if (renderer && renderer->isMenuOpen()) {
        switch (key) {
            case GLUT_KEY_UP:
                renderer->menuNavigate(-1);
                break;
            case GLUT_KEY_DOWN:
                renderer->menuNavigate(1);
                break;
        }
    }
}

void mouse(int button, int state, int x, int y) {
    std::cout << "Mouse event: button=" << button << " state=" << state << std::endl;
    
    // Don't process mouse clicks if menu is open
    if (renderer && renderer->isMenuOpen()) {
        std::cout << "Menu is open - ignoring mouse click" << std::endl;
        return;
    }
    
    // Handle mouse wheel (buttons 3 and 4 in GLUT)
    if (state == GLUT_DOWN && renderer) {
        if (button == 3) { // Scroll up - zoom in
            renderer->zoom(0.9f);
            return;
        } else if (button == 4) { // Scroll down - zoom out
            renderer->zoom(1.1f);
            return;
        }
    }
    
    if (state == GLUT_DOWN && renderer && world) {
        Vector3 pos = renderer->getCameraPosition();
        
        // Get camera direction from renderer
        float cameraYaw = renderer->getCameraYaw();
        float cameraPitch = renderer->getCameraPitch();
        
        // Calculate ray direction
        float yawRad = cameraYaw * 3.14159f / 180.0f;
        float pitchRad = cameraPitch * 3.14159f / 180.0f;
        
        float rayX = -sin(yawRad) * cos(pitchRad);
        float rayY = -sin(pitchRad);
        float rayZ = -cos(yawRad) * cos(pitchRad);
        
        std::cout << "Ray direction: (" << rayX << "," << rayY << "," << rayZ << ")" << std::endl;
        
        // Raycast forward to find block
        float stepSize = 0.1f;
        float maxDistance = 5.0f;
        std::cout << "Starting raycast from (" << pos.x << "," << pos.y << "," << pos.z << ")" << std::endl;
        
        // Simple test: just try blocks directly in front of player
        if (button == GLUT_LEFT_BUTTON) {
            std::cout << "Left click detected - triggering arm swing" << std::endl;
            renderer->triggerArmSwing();
            
            // Try breaking block directly in front
            int testX = (int)(pos.x + rayX * 2.0f);
            int testY = (int)(pos.y + rayY * 2.0f);  
            int testZ = (int)(pos.z + rayZ * 2.0f);
            
            Block* testBlock = world->getBlockAt(testX, testY, testZ);
            if (testBlock && !testBlock->isEmpty()) {
                *testBlock = Block(BlockType::AIR);
                std::cout << "Broke test block at (" << testX << "," << testY << "," << testZ << ")" << std::endl;
            } else {
                std::cout << "No block found at (" << testX << "," << testY << "," << testZ << ")" << std::endl;
            }
        }
        
        // Original raycast code for more precise detection
        for (float dist = 0.5f; dist < maxDistance; dist += stepSize) {
            int blockX = (int)(pos.x + rayX * dist);
            int blockY = (int)(pos.y + rayY * dist);
            int blockZ = (int)(pos.z + rayZ * dist);
            
            Block* block = world->getBlockAt(blockX, blockY, blockZ);
            if (block && !block->isEmpty()) {
                if (button == GLUT_RIGHT_BUTTON) {
                    // Place block one step back
                    int placeX = (int)(pos.x + rayX * (dist - stepSize));
                    int placeY = (int)(pos.y + rayY * (dist - stepSize));
                    int placeZ = (int)(pos.z + rayZ * (dist - stepSize));
                    
                    Block* placeBlock = world->getBlockAt(placeX, placeY, placeZ);
                    if (placeBlock && placeBlock->isEmpty()) {
                        // Trigger arm swing animation for placement too
                        renderer->triggerArmSwing();
                        *placeBlock = Block(selectedBlockType);
                        std::cout << "Placed " << placeBlock->toString() << " at (" << placeX << "," << placeY << "," << placeZ << ")" << std::endl;
                    }
                }
                break; // Stop at first solid block
            }
        }
    }
}

void motion(int x, int y) {
    // Don't process mouse movement when menu is open
    if (renderer && renderer->isMenuOpen()) {
        return;
    }
    
    static int lastX = 400, lastY = 300;
    static bool firstMouse = true;
    
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }
    
    float xoffset = x - lastX;
    float yoffset = y - lastY; // Fixed mouse inversion
    
    lastX = x;
    lastY = y;
    
    float sensitivity = renderer->getMouseSensitivity();
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    if (renderer) {
        renderer->rotateCamera(xoffset, yoffset);
    }
}


int main(int argc, char** argv) {
    // Initialize random seed for world generation
    srand(time(nullptr));
    
    std::cout << "MY-CRAFT by Kelsi Davis - Started!" << std::endl;
    std::cout << "High Resolution Voxel World with Physics & Biomes!" << std::endl;
    std::cout << "Random seed: " << time(nullptr) << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move with physics (gravity & swimming)" << std::endl;
    std::cout << "  Space - Jump/Swim up, C - Swim down/Creative fly down" << std::endl;
    std::cout << "  Mouse - Look around (smooth mouse look)" << std::endl;
    std::cout << "  Mouse Wheel - Zoom in/out (10°-120° FOV)" << std::endl;
    std::cout << "  Left Click - Break blocks, Right Click - Place blocks" << std::endl;
    std::cout << "  1-0 - Select blocks: Grass/Dirt/Stone/Wood/Leaves/Sand/Water/Coal/Iron/Diamond" << std::endl;
    std::cout << "  R/T/Y - Wireframe/Solid/Textured render modes" << std::endl;
    std::cout << "  F - Toggle Flight Mode (Free floating)" << std::endl;
    std::cout << "  ESC - Settings Menu" << std::endl;
    std::cout << "World: 128x1024x128 blocks with biomes, ores, trees, water!" << std::endl;
    
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("MY-CRAFT by Kelsi Davis");
    
    // Initialize OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Sky blue background
    
    // Create game objects
    world = new World();
    world->generateWorld();
    
    renderer = new Renderer(world);
    renderer->init();
    renderer->initPlayerPosition(); // Set player on solid ground
    
    // Register callbacks
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(motion);
    
    // Hide cursor and center it
    glutSetCursor(GLUT_CURSOR_NONE);
    
    // Start main loop
    glutMainLoop();
    
    // Cleanup (won't reach here)
    delete renderer;
    delete world;
    
    return 0;
}
