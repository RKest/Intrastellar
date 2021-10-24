#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <array>
#include "glm/gtx/string_cast.hpp"      
#include "glm/gtx/vector_angle.hpp"

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

[[maybe_unused]]constexpr std::array UNIFORMS = {"transform", "projection"};

constexpr const ui CUSTOM_RAND_SEED = 982347557;
constexpr const ui SCREEN_WIDTH = 960;
constexpr const ui SCREEN_HEIGHT = 540;
constexpr const ft SCREEN_ASPECT = static_cast<ft>(SCREEN_WIDTH) / static_cast<ft>(SCREEN_HEIGHT);

//Shooter 
constexpr const ui MAX_PROJ_AMOUNT = 100;
constexpr const ui MAX_NO_ENEMIES = 1;
constexpr const ui MAX_NO_SHOOTER_ENEMIES = 1;
constexpr const ui MAX_NO_ORBITER_ENEMIES = 1;

constexpr const ui MAX_ENEMY_PROJ_AMOUNT = 300;
constexpr const ui MAX_PROJ_AMOUNT_PER_ENEMY = 10;
constexpr const ui MAX_PROJ_AMOUNT_PER_ORBIT = 20;

constexpr const ui CARD_MAX_PROJ_COUNT = 300;
constexpr const ui NO_CARDS = 3;
constexpr const ui MAX_EXP_PART_NO = 200;

constexpr const ft TAU = 2.0f * glm::pi<ft>();
constexpr const db TAU_d = 2.0 * glm::pi<db>();
constexpr const ft PI = glm::pi<ft>();
constexpr const db PI_d = glm::pi<db>();
constexpr const ft MAX_PROJ_TURNING_RAD = glm::radians(1.0f);
constexpr const ft DEF_ANGLE_BETWEEN_SHOTS = glm::radians(15.0f);

inline db ENEMY_ORPHANDED_PROJ_LIFETIME = 5000.0;
constexpr const ft ENEMY_ORBIT_RADIUS = 5.0f;
constexpr const ft ENEMY_ORBIT_CIRC = ENEMY_ORBIT_RADIUS * TAU;
constexpr const ft ENEMY_ORBIT_TICS_TO_DESIERED_ANGLE = 1000.0F;
constexpr const glm::mat4 ENEMY_ORBIT_TO_ORBIT_TRANSLATE{{0.0f,1.0f,0.0f,0.0f}, {-1.0f,0.0f,0.0f,0.0f},{0.0f, 0.0f, 1.0f, 0.0f},{0.0f, ENEMY_ORBIT_RADIUS + 0.1f, 0.0f, 1.0f}};

//Same in the file:///home/max/Documents/Intrastellar/Shaders/WeaponIcon/VS.glsl
constexpr const ui WEAPONS_NO_WEAPONS = 10;
constexpr const ft WEAPONS_ICON_DIMS = static_cast<ft>(SCREEN_HEIGHT) / static_cast<ft>(WEAPONS_NO_WEAPONS);

inline size_t _dummy_size_t;
inline bool _dummy_bool;
inline si _dummy_si;
inline const glm::mat4 _blankTransform = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
template<typename T, typename ...Ts>
void _log(T first, Ts ...rest)
{
    std::cout << first << ", ";
    if constexpr (sizeof...(rest) > 0)
        _log(rest...);
    else 
        std::cout << '\n';
}

#if __cplusplus > 201703L
#include <experimental/source_location>
#define LOG(...) log(std::experimental::fundamentals_v2::source_location::current(), __VA_ARGS__)
template<typename ...T>
void log(std::experimental::fundamentals_v2::source_location loc, T ...args)
{
    std::cout << "[" << loc.file_name() << ":" << loc.line() << ":" << loc.column() << "] "
              << "(" << loc.function_name() << ") ";
    _log(args...);
}
#else
#define LOG(...) log(__VA_ARGS__)
template<typename ...T>
void log(T ...args)
{
    _log(args...);
}

#endif

template <typename T, typename = void>
struct is_std_container : std::false_type { };

template <typename T>
struct is_std_container<T,
    std::void_t<decltype(std::declval<T&>().begin()),
                decltype(std::declval<T&>().end()),
                typename T::value_type>>
    : std::true_type { };

template <typename T>
concept container_type = is_std_container<T>::value;
template <typename T>
concept non_container_type = !is_std_container<T>::value;

template <container_type T, typename U>
constexpr T::value_type decl_cast([[maybe_unused]]const T &to, const U &from) 
{
   return static_cast<typename T::value_type>(from);
}

template <non_container_type T, typename U>
constexpr T decl_cast([[maybe_unused]]const T &to, const U &from) 
{
   return static_cast<T>(from);
}

#define DECL_INST(name, val) decltype(val) name = val