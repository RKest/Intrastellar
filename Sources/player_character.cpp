#include "player_character.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

PlayerCharacter::PlayerCharacter(const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projParams, Stats &pcStats, 
    Camera &camera, Text &text, Timer &timer)
	: _pcShader("./Shaders/PC"), _projectileShader("./Shaders/Projectile"), _pcCardShader("./Shaders/PC_Card"),
		_pcMesh(pcParams), _projMesh(projParams), _maxProjectileAmount(maxProjectileAmount), _camera(camera), _text(text), _timer(timer), 
		_pcCardMesh(pcParams, NO_CARDS, 4), _projCardMesh(projParams, CARD_MAX_PROJ_COUNT, 4)
{
	_perFrameProjectileTravel = 0.05;
	_pcIntersectionCallback = [this]{ _isAlive = false; };
	_projHitCallback  = [this](ui projectileIndex)
	{
		_projInstanceTransforms[projectileIndex] = _projInstanceTransforms.back();
		_projInstanceTransforms.pop_back();
		_enemiesShotCounter++;
	};
	
	_projectileShader.Bind();
	const std::vector<glm::vec3> colours = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
	for (ui i = 0; i < colours.size(); ++i)
		_projectileShader.SetVec3("colours[" + std::to_string(i) + ']', colours[i]);
}

void PlayerCharacter::Reset()
{
	_projInstanceTransforms.clear();
	_oldestProjectileIndex = 0;
	_enemiesShotCounter = 0;
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

void PlayerCharacter::Update()
{
	_perFrameProjectileTransform = glm::translate(glm::vec3(0.0f, _timer.Scale(_pcStats.shotSpeed), 0.0f));
	std::for_each
	(
		std::execution::par_unseq,
		_projInstanceTransforms.begin(), _projInstanceTransforms.end(),
		[this](auto &&mat){ mat *= _perFrameProjectileTransform; }
	);
}

void PlayerCharacter::Draw()
{
	helpers::render(_projectileShader, _projMesh, _projInstanceTransforms.data(), _projInstanceTransforms.size(), 
		_blankTransform, _camera.ViewProjection());
	helpers::render(_pcShader, _pcMesh, _pcTransform.Model(), _camera.ViewProjection());
}

void PlayerCharacter::ExternDraw(const std::vector<glm::mat4> &pcTransforms, std::vector<glm::mat4> &projTransforms, const std::vector<ui> &clockIds, 
	const std::vector<helpers::BoundingBox> &targetBoundingBoxes, const std::vector<Stats> &pcStats, const glm::mat4 &projection, ui &oldestProjIndex)
{
	for(auto i = projTransforms.begin(); i != projTransforms.end(); ++i)
	{
		glm::vec2 projectilePosition = glm::vec2(*i * glm::vec4(0,0,0,1));
		for(ui j = 0; j < targetBoundingBoxes.size(); ++j)
		{
			if(targetBoundingBoxes[j].IsThereAnIntersection(projectilePosition))
			{
				projTransforms.erase(i);
				break;
			}
		}
	}

	for (ui i = 0; i < clockIds.size(); ++i)
	{
		glm::mat4 perFrameProjectileTransform = glm::translate(glm::vec3(0, _timer.Scale(pcStats[i].shotSpeed), 0));
		std::for_each(std::execution::par_unseq, projTransforms.begin(), projTransforms.end(),
			[&perFrameProjectileTransform](auto &mat){ mat *= perFrameProjectileTransform; });

		if (_timer.HeapIsItTime(clockIds[i]))
            helpers::pushToCappedVector(projTransforms, pcTransforms[i], oldestProjIndex, CARD_MAX_PROJ_COUNT);
	}

	helpers::render(_projectileShader, _projCardMesh, projTransforms.data(), projTransforms.size(), _blankTransform, projection);
	helpers::render(_pcCardShader, _pcCardMesh, projTransforms.data(), projTransforms.size(), _blankTransform, projection);
}

void PlayerCharacter::Shoot(const glm::mat4 &originTransform)
{
    helpers::pushToCappedVector(_projInstanceTransforms, originTransform, _oldestProjectileIndex, MAX_PROJ_AMOUNT);
}