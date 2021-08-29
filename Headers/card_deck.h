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

class Card
{
public: 
    Card(const std::string &cardText);
    Stats statAltaration;

private:
    const std::string &cardText;
};

class CardDeck 
{
public:
    CardDeck(Shader &pcShader, Shader &projectileShader, Shader &targetShader, Text &text, Timer &timer, Stats &stats,
        const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projectileParams, const UntexturedMeshParams &overlayParams,
        const UntexturedMeshParams &cardBorderParams, const UntexturedMeshParams &targetMeshParams);
    void DrawCards();
    inline bool &AreCarsDrawn() { return _areCardsDrawn; }

protected:
private:
    Shader _cardBorderShader;
    Shader _overlayShader;
    Shader &_pcShader;
    Shader &_projectileShader;
    Shader &_targetShader;
    Text &_text;
    Timer &_timer;
    Stats  &_stats;
    UntexturedMeshParams    _cardBorderParams;
    UntexturedMeshParams    _targetParams;
    UntexturedInstancedMesh _pcMesh;
    UntexturedInstancedMesh _projectileMesh;
    UntexturedInstancedMesh _cardBorderMesh;
    UntexturedInstancedMesh _targetMesh;
    UntexturedMesh          _overlayMesh;
    Transform _pcTransform;
    Transform _cardBorderTransform;

    std::vector<glm::mat4> _pcInstanceTransforms;
    std::vector<glm::mat4> _projectileInstanceTransforms;
    std::vector<glm::mat4> _cardBorderInstanceTransforms;
    std::vector<glm::mat4> _targetInstanceTransforms;
    std::vector<glm::vec2> _projectilePositions;
    std::vector<ui> _cardClockIds = std::vector<ui>(3);
    std::vector<db> _cardShotDelays = { 100.0, 150.0, 300.0 };
    std::vector<helpers::BoundingBox> _cardBoundingBoxes = std::vector<helpers::BoundingBox>(3);
    std::vector<helpers::BoundingBox> _targetBoundingBoxes = std::vector<helpers::BoundingBox>(3);

    glm::mat4 _cardBorderProjection;
    glm::mat4 _inverseFlippedCardBorderProjection; //Kinda just stumbeled myself upon that one, by all metrics it shouldn't work but it does

    const ui _noCards = 3;
	const db _perFrameProjectileTravel = 0.02;
	const ui _maxProjectileAmount = 90;

	ui _oldestProjectileIndex = 0;
    bool _areCardsDrawn = false;
    bool _isLBMPressed = false;

    void _choseCards(const ui cardIndex);
};

#endif