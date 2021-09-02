#ifndef STATS_H
#define STATS_H

#include "_config.h"

struct Stats
{
    db shotDelay = 0.0;
    ft shotDelayMultiplayar = 0.0f;
    ft shotDamage = 0.0f;
    ft shotDanageMultiplayer = 0.0f;
    ft shotSpeed = 0.0f;
    ui noShots = 0;
    ft shotVariance = 0.0f;
    ft shotHomingStrength = 0.0f;
    si maxHP = 0;
    si currHP = 0;

    db enemySpawnRate;

    Stats operator+(const Stats& lval) const;
    void operator+=(const Stats& lval);
    inline ft Damage(){ return shotDamage * shotDanageMultiplayer; }
    inline db Delay() { return shotDelay * static_cast<db>(shotDelayMultiplayar); }
};

static const Stats defaultStats = 
{
    150.0,     // shotDelay  
    1.0f,      // shotDelayMultiplayar
    20.0f,     // shotDamage
    1.0f,      // shotDanageMultiplayer
    0.02f,     // shotSpeed
    1,         // noShots
    0.0f,      // shotVariance
    0.0f,      // shotHomingStrength
    3,         // maxHP
    3,         // currHP
    200.0      // enemySpawnRate
};

#endif