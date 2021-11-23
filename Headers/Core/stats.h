#pragma once
#include "_config.h"

struct PlayerStats
{
    PlayerStats(db shotDelay = 0.0, ft shotDelayMultiplayar = 1.0f, ft shotDamage = 0.0f, ft shotDanageMultiplayer = 1.0f, ft shotSpeed = 0.0f,
          ui noShots = 0, ui noPiercings = 0, ft shotVariance = 0.0f, ft shotHomingStrength = 0.0f, si maxHP = 0, si currHP = 0, db enemySpawnRate = 0.0)
        : shotDelay(shotDelay), shotDelayMultiplayar(shotDelayMultiplayar), shotDamage(shotDamage),
          shotDanageMultiplayer(shotDanageMultiplayer), shotSpeed(shotSpeed), noShots(noShots), noPiercings(noPiercings), shotVariance(shotVariance),
          shotHomingStrength(shotHomingStrength), maxHP(maxHP), currHP(currHP), enemySpawnRate(enemySpawnRate),
          actualShotDelay(shotDelay * shotDelayMultiplayar), actualDamage(shotDamage * shotDanageMultiplayer) {}

    db shotDelay;
    ft shotDelayMultiplayar;
    ft shotDamage;
    ft shotDanageMultiplayer;
    ft shotSpeed;
    ui noShots;
    ui noPiercings;
    ft shotVariance;
    ft shotHomingStrength;
    si maxHP;
    si currHP;

    db enemySpawnRate;

    db actualShotDelay;
    ft actualDamage;

    void operator+=(const PlayerStats &lval)
    {
        shotDelay += lval.shotDelay;
        shotDelayMultiplayar *= lval.shotDelayMultiplayar;
        shotDamage += lval.shotDamage;
        shotDanageMultiplayer *= lval.shotDanageMultiplayer;
        shotSpeed += lval.shotSpeed;
        noShots += lval.noShots;
        noPiercings += lval.noPiercings;
        shotVariance += lval.shotVariance;
        shotHomingStrength += lval.shotHomingStrength;
        maxHP += lval.maxHP;
        currHP += lval.currHP;
        enemySpawnRate += lval.enemySpawnRate;
        if (currHP > maxHP)
            currHP = maxHP;
        Recalculate();
    }
    void operator-=(const PlayerStats &lval)
    {
        shotDelay -= lval.shotDelay;
        shotDelayMultiplayar /= lval.shotDelayMultiplayar;
        shotDamage -= lval.shotDamage;
        shotDanageMultiplayer /= lval.shotDanageMultiplayer;
        shotSpeed -= lval.shotSpeed;
        noShots -= lval.noShots;
        noPiercings -= lval.noPiercings;
        shotVariance -= lval.shotVariance;
        shotHomingStrength -= lval.shotHomingStrength;
        maxHP -= lval.maxHP;
        currHP -= lval.currHP;
        enemySpawnRate -= lval.enemySpawnRate;
        if (currHP > maxHP)
            currHP = maxHP;
        Recalculate();
    }
    PlayerStats operator+(const PlayerStats &lval) const
    {
        return PlayerStats(
            shotDelay + lval.shotDelay,
            shotDelayMultiplayar * lval.shotDelayMultiplayar,
            shotDamage + lval.shotDamage,
            shotDanageMultiplayer * lval.shotDanageMultiplayer,
            shotSpeed + lval.shotSpeed,
            noShots + lval.noShots,
            noPiercings + lval.noPiercings,
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
    inline PlayerStats SHOT_DELAY(db arg)              { return PlayerStats(arg); }
    inline PlayerStats SHOT_DELAY_MULTIPLAYER(ft arg)  { return PlayerStats(0.0, arg); }
    inline PlayerStats SHOT_DAMADE(ft arg)             { return PlayerStats(0.0, 1.0f, arg); }
    inline PlayerStats SHOT_DAMADE_MULTIPLAYER(ft arg) { return PlayerStats(0.0, 1.0f, 0.0f, arg); }
    inline PlayerStats SHOT_SPEED(ft arg)              { return PlayerStats(0.0, 1.0f, 0.0f, 1.0f, arg); }
    inline PlayerStats NO_SHOTS(ui arg)                { return PlayerStats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, arg); }
    inline PlayerStats NO_PIERCINGS(ui arg)            { return PlayerStats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, arg); }
    inline PlayerStats SHOT_VARIANCE(ft arg)           { return PlayerStats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0, arg); }
    inline PlayerStats SHOT_HOMING_STRENGTH(ft arg)    { return PlayerStats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0, 0.0f, arg); }
    inline PlayerStats MAX_HP(si arg)                  { return PlayerStats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0, 0.0f, 0.0f, arg); }
    inline PlayerStats CURR_HP(si arg)                 { return PlayerStats(0.0, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0, 0.0f, 0.0f, 0, arg); }
}

static const PlayerStats defaultStats(
    150.0, // shotDelay
    1.0f,  // shotDelayMultiplayar
    20.0f, // shotDamage
    1.0f,  // shotDanageMultiplayer
    0.02f, // shotSpeed
    1,     // noShots
    1,     // noPiercings
    0.0f,  // shotVariance
    5.0f,  // shotHomingStrength
    3,     // maxHP
    3,     // currHP
    200.0  // enemySpawnRate
);

struct EnemyStats 
{
    db speed;
    ft shotSpeed;
    db shotDelay;
    ft shotRange;
    ui health;
};

inline PlayerStats g_playerStats = defaultStats;

static const EnemyStats defaultEnemyStats{
    0.01,   //speed
    0.01f,  //shotSpeed
    300.0,  //shotDelay
    0.0f,   //shotRange
    100     //health
};