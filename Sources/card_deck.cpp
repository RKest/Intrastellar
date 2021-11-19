#include "card_deck.h"

Card::Card(const std::string &cardText, const ui weight, const std::vector<PlayerStats> &statAltarationList)
	: cardText(cardText), weight(weight), statAltarations(std::accumulate(statAltarationList.begin(), statAltarationList.end(), PlayerStats()))
{
}

void CardDeck::DrawCards()
{
	if(!_areCardsFullyDrawn)
	{
		const db remainingTime = m_overlayClock.RemainingTime();
		const db remainingTimeFraction = 1.0 - remainingTime / OVERLAY_TRANSITION_TIME;
		const ft overlayAlpha = decl_cast(overlayAlpha, remainingTimeFraction) * OVERLAY_MAX_APLHA;
		_overlayAlphaUni.second = overlayAlpha;
		m_overlayClock.Inspect();
	}
	helpers::render(_overlayShader, _overlayMesh, _overlayAlphaUni);
	helpers::render(_cardBorderShader, _cardBorderMesh, _cardBorderInstanceTransforms.data(), NO_CARDS, _blankTransform, _cardProjection);

	for (ui i = 0; i < NO_CARDS; ++i)
	{
		if(_cardBoundingBoxes[i].IsThereAnIntersection(helpers::mouseCoordsTransformed(_inverseFlippedCardBorderProjection)))
		{
			bool tempIsLBMPressed = helpers::IsLBMPressed();
			Text::Render(_cards[_chosenCardIndices[i]].cardText, 100.0f, 100.0f, 0.5f, glm::vec3(1));
			if(_isLBMPressed && !tempIsLBMPressed)
				_choseCards(i);
			else
				_isLBMPressed = tempIsLBMPressed;
		}
	}
}

CardDeck::CardDeck(helpers::Core &core, const UntexturedMeshParams &overlayParams, const UntexturedMeshParams &cardBorderParams)
	:   _stats(core.stats), _customRand(CUSTOM_RAND_SEED), _overlayMesh(overlayParams),
	  _cardBorderParams(cardBorderParams), _cardBorderMesh(cardBorderParams, NO_CARDS)
{
	for (ui i = 0; i < NO_CARDS; ++i)
	{
		const ft xOffset = -0.9f + static_cast<ft>(i) * 0.9f;
		_cardBorderTransform		 .Pos().x = xOffset; 
		_cardBorderInstanceTransforms.push_back(_cardBorderTransform.Model());
		_cardBoundingBoxes  [i] = ReqBoundingBox(_cardBorderParams, _cardBorderInstanceTransforms[i]);
	}

	for(ui i = 0; i < _cards.size(); ++i)
        _cardWeightSum += _cards[i].weight;
	
}

void CardDeck::RollCards()
{
	_areCardsDrawn = true;
	_areCardsFullyDrawn = false;
	_chosenCardIndices[0] = 9999;
	_chosenCardIndices[1] = 9999;
	_chosenCardIndices[2] = 9999;
	ui rolledCards = 0;
	ui rAcc = 0;

	m_overlayClock = Clock<>(OVERLAY_TRANSITION_TIME, [this]{
		_overlayAlphaUni.second = OVERLAY_MAX_APLHA;
		_areCardsFullyDrawn = true;
	});
	
	while (rolledCards < NO_CARDS)
	{
		const auto r = _customRand.NextU32(0, _cardWeightSum);
		for(ui i = 0; i < _cards.size(); ++i)
		{
			rAcc += _cards[i].weight;
			if(rAcc > r)
			{
				if(!helpers::contains(_chosenCardIndices, i))
				{
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
}
