#pragma once
#include "_config.h"
#include "Core/mesh.h"
#include "Core/timer.h"
#include "Core/random.h"
#include "Core/helpers.h"
#include "Core/camera.h"
#include "Core/transform.h"

#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <array>

enum ExpPartcleBehaviour : ui
{
	ENTROPY,
	ATTRACTION
};

struct InstanceState
{
	glm::mat4 randomEntropyDirection;
	glm::mat4 transform;
	ExpPartcleBehaviour behaviour;
};

using namespace std::placeholders;
using expStateClock_t = Clock<ui>;
class ExpManager
{
public:
	ExpManager(helpers::Core &core, const UntexturedMeshParams &expMeshParams, const UntexturedMeshParams &expBarMeshParams);

	void UpdateExpParticles(const glm::mat4 &pcModel);
	void CreateExpParticles(const glm::mat4 &originModel, const ui noParticles);
	void Reset();

	inline auto CreateExpParticlesCb() { return std::bind(&ExpManager::CreateExpParticles, this, _1, _2); }
	inline bool HasThereBeenLevelUp()  { return _hasThereBeenLevelUp; }
	
protected:
private:
	CustomRand _customRand;
	Shader _expParticleShader{"Shaders/Exp"};
	Shader _expBarShader{"Shaders/ExpBar"};
	UntexturedInstancedMesh _expMesh;
	UntexturedDynamicMesh _expBarMesh;

	ui _currentExp = 0;
	ui _prevLevelExpTreshold = 0;
	ui _nextLevelExpTreshold = 100;
	const ft _levelThresholdMultiplayer = 1.5f;
	bool _hasThereBeenLevelUp = false;
	const glm::mat4 _expBarProjection = glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), 0.0f, static_cast<ft>(SCREEN_HEIGHT));
	std::array<glm::vec3, 4> _expBarPositions{{glm::vec3(0), glm::vec3(0,1,0), glm::vec3(0), glm::vec3(0)}};

	std::list<expStateClock_t> _expParticleClusterClockIds;
	std::map<ui, std::vector<InstanceState*>> _clockIdToInstanceStatePtrMap;
	std::list<InstanceState> _instanceStates;

	db _expParticleAttractionDelay = 1000.0;
	const db _expParticleEntropySpeed = 0.002;
	const db _expParticleAttractionSpeed = 0.1;

	void _updateExpBar();
	inline InstanceState initInstanceState(const glm::mat4 &originModel) 
	{ 
		return InstanceState{
			glm::translate(glm::vec3(helpers::randomDirVector(_customRand, static_cast<ft>(Timer::Scale(_expParticleEntropySpeed))), 0.0f)),
			originModel, ExpPartcleBehaviour::ENTROPY
		}; 
	}
};