#include "player_character.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

PlayerCharacter::PlayerCharacter(const UntexturedMeshParams &pcParams)
	:  _pcMesh(pcParams), _pcBoundingBox(pcParams)
{
	IPlayerCharacter::Init(this);
}

void PlayerCharacter::Reset()
{
	_enemiesShotCounter = 0;
	g_playerStats = defaultStats;
}

void PlayerCharacter::RenderScore()
{
	//Setting the leading zeroes
	std::string scoreString = "";
	for(ui i = 4; i != 0; --i)
	{
		if (i > std::to_string(_enemiesShotCounter).length())
			scoreString += "0";
		else
		{
			scoreString += std::to_string(_enemiesShotCounter);
			break;
		}
	}
		
	Text::Render(scoreString, 10.0f, static_cast<ft>(SCREEN_HEIGHT) - 40.0f, 1.0f, glm::vec3(1));
}

void PlayerCharacter::Update()
{
	if(_isInvincible && !m_invincibilityClock.Inspect())
		_pcAlphaValue.second = _setAlpha(m_invincibilityClock.RemainingTime());
	else
		_pcAlphaValue.second = 1.0f;

	_pcBoundingBox.UpdateCoords(_pcTransform.Model());
}

void PlayerCharacter::Draw()
{
	helpers::render(_pcShader, _pcMesh, _pcTransform.Model(), Camera::ViewProjection(), _pcAlphaValue);
}

void PlayerCharacter::_pcIntersection()
{
	if(!_isInvincible)
	{
		_isInvincible = true;
		g_playerStats.currHP -= 1;
		if(!g_playerStats.currHP)
			_isAlive = false;
		else
		{
			m_invincibilityClock.Init(_invincibilityDuration, [this](){
				_isInvincible = false;
			});
		}
	}
}

constexpr ft PlayerCharacter::_setAlpha(db remainingInvincibilityTime)
{
	const db period = 3.0;
	db invincibilityFractionPassed = 1.0 - remainingInvincibilityTime / _invincibilityDuration;
	db scaledInfincibilityFractionPassed = TAU_d * invincibilityFractionPassed * period;
	return cos(static_cast<ft>(scaledInfincibilityFractionPassed));
}