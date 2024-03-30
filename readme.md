## Welcome
This is my first c++ project. I've started this to learn c++ and improve my
OpenGL skills.

Feel free to comment on what you don't like or like, and I'll try to improve.

Currently, been working on it in and out for ~4 months when I find free time/

## The Game
The game is basically a clone of Minecraft classic but with infinite worlds and
shadow mapping. 

![screenshot.png](screenshot.png)

There are still a few features I want to add like swimming, bug/crash fixes, UI,
an inventory system, day/night cycle and the sun.

## Current Features

- Infinite world generation
- Threaded chunk generation
- Placing blocks
- Breaking blocks
- Transparency sorting
- Water Animation using texture change
- Collision system
- "Physics"
- Sneaking using shift
- F11 for fullscreen (can't get out of fullscreen for now)
- Shadows using shadow maps from the sun
- Tree generation

## Known Issues

- Sometimes the tree leaves don't get added until chunk is updated
- Rare crash due to threading (possibly a race condition)
- Chunks not displaying their mesh when updated too many times (just like in the very early versions of Minecraft)
- Shadows on block sides sometimes do look out of place, need to use normals for bias calculations
- Rare collision issues if framerate is low
- Sneaking on chunk edges may not work properly all the time

## Features to implement

- Normal calculation for block faces to make shadow maps look better
- Water shader
- UI for block selection
- Inventory system (basically a creative menu)
- Flying
- Saving and loading Worlds
- More varied terrain generation
- Flowers
- Ores
- Caves
- Ambient occlusion
- Deferred rendering
- Swimming
- In-water post-processing effect
- Sun/moon
- Day/night cycle
- Clouds
- Slabs

Feel free to leave more suggestions (please be reasonable)

## Building

## Linux/Unix

git clone --recurse-submodules https://github.com/Vodkasas/CraftMine.git

Using CMake:

Open terminal and cd to the CraftMine folder, create a folder called "build" and cd there
Now type in:

    cmake ..
    make

Now your executable should be in the build folder

## Windows

I would recommend building using MinGW as it has the best performance for the game.
Easiest way to build is if you have CLion which should do it automatically for you.

Issue with Visual Studio is that it uses the MSVC compiler by default which makes the threads
slower compared to MinGW.


