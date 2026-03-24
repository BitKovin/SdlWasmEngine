This repository contains code of REngine 2. It's evolution of my previous RetroEngine that was written in C#.

# [Engine Systems Reference](EngineSystemsReference.md)

Core improvements of this version:

1. Jolt Physics as main physics engine (Instead of outdated Bullet Physics)
2. Full Quake 3 BSP map format support (including lightmaps, light volumes, precomputed visibility and additional rendering optimizations)
3. Behavior Tree editor
4. Emscripten compilation support with experimental multi-threading
5. Improved animation blending and playback
6. BGFX renderer with support of all backends.
7. Mobile control

This repository also contains modified versions of used libraries and tools.
Most important modifications:
1. Fixed import of gltf2 and glb2 formats by disabling gltf1 and glb1 formats in Assimp. Platforms like Linux and Emscipten used wrong importer.
2. Disabled remove of coplanar faces on dynamic entities (any entity that isn't worldspawn) in q3map2.
3. Increated max lightvol amount in q3map2.

Screenshots:

test.map opened in net radiant custom
<img width="1914" height="1029" alt="image" src="https://github.com/user-attachments/assets/1d834dcb-d07e-4a40-8f0e-c3ed66018f33" />

test.map opened in game
<img width="1914" height="1011" alt="image" src="https://github.com/user-attachments/assets/e400a2b6-50e5-4a12-87e2-d36fcddc2992" />


Behavior Tree Editor:
<img width="1910" height="1005" alt="image" src="https://github.com/user-attachments/assets/0e83cd65-e0a4-4bc6-a657-9fd6c80b9ebb" />

120 NPCs on screen in active combat with player. Including even spreading around player, search for cover, line of sight checks and animation blending.
<img width="1913" height="991" alt="image" src="https://github.com/user-attachments/assets/45ce7579-1081-4298-acef-bcf098065f45" />

Multiple NPCs affected by active ragdoll stun and recovery from ragdoll
<img width="1745" height="962" alt="image" src="https://github.com/user-attachments/assets/ea21fcb6-b8ea-4e3a-bfc1-e72d0e4c6cfd" />
