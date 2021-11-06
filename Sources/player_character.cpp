#include "player_character.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

PlayerCharacter::PlayerCharacter(helpers::Core &core, const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projParams)
	: _camera(core.camera), _text(core.text), _timer(core.timer), _pcStats(core.stats), _pcMesh(pcParams), _pcCardMesh(pcParams, NO_CARDS), 
		_projMesh(projParams, MAX_PROJ_AMOUNT), _projCardMesh(projParams, CARD_MAX_PROJ_COUNT), _pcBoundingBox(pcParams), 
		_pcInterface(new IPlayerCharacter(this))
{
	_projectileShader.Bind();
	const std::vector<glm::vec3> colours = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
	for (ui i = 0; i < colours.size(); ++i)
		_projectileShader.SetUni("colours[" + std::to_string(i) + ']', colours[i]);
}

PlayerCharacter::~PlayerCharacter()
{
	delete _pcInterface;
}

void PlayerCharacter::Reset()
{
	_projInstanceTransforms.clear();
	_oldestProjectileIndex0 = 0;
	_oldestProjectileIndex1 = 0;
	_oldestProjectileIndex2 = 0;
	_enemiesShotCounter = 0;
	_pcStats = defaultStats;
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
		
	_text.Render(scoreString, 10.0f, static_cast<ft>(SCREEN_HEIGHT) - 40.0f, 1.0f, glm::vec3(1));
}

void PlayerCharacter::Update(const std::vector<std::vector<glm::mat4>*> &enemyInstanceTransforms)
{
	if(_isInvincible && _timer.HeapIsItTime(_invincibilityClockId))
	{
		_isInvincible = false;
		_timer.DestroyHeapClock(_invincibilityClockId);
	}

	if(_isInvincible)
		_pcAlphaValue.second = _setAlpha(_timer.RemainingTime(_invincibilityClockId));
	else
		_pcAlphaValue.second = 1.0f;

	for (auto &projTransform : _projInstanceTransforms)
		projTransform *= _moveProj(enemyInstanceTransforms, projTransform);
	
	_pcBoundingBox.UpdateCoords(_pcTransform.Model());
}

void PlayerCharacter::Draw()
{
	helpers::render(_projectileShader, _projMesh, _projInstanceTransforms.data(), _projInstanceTransforms.size(),
		_blankTransform, _camera.ViewProjection());
	helpers::render(_pcShader, _pcMesh, _pcTransform.Model(), _camera.ViewProjection(), _pcAlphaValue);
}

void PlayerCharacter::Shoot()
{
	const auto pushProj = [this](const glm::mat4& transform)
	{
    	helpers::pushToCappedVector(_projInstanceTransforms, transform, _oldestProjectileIndex0, MAX_PROJ_AMOUNT);
		helpers::pushToCappedVector(_noLeftProjPiercings, _pcStats.noPiercings, _oldestProjectileIndex1, MAX_PROJ_AMOUNT);
		helpers::pushToCappedVector(_alreadyHitEnemyIds, std::vector<ui>(), _oldestProjectileIndex2, MAX_PROJ_AMOUNT);
	};

	const glm::mat4 originTransform = _pcTransform.Model();
	if(_pcStats.noShots == 1)
	{
		pushProj(originTransform);
	}
	else if (_pcStats.noShots % 2 == 0)
	{
		for (ui i = 0; i < _pcStats.noShots >> 1; ++i)
		{
			const ft shotAngle = static_cast<ft>(i) * DEF_ANGLE_BETWEEN_SHOTS;
			pushProj(originTransform * glm::translate(glm::vec3(0.1,0,0)) * glm::rotate(shotAngle, glm::vec3(0,0,1)));
			pushProj(originTransform * glm::translate(glm::vec3(0.1,0,0)) * glm::rotate(TAU - shotAngle, glm::vec3(0,0,1)));
		}
	}
	else
	{
		pushProj(originTransform);
		for (ui i = 1; i <= _pcStats.noShots >> 1; ++i)
		{
			const ft shotAngle = static_cast<ft>(i) * DEF_ANGLE_BETWEEN_SHOTS;
			pushProj(originTransform * glm::rotate(shotAngle, glm::vec3(0,0,1)));
			pushProj(originTransform * glm::rotate(TAU - shotAngle, glm::vec3(0,0,1)));
		}
	}
}

