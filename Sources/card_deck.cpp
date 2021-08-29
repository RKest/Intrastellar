#include "card_deck.h"

Card::Card(const std::string &cardText)
	: cardText(cardText)
{
}

void CardDeck::DrawCards()
{
	glm::mat4 perFrameProjectileTransform = glm::translate(glm::vec3(0, _timer.Scale(_stats.shotSpeed), 0));
	//Translate projectiles
	std::for_each(std::execution::par_unseq, _projectileInstanceTransforms.begin(), _projectileInstanceTransforms.end(),
		[&perFrameProjectileTransform](auto &mat){ mat *= perFrameProjectileTransform; });
	//Remove all projectiles that hit a target (no STL didn't do a good job)
	std::vector<glm::mat4>::iterator i = _projectileInstanceTransforms.begin();
	while(i != _projectileInstanceTransforms.end())
	{
		glm::vec2 projectilePosition = glm::vec2(*i * glm::vec4(0,0,0,1));
		for(ui j = 0; j < 3; ++j)
		{
			if(_targetBoundingBoxes[j].IsThereAnIntersection(projectilePosition))
			{
				_projectileInstanceTransforms.erase(i);
				break;
			}
		}
		++i;
	}

	for (ui i = 0; i < _noCards; ++i)
	{
		//Shooting
		if (_timer.HeapIsItTime(_cardClockIds[i]))
			if (_projectileInstanceTransforms.size() == _maxProjectileAmount)
			{
				_projectileInstanceTransforms[_oldestProjectileIndex] = _pcInstanceTransforms[i];
				_oldestProjectileIndex = (_oldestProjectileIndex + 1) % _maxProjectileAmount;
			}
			else
				_projectileInstanceTransforms.push_back(_pcInstanceTransforms[i]);
		//Checking Mouse Card Hovers
		if(_cardBoundingBoxes[i].IsThereAnIntersection(helpers::mouseCoordsTransformed(_inverseFlippedCardBorderProjection)))
		{
			bool tempIsLBMPressed = helpers::IsLBMPressed();
			if(_isLBMPressed && !tempIsLBMPressed)
				_choseCards(i);
			else
				_isLBMPressed = tempIsLBMPressed;
		}
	}

	helpers::render(_overlayShader, _overlayMesh);
	helpers::render(_projectileShader, _projectileMesh, _projectileInstanceTransforms.data(), _projectileInstanceTransforms.size(), 
		_blankTransform, _cardBorderProjection);
	helpers::render(_pcShader, _pcMesh, _pcInstanceTransforms.data(), _noCards, _blankTransform, _cardBorderProjection);
	helpers::render(_cardBorderShader, _cardBorderMesh, _cardBorderInstanceTransforms.data(), _noCards, _blankTransform, _cardBorderProjection);
	helpers::render(_targetShader, _targetMesh, _targetInstanceTransforms.data(), _noCards, _blankTransform, _cardBorderProjection);
}

CardDeck::CardDeck(Shader &pcShader, Shader &projectileShader, Shader &targetShader, Text &text, Timer &timer, Stats &stats,
    const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projectileParams, const UntexturedMeshParams &overlayParams,
    const UntexturedMeshParams &cardBorderParams, const UntexturedMeshParams &targetMeshParams)
	: _pcShader(pcShader), _projectileShader(projectileShader), _targetShader(targetShader), _text(text), _timer(timer), _stats(stats),
	  _pcMesh(pcParams, 3 /*_noCards*/), _projectileMesh(projectileParams, 90 /*_maxProjectileAmount*/), _overlayMesh(overlayParams), 
	  _overlayShader("./Shaders/Overlay", UNIFORMS), _cardBorderShader("./Shaders/CardBorder", UNIFORMS),
	  _cardBorderParams(cardBorderParams), _cardBorderMesh(cardBorderParams, 3, 4), 
	  _targetParams(targetMeshParams), _targetMesh(targetMeshParams, 3, 4)
{
	_pcTransform.SetRotAngle(-25.0f);
	_cardBorderProjection = glm::ortho(SCREEN_ASPECT, -SCREEN_ASPECT, -1.0f, 1.0f);
	_inverseFlippedCardBorderProjection = glm::inverse(glm::ortho(-SCREEN_ASPECT, SCREEN_ASPECT, -1.0f, 1.0f));
	for (ui i = 0; i < _noCards; ++i)
	{
		const ft xOffset = -0.9f + i * 0.9f;
		_pcTransform				 .Pos().x = xOffset; 
		_pcTransform				 .Pos().y = -0.3;
		_pcTransform				 .Scale() = glm::vec3(0.1);

		const glm::mat4 pcModel 				 = _pcTransform.Model();
		const glm::mat4 targetInstanceTransform  = pcModel * glm::translate(glm::vec3(0, 5, 0)) * 
			glm::rotate(glm::radians(-7.0f), glm::vec3(0,0,1));
		_cardBorderTransform		 .Pos().x = xOffset; 
		_pcInstanceTransforms		 .push_back(pcModel);
		_cardBorderInstanceTransforms.push_back(_cardBorderTransform.Model());
		_targetInstanceTransforms	 .push_back(targetInstanceTransform);
		_cardBoundingBoxes  [i] 	  = helpers::BoundingBox(_cardBorderParams, _cardBorderInstanceTransforms[i]);
		_targetBoundingBoxes[i]		  = helpers::BoundingBox(_targetParams, 	_targetInstanceTransforms[i]);
		_timer.InitHeapClock(_cardClockIds[i], _cardShotDelays[i]);
	}
}

void CardDeck::_choseCards(const ui cardIndex)
{
	_isLBMPressed = false;
	_areCardsDrawn = false;
}
