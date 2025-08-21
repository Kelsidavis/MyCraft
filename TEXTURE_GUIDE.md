# Minecraft C++ Texture Setup Guide

## Current Status 
The texture system is **already implemented** with procedural textures! You can use it right now:

### **How to Use Textures**
1. **Run the game**: `./minecraft`
2. **Press 'Y'** to switch to **TEXTURED mode**
3. **Press 'T'** to switch to **SOLID mode** (colored blocks)
4. **Press 'R'** to switch to **WIREFRAME mode**

## Current Features
-  **Black outlines** on blocks (fixed blending issue)
-  **Procedural texture atlas** (512x512 with 64x64 per block)
-  **10+ unique textures** for all block types
-  **Runtime texture generation** (no external files needed)

---

##  **For Custom Image Textures (Optional)**

If you want to replace the procedural textures with custom images:

### **Step 1: Create Texture Images**
Create 64x64 PNG images for each block type:
```
textures/
├── grass.png     (bright green with texture)
├── dirt.png      (brown soil texture)  
├── stone.png     (gray rocky texture)
├── wood.png      (brown wood grain)
├── leaves.png    (green leafy texture)
├── sand.png      (yellow sandy texture)
├── water.png     (blue watery texture)
├── coal_ore.png  (gray stone with black spots)
├── iron_ore.png  (gray stone with brown spots)
└── diamond_ore.png (gray stone with cyan spots)
```

### **Step 2: Install Image Loading Library**
Install SOIL (Simple OpenGL Image Library):
```bash
# Ubuntu/Debian
sudo apt-get install libsoil-dev

# Or download from: http://lonesock.net/soil.html
```

### **Step 3: Update CMakeLists.txt**
Add SOIL linking:
```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(SOIL REQUIRED soil)

target_link_libraries(minecraft ${SOIL_LIBRARIES})
target_include_directories(minecraft PRIVATE ${SOIL_INCLUDE_DIRS})
```

### **Step 4: Replace Texture Loading Code**
Update `loadTextures()` in `Renderer.cpp`:

```cpp
#include <SOIL/SOIL.h>

bool Renderer::loadTextures() {
    // Load texture atlas from file
    int width, height, channels;
    unsigned char* image = SOIL_load_image("textures/atlas.png", 
                                          &width, &height, &channels, 
                                          SOIL_LOAD_RGB);
    if (!image) {
        std::cout << "Failed to load texture atlas!" << std::endl;
        return false;
    }
    
    glGenTextures(1, &textureAtlas);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, 
                 GL_RGB, GL_UNSIGNED_BYTE, image);
    
    SOIL_free_image_data(image);
    texturesLoaded = true;
    return true;
}
```

---

##  **Creating Texture Atlas**

### **Option A: Manual Atlas Creation**
1. Create a 512x512 PNG image
2. Arrange your 64x64 textures in an 8x8 grid:
```
Row 0: [AIR] [GRASS] [DIRT] [STONE] [WOOD] [LEAVES] [WATER] [SAND]
Row 1: [COAL] [IRON] [DIAMOND] [unused] [unused] [unused] [unused] [unused]
```

### **Option B: Use ImageMagick**
```bash
# Install ImageMagick
sudo apt-get install imagemagick

# Create atlas from individual textures
montage grass.png dirt.png stone.png wood.png leaves.png sand.png water.png coal_ore.png iron_ore.png diamond_ore.png -tile 8x8 -geometry 64x64+0+0 atlas.png
```

### **Option C: Python Script**
```python
from PIL import Image
import os

# Create 512x512 atlas
atlas = Image.new('RGB', (512, 512), (128, 128, 128))

textures = ['grass.png', 'dirt.png', 'stone.png', 'wood.png', 
           'leaves.png', 'sand.png', 'water.png', 'coal_ore.png',
           'iron_ore.png', 'diamond_ore.png']

for i, texture_file in enumerate(textures):
    if os.path.exists(texture_file):
        img = Image.open(texture_file).resize((64, 64))
        x = (i % 8) * 64
        y = (i // 8) * 64
        atlas.paste(img, (x, y))

atlas.save('atlas.png')
print("Texture atlas created!")
```

---

##  **Texture Specifications**

### **Block Types & Atlas Positions**
| Block Type | Index | Row | Col | Position |
|------------|-------|-----|-----|----------|
| AIR | 0 | 0 | 0 | (0,0) |
| GRASS | 1 | 0 | 1 | (64,0) |
| DIRT | 2 | 0 | 2 | (128,0) |
| STONE | 3 | 0 | 3 | (192,0) |
| WOOD | 4 | 0 | 4 | (256,0) |
| LEAVES | 5 | 0 | 5 | (320,0) |
| WATER | 6 | 0 | 6 | (384,0) |
| SAND | 7 | 0 | 7 | (448,0) |
| COAL_ORE | 8 | 1 | 0 | (0,64) |
| IRON_ORE | 9 | 1 | 1 | (64,64) |
| DIAMOND_ORE | 10 | 1 | 2 | (128,64) |

### **Technical Requirements**
-  **Atlas Size**: 512x512 pixels
-  **Block Size**: 64x64 pixels each
-  **Format**: RGB (no alpha)
-  **Filtering**: GL_NEAREST (pixelated look)
-  **UV Coordinates**: Automatically calculated

---

##  **Quick Start (Using Current System)**

**The texture system is ready to use NOW!**

1. `./minecraft` - Start the game
2. **Press 'Y'** - Enable textured mode
3. **Walk around** - See procedural textures on all blocks
4. **Press 'T'** - Switch back to solid colors
5. **Build with different blocks** - Press 1-0 to select, right-click to place

**Features:**
- 🟩 **Grass**: Green noisy texture
- 🟫 **Dirt**: Brown soil pattern  
- ⬜ **Stone**: Gray rocky pattern
- 🟫 **Wood**: Vertical wood grain
- 🟩 **Leaves**: Dark green leafy pattern
- 🟦 **Water**: Blue wavy texture
- 🟨 **Sand**: Yellow sandy texture
- ⚫ **Coal Ore**: Gray with black spots
- 🟤 **Iron Ore**: Gray with brown spots  
- 💎 **Diamond Ore**: Gray with cyan spots


