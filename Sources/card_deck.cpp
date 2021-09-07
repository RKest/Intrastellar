#include "card_deck.h"

Card::Card(const std::string &cardText, const ui weight, const std::vector<Stats> &statAltarationList)
	: cardText(cardText), weight(weight), statAltarations(std::accumulate(statAltarationList.begin(), statAltarationList.end(), Stats()))
{
}

void CardDeck::DrawCards()
{
	helpers::render(_overlayShader, _overlayMesh);
    _pcDrawFunction(_pcInstanceTransforms, _projInstanceTransforms, _clockIds, _targetBoundingBoxes, _cardProjection, _oldestProjectileIndex);
	helpers::render(_cardBorderShader, _cardBorderMesh, _cardBorderInstanceTransforms.data(), NO_CARDS, _blankTransform, _cardProjection);
	helpers::render(_targetShader, _targetMesh, _targetInstanceTransforms.data(), NO_CARDS, _blankTransform, _cardProjection);

	for (ui i = 0; i < NO_CARDS; ++i)
	{
		if(_cardBoundingBoxes[i].IsThereAnIntersection(helpers::mouseCoordsTransformed(_inverseFlippedCardBorderProjection)))
		{
			bool tempIsLBMPressed = helpers::IsLBMPressed();
			_text.Render(_cards[_chosenCardIndices[i]].cardText, 100.0f, 100.0f, 0.5f, glm::vec3(1));
			if(_isLBMPressed && !tempIsLBMPressed)
				_choseCards(i);
			else
				_isLBMPressed = tempIsLBMPressed;
		}
	}
}

CardDeck::CardDeck(Shader &targetShader, helpers::Core &core, const UntexturedMeshParams &overlayParams, const UntexturedMeshParams &cardBorderParams, 
	const UntexturedMeshParams &targetMeshParams, const pcDrawFunc &drawPcCb)
	:  _targetShader(targetShader), _text(core.text), _timer(core.timer), _stats(core.stats), _overlayMesh(overlayParams), 
	  _overlayShader("./Shaders/Overlay"), _cardBorderShader("./Shaders/CardBorder"), _customRand(CUSTOM_RAND_SEED),
	  _cardBorderParams(cardBorderParams), _cardBorderMesh(cardBorderParams, NO_CARDS, 4),
	  _targetParams(targetMeshParams), _targetMesh(targetMeshParams, NO_CARDS, 4), _pcDrawFunction(drawPcCb)
{
	_pcTransform.SetRotAngle(-25.0f);
	_cardProjection = glm::ortho(-SCREEN_ASPECT, SCREEN_ASPECT, -1.0f, 1.0f);
	_inverseFlippedCardBorderProjection = glm::inverse(glm::ortho(-SCREEN_ASPECT, SCREEN_ASPECT, -1.0f, 1.0f));
	for (ui i = 0; i < NO_CARDS; ++i)
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
		_cardBoundingBoxes  [i] = helpers::BoundingBox(_cardBorderParams, _cardBorderInstanceTransforms[i]);
		_targetBoundingBoxes[i]	= helpers::BoundingBox(_targetParams, targetInstanceTransform);
	}

	for(ui i = 0; i < _cards.size(); ++i)
        _cardWeightSum += _cards[i].weight;
	
}

void CardDeck::RollCards()
{
	_areCardsDrawn = true;
	_chosenCardIndices[0] = 9999;
	_chosenCardIndices[1] = 9999;
	_chosenCardIndices[2] = 9999;
	ui rolledCards = 0;
	ui rAcc = 0;
	while (rolledCards < NO_CARDS)
	{
		const auto r = _customRand.NextU32(0, _cardWeightSum);
		for(ui i = 0; i < _cards.size(); ++i)
		{
			rAcc += _cards[i].weight;
			if(rAcc > r)
			{
				if(std::find(_chosenCardIndices.begin(), _chosenCardIndices.end(), i) == _chosenCardIndices.end())
				{
					_cardShotDelays[rolledCards] = _stats.actualShotDelay + _cards[i].statAltarations.actualShotDelay;
					_timer.InitHeapClock(_clockIds[rolledCards], _cardShotDelays[rolledCards]);
					_chosenCardIndices[rolledCards++] = i;
				}
				rAcc = 0;
				break;
			}
		}
	}
}

void CardDeck::_choseCards(const ui cardIndex)
{
	_isLBMPressed = false;
	_areCardsDrawn = false;
	_stats += _cards[_chosenCardIndices[cardIndex]].statAltarations;
	for(auto i : _clockIds)
		_timer.DestroyHeapClock(i);
}
