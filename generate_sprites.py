#!/usr/bin/env python3
"""
Minecraft C++ Sprite Generator
Creates 64x64 PPM textures for all block types
"""

import os
import random

def create_texture_directory():
    """Create the textures directory if it doesn't exist"""
    os.makedirs("assets/textures", exist_ok=True)

def save_ppm(width, height, pixel_data, filename):
    """Save pixel data as PPM format"""
    with open(filename, 'w') as f:
        f.write(f"P6\n{width} {height}\n255\n")
    
    with open(filename, 'ab') as f:
        for pixel in pixel_data:
            f.write(bytes([pixel[0], pixel[1], pixel[2]]))
    print(f"Created: {filename}")

def create_grass_texture():
    """Create grass block texture"""
    pixels = []
    for y in range(64):
        for x in range(64):
            # Base green with variations
            r = random.randint(20, 60)
            g = random.randint(120, 180)  
            b = random.randint(20, 80)
            pixels.append((r, g, b))
    
    save_ppm(64, 64, pixels, "assets/textures/grass.ppm")

def create_dirt_texture():
    """Create dirt block texture"""
    pixels = []
    for y in range(64):
        for x in range(64):
            # Brown dirt variations
            base = random.randint(80, 140)
            r = base + random.randint(-20, 20)
            g = int(base * 0.6) + random.randint(-15, 15)
            b = int(base * 0.3) + random.randint(-10, 10)
            r, g, b = max(0, min(255, r)), max(0, min(255, g)), max(0, min(255, b))
            pixels.append((r, g, b))
    
    save_ppm(64, 64, pixels, "assets/textures/dirt.ppm")

def create_stone_texture():
    """Create stone block texture"""
    pixels = []
    for y in range(64):
        for x in range(64):
            # Gray stone with variations
            gray = random.randint(80, 160)
            # Add some cracks
            if (x + y) % 11 < 2:
                gray = random.randint(40, 80)  # Darker for cracks
            pixels.append((gray, gray, gray))
    
    save_ppm(64, 64, pixels, "assets/textures/stone.ppm")

def create_wood_texture():
    """Create wood block texture"""
    pixels = []
    for y in range(64):
        for x in range(64):
            # Wood grain pattern
            base = 100 + (y % 30) + random.randint(-15, 15)
            r = base + random.randint(-10, 10)
            g = int(base * 0.6) + random.randint(-8, 8)
            b = int(base * 0.3) + random.randint(-5, 5)
            r, g, b = max(50, min(200, r)), max(30, min(120, g)), max(10, min(60, b))
            pixels.append((r, g, b))
    
    save_ppm(64, 64, pixels, "assets/textures/wood.ppm")

def create_leaves_texture():
    """Create leaves block texture"""
    pixels = []
    for y in range(64):
        for x in range(64):
            # Dark green leaves
            r = random.randint(10, 50)
            g = random.randint(80, 150)
            b = random.randint(10, 60)
            pixels.append((r, g, b))
    
    save_ppm(64, 64, pixels, "assets/textures/leaves.ppm")

def create_water_texture():
    """Create water block texture"""
    pixels = []
    for y in range(64):
        for x in range(64):
            # Blue water with waves
            wave = int(20 * abs((x * 0.1) % 2 - 1))
            r = random.randint(20, 60)
            g = random.randint(100, 150) + wave
            b = random.randint(180, 240) + wave
            r, g, b = max(0, min(255, r)), max(0, min(255, g)), max(0, min(255, b))
            pixels.append((r, g, b))
    
    save_ppm(64, 64, pixels, "assets/textures/water.ppm")

def create_sand_texture():
    """Create sand block texture"""
    pixels = []
    for y in range(64):
        for x in range(64):
            # Sandy yellow/beige
            base = random.randint(200, 240)
            r = base
            g = int(base * 0.9) + random.randint(-10, 10)
            b = int(base * 0.7) + random.randint(-15, 15)
            r, g, b = max(180, min(255, r)), max(160, min(255, g)), max(120, min(200, b))
            pixels.append((r, g, b))
    
    save_ppm(64, 64, pixels, "assets/textures/sand.ppm")

def create_ore_texture(filename, ore_color):
    """Create ore block texture (stone base with ore spots)"""
    pixels = []
    for y in range(64):
        for x in range(64):
            # Stone base
            gray = random.randint(80, 140)
            pixel = [gray, gray, gray]
            
            # Add ore spots randomly
            if random.randint(1, 100) <= 15:  # 15% chance of ore spot
                pixel = [
                    ore_color[0] + random.randint(-20, 20),
                    ore_color[1] + random.randint(-20, 20), 
                    ore_color[2] + random.randint(-20, 20)
                ]
                # Clamp values
                pixel = [max(0, min(255, c)) for c in pixel]
            
            pixels.append(tuple(pixel))
    
    save_ppm(64, 64, pixels, filename)

def generate_all_sprites():
    """Generate all block textures"""
    print("🎨 Generating Minecraft C++ Sprites...")
    
    create_texture_directory()
    
    # Basic blocks
    create_grass_texture()
    create_dirt_texture()
    create_stone_texture()
    create_wood_texture()
    create_leaves_texture()
    create_water_texture()
    create_sand_texture()
    
    # Ore blocks
    create_ore_texture("assets/textures/coal_ore.ppm", (40, 40, 40))      # Dark coal
    create_ore_texture("assets/textures/iron_ore.ppm", (184, 115, 51))    # Brown iron
    create_ore_texture("assets/textures/diamond_ore.ppm", (185, 242, 255)) # Light blue diamond
    
    # Create air texture (transparent-looking)
    pixels = [(200, 200, 255) for _ in range(64*64)]  # Light blue
    save_ppm(64, 64, pixels, "assets/textures/air.ppm")
    
    print(f"\n✅ Generated all sprites in assets/textures/")
    print(f"📁 Each texture is 64x64 pixels in PPM format")
    print(f"🎮 Run 'make && ./minecraft' and press 'Y' for textured mode!")
    print(f"🔍 Debug atlas will be saved as 'texture_atlas_debug.ppm'")

if __name__ == "__main__":
    generate_all_sprites()