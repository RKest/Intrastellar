#ifndef _CONFIG_H
#define _CONFIG_H

#include <cstddef>
#include <iostream>
#include <string>
#include <array>
#include "glm/gtx/string_cast.hpp"

typedef unsigned long long u64; 
typedef unsigned int ui;
typedef int si;
typedef unsigned char uc;
typedef float ft;
typedef double db;

template <typename T, size_t N>
constexpr size_t ARR_SIZE(T (&)[N]) {
    return N;
}

#ifndef MESH_PARAMS_FROM_PATH
#define MESH_PARAMS_FROM_PATH(path, params)\
	const auto model = OBJModel(path).ToIndexedModel(); \
	const UntexturedMeshParams params = {model.positions.data(), model.indices.data(), static_cast<ui>(model.positions.size()), static_cast<ui>(model.indices.size())};
#endif

constexpr ui CUSTOM_RAND_SEED = 982347557;
constexpr ui SCREEN_WIDTH = 960;
constexpr ui SCREEN_HEIGHT = 540;
constexpr ft SCREEN_ASPECT = static_cast<ft>(SCREEN_WIDTH) / static_cast<ft>(SCREEN_HEIGHT);

constexpr std::array UNIFORMS = {"transform", "projection"};

//Shooter 
#define MAX_NO_SHOOTER_PROJECTILES 10
#define MAX_NO_ENEMIES 10


inline size_t _dummy_size_t;
inline bool _dummy_bool;
inline const glm::mat4 _blankTransform = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);

#endif