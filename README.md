# OpenGL Project Dead Space
This is my OpenGL project called dead space. It is for my 3015 submission under the University of Plymouth.

I should also mention that this project was developed from a template provided by the university. This template served as a framework providing tools such as mesh and texture loading, scene management etc. However, all work displayed in the scene was achieved soley by myself.

# System Info

Visual studio 2022

Windows 10

OpenGL: 4.6

GLSL: 4.60

GPU: RX 6800 (AMD)

# Controls

WASD to move, Q and E to rotate left and right.

# Objectives

In this small OpenGL game, the objective is to navigate a small asteroid field. Collision with an asteroid reduces the ship health. When the ship health reaches 0, you lose the game.

# Code Break Down

### Scene Basic Uniform

This script handles the initialization of scenes. it sets up the rendering pipeline, loads meshes and textures, initializes the collision system, and creates the asteroid field.

The game state, ship movement, lighting effects and collision detection is handled in update.

I also implement a two pass rendering system. The first pass renders the skybox, ship, astroids to a HDR frame buffer while the second pass applies post processing effects like tone mapping and damage effects.

### Ship Controller Class

This class does the following:

- Stores the ship's position and direction vectors.
- Processes keyboard input in handleInput() to move the ship.
- Provides getPosition() and getDirection() methods used by the camera system and collision detection.


### Asteroid Mananger Class

This class stores asteroid positions, rotations, scales and rendering properties. It also creates and populates the asteroid field in generateAsteroids(). Asteroid rotations are handled in update() and are rendered efficiently in render(). Collision information is also provided through getAsteroid() and getBaseMeshBoundingBox().

### Collision Detection Class

This class does the following:

- In initialize(), references are set up to the ship and asteroid manager.
- The update() method handles collision cooldown timing.
- checkShipAsteroidCollisions() detects collisions between the ship and asteroids.
- Uses a callback system to notify the main application when collisions occur.

### Collision Detection System

1. The ship is approximated as a sphere with a radius calculated from the model bounds.
2. Each asteroid uses an axis aligned bounding box (AABB) for collision testing.
3. For each asteroid, the system:
   - Transforms the asteroid's AABB to world space
   - finds the cloest point on the AABB to the ship's position
   - Checks if the distance to this point is less than the ship's collision radius.
4. A scaling factor of 0.4f reduces the effective collision disance for gameplay tuning.
5. A cooldown timer prevents multiple collisions in rapid succession.

# Render Pipeline and Shaders

### Ship Shader (Basic uniform.vert/frag)

1. Vertex Shader:
   - Transforms vertex positions from model space to world space.
   - Calculates the TBN matrix for normal mapping.
   - Passes texture coordinates to the fragment shader.

2. Fragment Shader:
   
   A. Implements a full PBR lighting model with:
      - Metallic workflow, using albedo, metallic, roughness and AO maps.
      - Normal mapping for surface detail.
      - Environment mapping for reflections.
      - Chromatic abberation effect.
      - Microfacet BRDF (Bidrectional Refflectance Distribution Function).
        
   B. Key PBR Components:
      - GGX/Trowbridge-Reitz for normal distribution function.
      - Schlick-GGX for geometry function.
      - Fresnel-Schlick approximation.

   C. Applies tone mapping and gamma correction.

### Asteroid Shader (Vert and Frag)

1. Vertex Shader:
   - Similar to the spaceship shader but uses a normal matrix for proper normal transformation.
   - Calculates TBN matrix for normal mapping.

2. Fragment Shader:
   - Uses spherical UV mapping to wrap textures around the asteroid.
   - Implements a simplified lighting model consisting of basic ambient light, diffuse lighting with attentuation and normal mapping.

3. Applies distance based attenuation for light falloff.
4. Performs gamma correction

### Skybox Shader (Vert and Frag).

1. Vertex Shader:
   - Removes translation from the view matrix to make the skybox follow the camera.
   - Sets position depth equal to W component to ensure skybox is always at the maximum depth.

2. Fragment Shader:
   - Simply samples the cubemap texture at the direction provided by the vertex shader.
   - Outputs the texture colour without any aditional processing.

### HDR Post Processing Shader:

1. Vertex Shader:
   - Passes screen coordinates for a full screen quad.

2. Fragment Shader:
   - Cold blue colour tint.
   - Vignette darkening at screen edges.
   - Fil grain/noise.
   - Colour distortion in darker areas.
   - Red flash when the ship takes damage.
   - Applies gamma correction.

# Game Flow

1. Initialization:
   - Loads 3D models, textures and compile shaders.
   - Set up HDR frame buffer for post processing.
   - Initialize asteroid field with 500 asteroids.
   - Configure collision system with appropriate ship radius.
     
2. Game Loop:
   - Process user input for ship movement.
   - Set up HDR framebuffer for post processing.
   - Initialize asteroid field with 500 asteroids.
   - Configure collision detection system with the appropriate ship radius.

4.Game Loop:
- When a collision is detected, player health is reduced.
- Visual effects are triggered (red flash).
- if health reaches zero, a game over screen is displayed in console.

# Pictures

![image](https://github.com/user-attachments/assets/4fcc02db-b40e-475a-ad13-ad9424ab1e80)

![image](https://github.com/user-attachments/assets/8cd5d07d-be39-4393-9348-cf6bc81d6a68)

![image](https://github.com/user-attachments/assets/bb811868-dde7-4283-9476-6c87e27e0794)

![image](https://github.com/user-attachments/assets/c7660a0f-eb34-4e10-886f-814113e92aa0)

# Inspiration

My aim was to capture the lost in space vibe you get from old scifi movies such as Alien (1979) / Aliens (1986).

# Reflection

My only gripe with this project is the spaceship model. It is the most unoptimised model I have ever worked with and it heavily contributes to the start up time of the project.

Furthermore, there is currently a strange bug that can't be replicated where collisions do not seem to work accurately when this project is ran on other machines. This will be investigated in the near future.

# Youtube Link:



