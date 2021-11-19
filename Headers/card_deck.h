#pragma once
#include "_config.h"

#include "Core/text.h"
#include "Core/transform.h"
#include "Core/shader.h"
#include "Core/mesh.h"
#include "Core/camera.h"
#include "Core/stats.h"
#include "Core/timer.h"
#include "Core/helpers.h"
#include "Core/bounding_box.h"

#include <string>
#include <execution>
#include <algorithm>
#include <functional>

struct Card
{
    Card(const std::string &cardText, const ui weight, const std::vector<PlayerStats> &statAltarationList);
    const std::string cardText;
    const ui weight;
    const PlayerStats statAltarations;
};

using pcDrawFunc = std::function<void(const std::vector<glm::mat4>&, std::vector<glm::mat4>&, const std::vector<ui>&, 
    const std::vector<ReqBoundingBox>&, const glm::mat4&, ui&)>;
class CardDeck 
{
public:
    CardDeck(helpers::Core &core, const UntexturedMeshParams &overlayParams, const UntexturedMeshParams &cardBorderParams);
    void DrawCards();
    void RollCards();
    inline bool &AreCarsDrawn() { return _areCardsDrawn; }

protected:
private:
    PlayerStats  &_stats;
    CustomRand _customRand;
    Shader _cardBorderShader{"Shaders/CardBorder"};
    Shader _overlayShader{"Shaders/Overlay"};
    UntexturedMesh          _overlayMesh;
    UntexturedMeshParams    _cardBorderParams;
    UntexturedInstancedMesh _cardBorderMesh;
    Transform _cardBorderTransform;

    std::vector<glm::mat4> _cardBorderInstanceTransforms;
    std::vector<ReqBoundingBox> _cardBoundingBoxes = std::vector<ReqBoundingBox>(3);

    glm::mat4 _cardProjection                       = glm::ortho(-SCREEN_ASPECT, SCREEN_ASPECT, -1.0f, 1.0f);
    glm::mat4 _inverseFlippedCardBorderProjection   = glm::inverse(glm::ortho(-SCREEN_ASPECT, SCREEN_ASPECT, -1.0f, 1.0f));

    bool _areCardsDrawn = false;
    bool _areCardsFullyDrawn = false;
    bool _isLBMPressed = false;

    ftUni _overlayAlphaUni{"overlayAlpha", 0.0f};
    Clock<> m_overlayClock;

    void _choseCards(const ui cardIndex);
    std::vector<ui> _chosenCardIndices = std::vector<ui>(3, 9999);
    std::vector<db> _cardShotDelays = std::vector<db>(3);
    ui _cardWeightSum = 0;
    const std::array<Card, 9> _cards\
    {{
        Card("Decrease Shot Delay",                        5, std::vector<PlayerStats>{ stat_altarations::SHOT_DELAY(-50.0)                 }),
        Card("Increase Damage",                            5, std::vector<PlayerStats>{ stat_altarations::SHOT_DAMADE(10.0f)                }),
        Card("Double Damage, Increase Shot Delay",         2, std::vector<PlayerStats>{ stat_altarations::SHOT_DAMADE_MULTIPLAYER(2.0f)       ,
                                                                                        stat_altarations::SHOT_DELAY_MULTIPLAYER(1.5f)      }),
        Card("HP Up, Full Health",                         4, std::vector<PlayerStats>{ stat_altarations::MAX_HP(1)                           ,
                                                                                        stat_altarations::CURR_HP(999)                      }),
        Card("HP Up, Increase Shot Speed",                 3, std::vector<PlayerStats>{ stat_altarations::MAX_HP(1)                           ,
                                                                                        stat_altarations::CURR_HP(1)                          ,
                                                                                        stat_altarations::SHOT_SPEED(0.01f)                 }),
        Card("Half Shot Delay, Decrease Shot Speed",       2, std::vector<PlayerStats>{ stat_altarations::SHOT_DELAY_MULTIPLAYER(0.5f)        ,
                                                                                        stat_altarations::SHOT_SPEED(-0.015f)               }),
        Card("All stats Up, HP down",                      1, std::vector<PlayerStats>{ stat_altarations::MAX_HP(-1)                          ,
                                                                                        stat_altarations::SHOT_DAMADE(10.0f)                  ,
                                                                                        stat_altarations::SHOT_DELAY(-50.0)                   ,
                                                                                        stat_altarations::SHOT_SPEED(0.01f)                 }),
        Card("+1 Shots, Increase Shot Delay",              2, std::vector<PlayerStats>{ stat_altarations::NO_SHOTS(1)                         ,
                                                                                        stat_altarations::SHOT_DELAY(200.0)                 }),
        Card("+2 Shots, Half Shot Delay, Damage Way Down", 1, std::vector<PlayerStats>{ stat_altarations::NO_SHOTS(2)                         ,
                                                                                        stat_altarations::SHOT_DELAY_MULTIPLAYER(0.5f)        ,
                                                                                        stat_altarations::SHOT_DAMADE_MULTIPLAYER(0.3f)     })
    }};
};