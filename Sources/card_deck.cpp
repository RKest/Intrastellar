#include "card_deck.h"

Card::Card(const std::string &cardText, const ui weight, const std::vector<Stats> &statAltarationList)
	: _cardText(cardText), weight(weight)
{
	for (auto stat : statAltarationList)
		statAltarations += stat;
}

void CardDeck::DrawCards()
{
	for (ui i = 0; i < _noCards; ++i)
	{
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
    _pcDrawFunction(_pcInstanceTransforms, _projInstanceTransforms, _clockIds, _targetBoundingBoxes, _cardStats, _cardProjection, 
        _oldestProjectileIndex);
	helpers::render(_cardBorderShader, _cardBorderMesh, _cardBorderInstanceTransforms.data(), _noCards, _blankTransform, _cardProjection);
	helpers::render(_targetShader, _targetMesh, _targetInstanceTransforms.data(), _noCards, _blankTransform, _cardProjection);
}

CardDeck::CardDeck(Shader &targetShader, Text &text, Timer &timer, Stats &stats, 
	const UntexturedMeshParams &overlayParams, const UntexturedMeshParams &cardBorderParams, 
	const UntexturedMeshParams &targetMeshParams, const pcDrawFunc &drawPcCb)
	:  _targetShader(targetShader), _text(text), _timer(timer), _stats(stats), _overlayMesh(overlayParams), 
	  _overlayShader("./Shaders/Overlay"), _cardBorderShader("./Shaders/CardBorder"), _customRand(CUSTOM_RAND_SEED),
	  _cardBorderParams(cardBorderParams), _cardBorderMesh(cardBorderParams, NO_CARDS, 4),
	  _targetParams(targetMeshParams), _targetMesh(targetMeshParams, NO_CARDS, 4), _pcDrawFunction(drawPcCb)
{
	_pcTransform.SetRotAngle(-25.0f);
	_cardProjection = glm::ortho(SCREEN_ASPECT, -SCREEN_ASPECT, -1.0f, 1.0f);
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
		_cardBoundingBoxes  [i] 	  = helpers::BoundingBox(_cardBorderParams, _cardBorderInstanceTransforms[i]);
		_targetBoundingBoxes[i]		  = helpers::BoundingBox(_targetParams, 	_targetInstanceTransforms[i]);
        _cardWeightSum += _cards[i].weight;
        _cardStats[i] = &(_cards[i].statAltarations);
		_timer.InitHeapClock(_clockIds[i], _stats.Delay() + _cardStats[i].Delay());
	}
    //	for(ui i = 0; i < 1; ++i)
    // {
	// 	_rollCards();
	// 	testVec.push_back(_chosenCardIndices[0]);
	// 	testVec.push_back(_chosenCardIndices[1]);
	// 	testVec.push_back(_chosenCardIndices[2]);
	// }

	// for(ui i = 0; i < _cards.size(); ++i)
	// 	std::cout << i << ": " << std::count(testVec.begin(), testVec.end(), i) << std::endl;

}

void CardDeck::_rollCards()
{
    _chosenCardIndices[0] = 0;    
    _chosenCardIndices[1] = 1;
    _chosenCardIndices[2] = 2;

//	_chosenCardIndices[0] = 9999;
//	_chosenCardIndices[1] = 9999;
//	_chosenCardIndices[2] = 9999;
//	ui rolledCards = 0;
//	ui rAcc = 0;
//	while (rolledCards < _noCards)
//	{
//		const auto r = _customRand.NextU32(0, _cardWeightSum);
//		for(ui i = 0; i < _cards.size(); ++i)
//		{
//			rAcc += _cards[i].weight;
//			if(rAcc > r)
//			{
//				if(std::find(_chosenCardIndices.begin(), _chosenCardIndices.end(), i) == _chosenCardIndices.end())
//					_chosenCardIndices[rolledCards++] = i;
//				break;
//			}
//		}
//	}
}

void CardDeck::_choseCards(const ui cardIndex)
{
	_isLBMPressed = false;
	_areCardsDrawn = false;
}
