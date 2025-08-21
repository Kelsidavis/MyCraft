#!/usr/bin/env python3
"""
Minecraft C++ Sprite Generator
Creates 64x64 PPM textures for all block types
"""

from PIL import Image, ImageDraw
import os
import random

def create_texture_directory():
    """Create the textures directory if it doesn't exist"""
    os.makedirs("assets/textures", exist_ok=True)

def save_ppm(image, filename):
    """Save image as PPM format"""
    # Convert to RGB if needed
    if image.mode != 'RGB':
        image = image.convert('RGB')
    
    with open(filename, 'wb') as f:
        f.write(f"P6\n{image.width} {image.height}\n255\n".encode())
        for pixel in image.getdata():
            f.write(bytes(pixel))
    print(f"Created: {filename}")

def create_grass_texture():
    """Create grass block texture"""
    img = Image.new('RGB', (64, 64), (34, 139, 34))  # Forest green base
    draw = ImageDraw.Draw(img)
    
    # Add grass blade details
    for _ in range(200):
        x = random.randint(0, 63)
        y = random.randint(0, 63)
        # Vary green shades
        green = random.randint(20, 180)
        color = (random.randint(0, 30), green, random.randint(0, 50))
        draw.point((x, y), fill=color)
    
    save_ppm(img, "assets/textures/grass.ppm")

def create_dirt_texture():
    """Create dirt block texture"""
    img = Image.new('RGB', (64, 64), (139, 69, 19))  # Saddle brown base
    draw = ImageDraw.Draw(img)
    
    # Add dirt variations
    for _ in range(300):
        x = random.randint(0, 63)
        y = random.randint(0, 63)
        brown = random.randint(40, 160)
        color = (brown, int(brown * 0.6), int(brown * 0.2))
        draw.point((x, y), fill=color)
    
    save_ppm(img, "assets/textures/dirt.ppm")

def create_stone_texture():
    """Create stone block texture"""
    img = Image.new('RGB', (64, 64), (128, 128, 128))  # Gray base
    draw = ImageDraw.Draw(img)
    
    # Add stone variations and cracks
    for _ in range(400):
        x = random.randint(0, 63)
        y = random.randint(0, 63)
        gray = random.randint(60, 200)
        color = (gray, gray, gray)
        draw.point((x, y), fill=color)
    
    # Add some darker cracks
    for _ in range(50):
        x1, y1 = random.randint(0, 63), random.randint(0, 63)
        x2, y2 = x1 + random.randint(-5, 5), y1 + random.randint(-5, 5)
        draw.line([(x1, y1), (x2, y2)], fill=(40, 40, 40), width=1)
    
    save_ppm(img, "assets/textures/stone.ppm")

def create_wood_texture():
    """Create wood block texture"""
    img = Image.new('RGB', (64, 64), (160, 82, 45))  # Saddle brown
    draw = ImageDraw.Draw(img)
    
    # Add wood grain (vertical lines)
    for x in range(64):
        for y in range(64):
            # Create wood grain pattern
            brown_base = 100 + (y % 40) + random.randint(-20, 20)
            brown_base = max(50, min(200, brown_base))
            color = (brown_base, int(brown_base * 0.6), int(brown_base * 0.2))
            draw.point((x, y), fill=color)
    
    # Add wood rings
    for ring in range(3):
        center_y = 20 + ring * 15
        for y in range(max(0, center_y-2), min(64, center_y+3)):
            for x in range(64):
                color = (80, 40, 20)
                draw.point((x, y), fill=color)
    
    save_ppm(img, "assets/textures/wood.ppm")

def create_leaves_texture():
    """Create leaves block texture"""
    img = Image.new('RGB', (64, 64), (34, 100, 34))  # Dark green base
    draw = ImageDraw.Draw(img)
    
    # Add leaf variations
    for _ in range(500):
        x = random.randint(0, 63)
        y = random.randint(0, 63)
        green = random.randint(20, 150)
        color = (random.randint(0, 40), green, random.randint(0, 60))
        draw.point((x, y), fill=color)
    
    save_ppm(img, "assets/textures/leaves.ppm")

def create_water_texture():
    """Create water block texture"""
    img = Image.new('RGB', (64, 64), (64, 164, 223))  # Blue base
    draw = ImageDraw.Draw(img)
    
    # Add water ripples/waves
    for y in range(64):
        for x in range(64):
            wave = int(20 * (0.5 + 0.5 * (x * 0.1 + y * 0.1)))
            blue = 150 + wave + random.randint(-30, 30)
            blue = max(100, min(255, blue))
            color = (random.randint(20, 80), random.randint(100, 160), blue)
            draw.point((x, y), fill=color)
    
    save_ppm(img, "assets/textures/water.ppm")

def create_sand_texture():
    """Create sand block texture"""
    img = Image.new('RGB', (64, 64), (238, 203, 173))  # Sandy brown
    draw = ImageDraw.Draw(img)
    
    # Add sand grain variations
    for _ in range(600):
        x = random.randint(0, 63)
        y = random.randint(0, 63)
        brightness = random.randint(150, 255)
        color = (brightness, int(brightness * 0.85), int(brightness * 0.65))
        draw.point((x, y), fill=color)
    
    save_ppm(img, "assets/textures/sand.ppm")

def create_ore_texture(base_name, ore_color, ore_spots=30):
    """Create ore block texture (stone base with ore spots)"""
    # Start with stone texture
    img = Image.new('RGB', (64, 64), (128, 128, 128))
    draw = ImageDraw.Draw(img)
    
    # Add stone base
    for _ in range(300):
        x = random.randint(0, 63)
        y = random.randint(0, 63)
        gray = random.randint(80, 160)
        color = (gray, gray, gray)
        draw.point((x, y), fill=color)
    
    # Add ore spots
    for _ in range(ore_spots):
        x = random.randint(1, 62)
        y = random.randint(1, 62)
        
        # Draw ore spot (3x3 area)
        for dx in range(-1, 2):
            for dy in range(-1, 2):
                if 0 <= x+dx < 64 and 0 <= y+dy < 64:
                    # Add some randomness to ore color
                    r = max(0, min(255, ore_color[0] + random.randint(-30, 30)))
                    g = max(0, min(255, ore_color[1] + random.randint(-30, 30)))
                    b = max(0, min(255, ore_color[2] + random.randint(-30, 30)))
                    draw.point((x+dx, y+dy), fill=(r, g, b))
    
    save_ppm(img, f"assets/textures/{base_name}.ppm")

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
    create_ore_texture("coal_ore", (40, 40, 40), 25)      # Dark gray/black coal
    create_ore_texture("iron_ore", (184, 115, 51), 20)    # Brown iron spots
    create_ore_texture("diamond_ore", (185, 242, 255), 15) # Light blue diamonds
    
    print(f"\n✅ Generated all sprites in assets/textures/")
    print(f"📁 Atlas will be saved as 'texture_atlas_debug.ppm' when you run the game")
    print(f"🎮 Run './minecraft' and press 'Y' for textured mode!")

if __name__ == "__main__":
    generate_all_sprites()