# 🎨 Minecraft C++ Sprite Setup Guide

## ✅ **System Ready!**

Your Minecraft C++ now supports **sprite-based textures**!

### **Current Status:**
- ✅ **Larger World**: 128×512×128 blocks (8×4×8 chunks)
- ✅ **Sprite System**: Loads 64×64 PPM images
- ✅ **Auto-generated Sprites**: 11 unique block textures already created!
- ✅ **Fallback System**: Uses procedural textures if sprites missing

---

## 🚀 **Quick Start (Already Done!)**

The sprites are **already generated**! Just run:

```bash
make && ./minecraft
# Press 'Y' for textured mode
# Press 'T' for solid colors  
# Press 'R' for wireframe
```

---

## 📁 **Generated Sprites**

Located in `assets/textures/`:

| Block Type | File | Description |
|------------|------|-------------|
| Grass | `grass.ppm` | Green grass with natural variations |
| Dirt | `dirt.ppm` | Brown soil texture |  
| Stone | `stone.ppm` | Gray stone with cracks |
| Wood | `wood.ppm` | Wood grain pattern |
| Leaves | `leaves.ppm` | Dark green leafy texture |
| Water | `water.ppm` | Blue water with wave patterns |
| Sand | `sand.ppm` | Sandy yellow/beige texture |
| Coal Ore | `coal_ore.ppm` | Stone with black coal spots |
| Iron Ore | `iron_ore.ppm` | Stone with brown iron veins |
| Diamond Ore | `diamond_ore.ppm` | Stone with cyan diamond spots |
| Air | `air.ppm` | Light blue (rarely seen) |

---

## 🛠️ **Creating Custom Sprites**

### **Method 1: Regenerate All Sprites**
```bash
python3 generate_sprites.py
```

### **Method 2: Edit Individual Sprites**
1. **Open any `.ppm` file** in `assets/textures/`
2. **Edit with GIMP, Photoshop, or any image editor**
3. **Save as 64×64 RGB image**
4. **Convert to PPM format**

### **Method 3: Replace with Your Own Images**

#### **Using GIMP:**
1. Create **64×64 image**
2. Design your block texture
3. **Export As** → Choose "PPM" format
4. Save in `assets/textures/` with correct name

#### **Using ImageMagick:**
```bash
# Convert any image to PPM
convert your_texture.png -resize 64x64 assets/textures/grass.ppm

# Batch convert multiple images
for img in *.png; do
    convert "$img" -resize 64x64 "assets/textures/${img%.png}.ppm"
done
```

#### **Using Python/PIL:**
```python
from PIL import Image

# Load and resize any image
img = Image.open("your_texture.jpg")
img = img.resize((64, 64))
img = img.convert('RGB')

# Save as PPM
with open("assets/textures/grass.ppm", 'wb') as f:
    f.write(f"P6\n64 64\n255\n".encode())
    for pixel in img.getdata():
        f.write(bytes(pixel))
```

---

## 📐 **Sprite Requirements**

### **Technical Specs:**
- ✅ **Size**: Exactly 64×64 pixels
- ✅ **Format**: PPM (P6 binary format)
- ✅ **Colors**: RGB (no alpha channel)
- ✅ **Pixel Art Style**: Recommended (uses GL_NEAREST filtering)

### **File Naming:**
Must match these exact names:
```
assets/textures/
├── air.ppm
├── grass.ppm
├── dirt.ppm
├── stone.ppm
├── wood.ppm
├── leaves.ppm
├── water.ppm
├── sand.ppm
├── coal_ore.ppm
├── iron_ore.ppm
└── diamond_ore.ppm
```

---

## 🎨 **Advanced Sprite Creation**

### **Pixel Art Tools:**
- **Aseprite** (paid, best for pixel art)
- **Piskel** (free, web-based)
- **GIMP** (free, set to "None" interpolation)
- **MS Paint** (surprisingly good for pixel art!)

### **Design Tips:**
1. **High Contrast**: Make textures clearly distinguishable
2. **Tileable**: Textures should seamlessly repeat
3. **Minecraft Style**: Look at official Minecraft textures for inspiration
4. **Distinct Colors**: Each block type should be unique

### **Color Palettes:**
- **Grass**: Greens (34-180 range)
- **Dirt**: Browns (80-140 range)  
- **Stone**: Grays (80-160 range)
- **Wood**: Browns with grain
- **Water**: Blues (100-255 range)
- **Ores**: Base stone + ore color spots

---

## 🔧 **Troubleshooting**

### **"Failed to load image file" Error:**
- Check file exists in `assets/textures/`
- Verify filename matches exactly (case sensitive)
- Ensure PPM format is correct

### **"Wrong size" Warning:**
- Resize image to exactly 64×64 pixels
- System will fall back to procedural texture

### **Textures Look Wrong:**
- Check PPM format is P6 (binary), not P3 (ASCII)
- Verify RGB color order (not BGR)
- Make sure file isn't corrupted

### **Debug Atlas:**
- Run the game to generate `texture_atlas_debug.ppm`
- Open this file to see how your textures are arranged
- Each 64×64 block shows one texture

---

## 🚀 **Performance Notes**

The sprite system is **highly optimized**:
- ✅ **Single texture atlas** (fast GPU access)
- ✅ **Face culling** (hidden faces not rendered)
- ✅ **Distance culling** (far chunks not rendered)
- ✅ **Fallback system** (missing sprites don't crash)

**Expected Performance:** 30-60 FPS on modern systems

---

## 📦 **Example Workflow**

1. **Create sprites** in your favorite editor (64×64)
2. **Save as PNG/JPG** initially
3. **Convert to PPM** using ImageMagick or Python
4. **Place in** `assets/textures/`
5. **Run game** and press 'Y' to see textures
6. **Check** `texture_atlas_debug.ppm` for verification

---

## 🎮 **Ready to Play!**

Your enhanced Minecraft world now features:
- **🌍 Large world**: 128×512×128 blocks
- **🎨 Custom sprites**: All block types have unique textures  
- **⚡ High performance**: Optimized rendering pipeline
- **🎯 Easy customization**: Drop in new sprites anytime

**Run `make && ./minecraft` and press 'Y' to experience your textured world!**