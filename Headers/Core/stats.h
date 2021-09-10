#ifndef STATS_H
#define STATS_H

#include "_config.h"

struct Stats
{
    Stats(db shotDelay = 0.0, ft shotDelayMultiplayar = 1.0f, ft shotDamage = 0.0f, ft shotDanageMultiplayer = 1.0f, ft shotSpeed = 0.0f,
          ui noShots = 0, ft shotVariance = 0.0f, ft shotHomingStrength = 0.0f, si maxHP = 0, si currHP = 0, db enemySpawnRate = 0.0)
        : shotDelay(shotDelay), shotDelayMultiplayar(shotDelayMultiplayar), shotDamage(shotDamage),
          shotDanageMultiplayer(shotDanageMultiplayer), shotSpeed(shotSpeed), noShots(noShots), shotVariance(shotVariance),
          shotHomingStrength(shotHomingStrength), maxHP(maxHP), currHP(currHP), enemySpawnRate(enemySpawnRate),
          actualShotDelay(shotDelay * shotDelayMultiplayar), actualDamage(shotDamage * shotDanageMultiplayer) {}

    db shotDelay;
    ft shotDelayMultiplayar;
    ft shotDamage;
    ft shotDanageMultiplayer;
    ft shotSpeed;
    ui noShots;
    ft shotVariance;
    ft shotHomingStrength;
    si maxHP;
    si currHP;

    db enemySpawnRate;

    db actualShotDelay;
    ft actualDamage;

    void operator+=(const Stats &lval)
    {
        shotDelay += lval.shotDelay;
        shotDelayMultiplayar *= lval.shotDelayMultiplayar;
        shotDamage += lval.shotDamage;
        shotDanageMultiplayer *= lval.shotDanageMultiplayer;
        shotSpeed += lval.shotSpeed;
        noShots += lval.noShots;
        shotVariance += lval.shotVariance;
        shotHomingStrength += lval.shotHomingStrength;
        maxHP += lval.maxHP;
        currHP += lval.currHP;
        enemySpawnRate += lval.enemySpawnRate;
        if (currHP > maxHP)
            currHP = maxHP;
        Recalculate();
    }
    Stats operator+(const Stats &lval) const
    {
        return Stats(
            shotDelay + lval.shotDelay,
            shotDelayMultiplayar * lval.shotDelayMultiplayar,
            shotDamage + lval.shotDamage,
            shotDanageMultiplayer * lval.shotDanageMultiplayer,
            shotSpeed + lval.shotSpeed,
            noShots + lval.noShots,
            shotVariance + lval.shotVariance,
            shotHomingStrength + lval.shotHomingStrength,
            maxHP + lval.maxHP,
            currHP + lval.currHP,
            enemySpawnRate  + lval.enemySpawnRate
            );
    }
    void Recalculate()
    {
        actualDamage = shotDamage * shotDanageMultiplayer;
        actualShotDelay = shotDelay * static_cast<db>(shotDelayMultiplayar);
    }
};

namespace stat_altarations
{
    inline Stats SHOT_DELAY(db arg)              { return Stats(arg); }
    inline Stats SHOT_DELAY_MULTIPLAYER(ft arg)  { return Stats(0.0, arg); }
    inline Stats SHOT_DAMADE(ft arg)             { return Stats(0.0, 1.0f, arg); }
    inline Stats SHOT_DAMADE_MULTIPLAYER(ft arg) { return Stats(0.0, 1.0f, 0.0f, arg); }
    inline Stats SHOT_SPEED(ft arg)              { return Stats(0.0, 1.0f, 0.0f, 1.0f, arg); }
    inline Stats NO_SHOTS(ui arg)                { return Stats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, arg); }
    inline Stats SHOT_VARIANCE(ft arg)           { return Stats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, arg); }
    inline Stats SHOT_HOMING_STRENGTH(ft arg)    { return Stats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0.0f, arg); }
    inline Stats MAX_HP(si arg)                  { return Stats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0.0f, 0.0f, arg); }
    inline Stats CURR_HP(si arg)                 { return Stats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0.0f, 0.0f, 0, arg); }
}

static const Stats defaultStats(
    150.0, // shotDelay
    1.0f,  // shotDelayMultiplayar
    10.0f, // shotDamage
    1.0f,  // shotDanageMultiplayer
    0.02f, // shotSpeed
    1,     // noShots
    0.0f,  // shotVariance
    5.0f,  // shotHomingStrength
    3,     // maxHP
    3,     // currHP
    200.0  // enemySpawnRate
);

#endif