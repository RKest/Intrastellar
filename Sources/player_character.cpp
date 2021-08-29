#include "player_character.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

PlayerCharacter::PlayerCharacter(Shader &pcShader, Shader &projectileShader, const UntexturedMeshParams &pcParams, 
	const UntexturedMeshParams &projectileParams, Camera &camera, Text &text, Timer &timer, const ui maxProjectileAmount)
	: _pcShader(pcShader), _projectileShader(projectileShader), _pcMesh(pcParams), _projectileMesh(projectileParams),
		_maxProjectileAmount(maxProjectileAmount), _camera(camera), _text(text), _timer(timer)
{
	_perFrameProjectileTravel = 0.05;
	_pcIntersectionCallback = [this]{ _isAlive = false; };
	_projectileHitCallback  = [this](ui projectileIndex)
	{
		_projectileInstanceTransforms[projectileIndex] = _projectileInstanceTransforms.back();
		_projectileInstanceTransforms.pop_back();
		_enemiesShotCounter++;
	};
	
	projectileShader.Bind();
	const std::vector<glm::vec3> colours = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
	for (ui i = 0; i < colours.size(); ++i)
		projectileShader.SetVec3("colours[" + std::to_string(i) + ']', colours[i]);
}

void PlayerCharacter::Reset()
{
	_projectileInstanceTransforms.clear();
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
	_perFrameProjectileTransform = glm::translate(glm::vec3(0.0f, _timer.Scale(_perFrameProjectileTravel), 0.0f));
	std::for_each
	(
		std::execution::par_unseq,
		_projectileInstanceTransforms.begin(), _projectileInstanceTransforms.end(),
		[this](auto &&mat){ mat *= _perFrameProjectileTransform; }
	);
}

void PlayerCharacter::Draw()
{
	helpers::render(_projectileShader, _projectileMesh, _projectileInstanceTransforms.data(), _projectileInstanceTransforms.size(), 
		_blankTransform, _camera.ViewProjection());
	helpers::render(_pcShader, _pcMesh, _pcTransform.Model(), _camera.ViewProjection());
}

void PlayerCharacter::Shoot(const glm::mat4 &originTransform)
{
	if (_projectileInstanceTransforms.size() == _maxProjectileAmount)
	{
		_projectileInstanceTransforms[_oldestProjectileIndex] = originTransform;
		_oldestProjectileIndex = (_oldestProjectileIndex + 1) % _maxProjectileAmount;
	}
	else
		_projectileInstanceTransforms.push_back(originTransform);
}

PlayerCharacter::~PlayerCharacter()
{
}