bool PlayerCharacter::_projHit(const ui projIndex, const ui enemyIndex)
{
	if(!helpers::contains(_alreadyHitEnemyIds[projIndex], enemyIndex))
	{
		_noLeftProjPiercings[projIndex] -= 1;
		if(!_noLeftProjPiercings[projIndex])
		{
			_projInstanceTransforms	[projIndex] = _projInstanceTransforms	.back();
			_noLeftProjPiercings	[projIndex] = _noLeftProjPiercings		.back();
			_alreadyHitEnemyIds		[projIndex] = _alreadyHitEnemyIds		.back();
			_projInstanceTransforms											.pop_back();
			_noLeftProjPiercings											.pop_back();
			_alreadyHitEnemyIds												.pop_back();
		}
		else
			_alreadyHitEnemyIds[projIndex].push_back(enemyIndex);
		_enemiesShotCounter++;
		return true;
	}
	return false;
}

void PlayerCharacter::_pcIntersection()
{
	if(!_isInvincible)
	{
		_pcStats.currHP -= 1;
		if(!_pcStats.currHP)
			_isAlive = false;
		else
		{
			_timer.InitHeapClock(_invincibilityClockId, _invincibilityDuration);
			_isInvincible = true;
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


glm::mat4 PlayerCharacter::_moveProj(const std::vector<std::vector<glm::mat4>*> &enemyInstanceTransforms, const glm::mat4 &projTransform) const
{
	const ft maxProjTurningRadius = _timer.Scale(MAX_PROJ_TURNING_RAD);
	const glm::mat4 perFrameTransform = glm::translate(glm::vec3(0, _timer.Scale(_pcStats.shotSpeed), 0));
	if(_pcStats.shotHomingStrength == 0.0f)
		return perFrameTransform; 

	if(enemyInstanceTransforms.empty())
		return perFrameTransform;
		
	const glm::vec2 projPos{projTransform * glm::vec4(0,0,0,1)};
	auto closestEnemyMatIter = enemyInstanceTransforms[0]->cbegin();
	for(auto &enemyInstanceTransformsVecPtr : enemyInstanceTransforms)
	{
		const auto closestEnemyMat = std::min_element(enemyInstanceTransformsVecPtr->cbegin(), enemyInstanceTransformsVecPtr->cend(), [&projPos](auto &m1, auto &m2)
			{ return glm::distance(glm::vec2(m1 * glm::vec4(0,0,0,1)), projPos) < glm::distance(glm::vec2(m2 * glm::vec4(0,0,0,1)), projPos); });
		if(glm::distance(glm::vec2(*closestEnemyMatIter * glm::vec4(0,0,0,1)), projPos) > 
		   glm::distance(glm::vec2(*closestEnemyMat 	* glm::vec4(0,0,0,1)), projPos))
			closestEnemyMatIter = closestEnemyMat;
	}

	for(auto &enemyInstanceTransformsVecPtr : enemyInstanceTransforms)
		if(closestEnemyMatIter == enemyInstanceTransformsVecPtr->cend())
			return perFrameTransform;

	const glm::vec2 closestEnemyPos{*closestEnemyMatIter * glm::vec4(0,0,0,1)};
	if(glm::distance(projPos, closestEnemyPos) > _pcStats.shotHomingStrength)
		return perFrameTransform; 
		
	const ft angle = helpers::angleBetweenPoints(projTransform, *closestEnemyMatIter);
	if(angle < maxProjTurningRadius)
		return glm::rotate(angle, glm::vec3(0,0,1)) * perFrameTransform;
	if(TAU - angle < maxProjTurningRadius)
		return glm::rotate(-angle, glm::vec3(0,0,1)) * perFrameTransform;
	if(angle < PI)
		return glm::rotate(maxProjTurningRadius, glm::vec3(0,0,1)) * perFrameTransform;
	return glm::rotate(-maxProjTurningRadius, glm::vec3(0,0,1)) * perFrameTransform;
}