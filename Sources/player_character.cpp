#include "player_character.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

PlayerCharacter::PlayerCharacter(helpers::Core &core, const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projParams)
	: _camera(core.camera), _text(core.text), _timer(core.timer), _pcStats(core.stats),
		_pcMesh(pcParams), _projMesh(projParams, MAX_PROJ_AMOUNT), _pcCardMesh(pcParams, NO_CARDS), _projCardMesh(projParams, CARD_MAX_PROJ_COUNT)
{
	_projectileShader.Bind();
	const std::vector<glm::vec3> colours = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
	for (ui i = 0; i < colours.size(); ++i)
		_projectileShader.SetUni("colours[" + std::to_string(i) + ']', colours[i]);
}

void PlayerCharacter::Reset()
{
	_projInstanceTransforms.clear();
	_oldestProjectileIndex = 0;
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

void PlayerCharacter::Update(const std::vector<glm::mat4> &enemyInstanceTransforms)
{
	if(_isInvincible && _timer.HeapIsItTime(_invincibilityClockId))
		_isInvincible = false;

	if(_isInvincible)
		_pcAlphaValue.second = _setAlpha(_timer.RemainingTime(_invincibilityClockId));
	else
		_pcAlphaValue.second = 1.0f;

	std::for_each
	(
		std::execution::par_unseq,
		_projInstanceTransforms.begin(), _projInstanceTransforms.end(), 
			[this, enemyInstanceTransforms](auto &&mat){mat *= _moveProj(enemyInstanceTransforms, mat); }
	);
}

void PlayerCharacter::Draw()
{
	helpers::render(_projectileShader, _projMesh, _projInstanceTransforms.data(), _projInstanceTransforms.size(), 
		_blankTransform, _camera.ViewProjection());
	helpers::render(_pcShader, _pcMesh, _pcTransform.Model(), _camera.ViewProjection(), _pcAlphaValue);
}

void PlayerCharacter::Shoot(const glm::mat4 &originTransform)
{
    helpers::pushToCappedVector(_projInstanceTransforms, originTransform, _oldestProjectileIndex, MAX_PROJ_AMOUNT);
}

void PlayerCharacter::_projHit(const ui index)
{
	_projInstanceTransforms[index] = _projInstanceTransforms.back();
	_projInstanceTransforms.pop_back();
	_enemiesShotCounter++;
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

void PlayerCharacter::_externDraw(const std::vector<glm::mat4> &pcTransforms, std::vector<glm::mat4> &projTransforms, const std::vector<ui> &clockIds, 
	const std::vector<helpers::BoundingBox> &targetBoundingBoxes, const glm::mat4 &projection, ui &oldestProjIndex)
{
	ui projIndex;
	for(auto &targetBoundingBox : targetBoundingBoxes)
		if(targetBoundingBox.IsThereAnIntersection(projTransforms, projIndex))
			projTransforms.erase(projTransforms.begin() + projIndex);

	glm::mat4 perFrameProjectileTransform = glm::translate(glm::vec3(0, _timer.Scale(0.05f), 0));
	std::for_each(std::execution::par_unseq, projTransforms.begin(), projTransforms.end(),
		[&perFrameProjectileTransform](auto &mat){ mat *= perFrameProjectileTransform; });

	for (ui i = 0; i < clockIds.size(); ++i)
		if (_timer.HeapIsItTime(clockIds[i]))
            helpers::pushToCappedVector(projTransforms, pcTransforms[i], oldestProjIndex, CARD_MAX_PROJ_COUNT);

	helpers::render(_projectileShader, _projCardMesh, projTransforms.data(), projTransforms.size(), _blankTransform, projection);
	helpers::render(_pcCardShader, _pcCardMesh, pcTransforms.data(), pcTransforms.size(), _blankTransform, projection);
}
 
constexpr ft PlayerCharacter::_setAlpha(db remainingInvincibilityTime)
{
	const ft period = 3.0f;
	ft invincibilityFractionPassed = 1 - remainingInvincibilityTime / _invincibilityDuration;
	ft scaledInfincibilityFractionPassed = TAU * invincibilityFractionPassed * period;
	return cos(scaledInfincibilityFractionPassed);
}


glm::mat4 PlayerCharacter::_moveProj(const std::vector<glm::mat4> &enemyInstanceTransforms, const glm::mat4 &projTransform) const
{
	const ft maxProjTurningRadius = _timer.Scale(MAX_PROJ_TURNING_RAD);
	const glm::mat4 perFrameTransform = glm::translate(glm::vec3(0, _timer.Scale(_pcStats.shotSpeed), 0));
	if(_pcStats.shotHomingStrength == 0.0f)
		return perFrameTransform; 
	
	const glm::vec2 projPos{projTransform * glm::vec4(0,0,0,1)};
	auto closestEnemyMat = std::min_element(enemyInstanceTransforms.cbegin(), enemyInstanceTransforms.cend(), [&projPos](auto &m1, auto &m2)
		{ return glm::distance(glm::vec2(m1 * glm::vec4(0,0,0,1)), projPos) < glm::distance(glm::vec2(m2 * glm::vec4(0,0,0,1)), projPos); });

	if(closestEnemyMat == enemyInstanceTransforms.end())
		return perFrameTransform;

	const glm::vec2 closestEnemyPos{*closestEnemyMat * glm::vec4(0,0,0,1)};
	if(glm::distance(projPos, closestEnemyPos) > _pcStats.shotHomingStrength)
		return perFrameTransform; 
		
	const glm::vec2 enemyVec{glm::normalize(glm::vec2(glm::inverse(projTransform) * glm::vec4(closestEnemyPos,0,1)))};
	const ft angle = -glm::atan(glm::dot({1.0, 0.0}, enemyVec), helpers::det({1.0, 0.0}, enemyVec));
	if(angle < maxProjTurningRadius)
		return glm::rotate(angle, glm::vec3(0,0,1)) * perFrameTransform;
	if(TAU - angle < maxProjTurningRadius)
		return glm::rotate(-angle, glm::vec3(0,0,1)) * perFrameTransform;
	if(angle < PI)
		return glm::rotate(maxProjTurningRadius, glm::vec3(0,0,1)) * perFrameTransform;
	return glm::rotate(-maxProjTurningRadius, glm::vec3(0,0,1)) * perFrameTransform;
}