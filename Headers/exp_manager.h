#ifndef EXP_MANAGER_H
#define EXP_MANAGER_H

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
	ui clockId;
};

using namespace std::placeholders;
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
	Shader _expParticleShader;
	Shader _expBarShader;
	CustomRand _customRand;
	Timer &_timer;
	Camera &_camera;
	UntexturedInstancedMesh _expMesh;
	UntexturedDynamicMesh _expBarMesh;

	ui _currentExp = 0;
	ui _prevLevelExpTreshold = 0;
	ui _nextLevelExpTreshold = 100;
	const ui _levelThresholdMultiplayer = 2;
	bool _hasThereBeenLevelUp = false;
	const glm::mat4 _expBarProjection = glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), 0.0f, static_cast<ft>(SCREEN_HEIGHT));
	std::array<glm::vec3, 4> _expBarPositions{{glm::vec3(0), glm::vec3(0,1,0), glm::vec3(0), glm::vec3(0)}};

	std::list<ui> _expParticleClusterClockIds;
	std::map<ui, std::vector<InstanceState*>> _clockIdToInstanceStatePtrMap;
	std::list<InstanceState> _instanceStates;

	db _expParticleAttractionDelay = 1000.0;
	const ft _expParticleEntropySpeed = 0.002f;
	const ft _expParticleAttractionSpeed = 0.1f;

	void _updateExpBar();
};

#endif