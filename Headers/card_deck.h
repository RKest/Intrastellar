#ifndef CARD_DECK_H
#define CARD_DECK_H
#include "_config.h"

#include "Core/text.h"
#include "Core/transform.h"
#include "Core/shader.h"
#include "Core/mesh.h"
#include "Core/camera.h"
#include "Core/stats.h"
#include "Core/timer.h"
#include "Core/helpers.h"
#include "Core/display.h"

#include <string>
#include <execution>
#include <algorithm>
#include <functional>

class Card
{
public: 
    Card(const std::string &cardText, const ui weight, const std::vector<Stats> &statAltarationList);
    const ui weight;    
    Stats statAltarations;
private:
    const std::string &_cardText;
};

using pcDrawFunc = std::function<void(const std::vector<glm::mat4>&, std::vector<glm::mat4>&, const std::vector<ui>&, 
    const std::vector<helpers::BoundingBox>&, const std::vector<Stats*>&, const glm::mat4&, ui&)>;
class CardDeck 
{
public:
    CardDeck(Shader &targetShader, Text &text, Timer &timer, Stats &stats
	    const UntexturedMeshParams &overlayParams, const UntexturedMeshParams &cardBorderParams, 
	    const UntexturedMeshParams &targetMeshParams, const pcDrawFunc &drawPcCb);
    void DrawCards();
    inline bool &AreCarsDrawn() { return _areCardsDrawn; }

protected:
private:
    CustomRand _customRand;
    Shader _cardBorderShader;
    Shader _overlayShader;
    Shader &_targetShader;
    Text &_text;
    Timer &_timer;
    Stats  &_stats;
    UntexturedMeshParams    _cardBorderParams;
    UntexturedMeshParams    _targetParams;
    UntexturedInstancedMesh _cardBorderMesh;
    UntexturedInstancedMesh _targetMesh;
    UntexturedMesh          _overlayMesh;
    Transform _pcTransform;
    Transform _cardBorderTransform;

    std::vector<glm::mat4> _cardBorderInstanceTransforms;
    std::vector<glm::mat4> _targetInstanceTransforms;
    std::vector<helpers::BoundingBox> _cardBoundingBoxes = std::vector<helpers::BoundingBox>(3);

    glm::mat4 _cardProjection;
    glm::mat4 _inverseFlippedCardBorderProjection; //Kinda just stumbeled myself upon that one, by all metrics it shouldn't work but it does

	const db _perFrameProjectileTravel = 0.02;

    //Drawing player Characters --- Delegated to a callable in the PlayerCharacter class --- parameters in order listet below
    const pcDrawFunc _pcDrawFunction;
    std::vector<glm::mat4> _pcInstanceTransforms;
    std::vector<glm::mat4> _projInstanceTransforms;
    std::vector<ui> _clockIds = std::vector<ui>(3);
    std::vector<helpers::BoundingBox> _targetBoundingBoxes = std::vector<helpers::BoundingBox>(3);
    std::vector<Stats> _cardStats;
    //_cardBorderProection --- Just above here
	ui _oldestProjectileIndex = 0;

    bool _areCardsDrawn = false;
    bool _isLBMPressed = false;

    void _choseCards(const ui cardIndex);
    void _rollCards();
    std::vector<ui> _chosenCardIndices = std::vector<ui>(3, 9999);
    ui _cardWeightSum = 0;
    const std::array<Card, 9> _cards\
    {{
        Card("Decrease Shot Delay",                        5, std::vector<Stats>{ stat_altarations::SHOT_DELAY(-100.0)                }),
        Card("Increase Damage",                            5, std::vector<Stats>{ stat_altarations::SHOT_DAMADE(10.0f)                }),
        Card("Double Damage, Increase Shot Delay",         2, std::vector<Stats>{ stat_altarations::SHOT_DAMADE_MULTIPLAYER(2.0f)       ,
                                                                                  stat_altarations::SHOT_DELAY_MULTIPLAYER(1.5f)      }),
        Card("HP Up, Full Health",                         4, std::vector<Stats>{ stat_altarations::MAX_HP(1)                           ,
                                                                                  stat_altarations::CURR_HP(999)                      }),
        Card("HP Up, Increase Shot Speed",                 3, std::vector<Stats>{ stat_altarations::MAX_HP(1)                           ,
                                                                                  stat_altarations::SHOT_SPEED(0.01f)                 }),
        Card("Half Shot Delay, Decrease Shot Speed",       2, std::vector<Stats>{ stat_altarations::SHOT_DELAY_MULTIPLAYER(0.5f)        ,
                                                                                  stat_altarations::SHOT_SPEED(-0.015f)               }),
        Card("All stats Up, HP down",                      1, std::vector<Stats>{ stat_altarations::MAX_HP(-1)                          ,
                                                                                  stat_altarations::SHOT_DAMADE(10.0f)                  ,
                                                                                  stat_altarations::SHOT_DELAY(-100.0)                  ,
                                                                                  stat_altarations::SHOT_SPEED(0.1f)                  }),
        Card("+1 Shots, Increase Shot Delay",              2, std::vector<Stats>{ stat_altarations::NO_SHOTS(1)                         ,
                                                                                  stat_altarations::SHOT_DELAY(200.0f)                }),
        Card("+2 Shots, Half Shot Delay, Damage Way Down", 1, std::vector<Stats>{ stat_altarations::NO_SHOTS(2)                         ,
                                                                                  stat_altarations::SHOT_DELAY_MULTIPLAYER(0.5f)        ,
                                                                                  stat_altarations::SHOT_DAMADE_MULTIPLAYER(0.3f)     })
    }};
};

#endif