#include "Physics.h"

TempAllocatorImpl* Physics::tempMemAllocator = nullptr;

JobSystemThreadPool* Physics::threadPool = nullptr;

BPLayerInterfaceImpl* Physics::broad_phase_layer_interface = nullptr;

ObjectVsBroadPhaseLayerFilterImpl* Physics::object_vs_broadphase_layer_filter = nullptr;

ObjectLayerPairFilterImpl* Physics::object_vs_object_layer_filter = nullptr;

PhysicsSystem* Physics::physics_system = nullptr;

MyContactListener* Physics::contact_listener = nullptr;