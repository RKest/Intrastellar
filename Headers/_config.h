#ifndef _CONFIG_H
#define _CONFIG_H

#include <cstddef>
#include <iostream>
#include "glm/gtx/string_cast.hpp"

#ifndef ARR_SIZE
#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#define CUSTOM_RAND_SEED 982347557


#define SCREEN_WIDTH 960 
#define SCREEN_HEIGHT 540

//Shader Params
#define PC_PARAMS {"position"}
#define PC_PARAMS_NO 1

#define PROJECTILE_PARAMS {"position", "instanceTransform"}
#define PROJECTILE_PARAMS_NO 2

#define TEXT_PARAMS {"vertex"}
#define TEXT_PARAMS_NO 1

#define UNIFORMS {"transform", "projection"}
#define UNIFORMS_NO 2

//Shooter 
#define MAX_NO_SHOOTER_PROJECTILES 10
#define MAX_NO_ENEMIES 10

//Timer frequences (kHz)
#define SHOOTER_FREQUENCY 150.0
#define ENEMY_SPAWN_FREQUENCY 400.0

typedef unsigned long long u64; 
typedef unsigned int ui;
typedef unsigned char uc;
typedef float ft;
typedef double db;

static size_t _dummy_size_t;
static bool _dummy_bool;

#endif