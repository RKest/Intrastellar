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

class ExpManager
{
public:
	ExpManager(Shader &shader, Camera &camera, Timer &timer, const UntexturedMeshParams &expMeshParams, const ui customRandSeed, const ui maxNoExpParticles);

	void UpdateExpParticles(const glm::mat4 &pcModel);
	void CreateExpParticles(const glm::mat4 &originModel, const ui noParticles);
	void Reset();
	
protected:
private:
	CustomRand customRand;
	Shader &shader;
	Timer &timer;
	Camera &camera;
	Transform transform;
	UntexturedInstancedMesh expMesh;

	ui currentExp = 0;

	std::list<ui> expParticleClusterClockIds;
	std::map<ui, std::vector<InstanceState*>> clockIdToInstanceStatePtrMap;
	std::list<InstanceState> instanceStates;

	const ui maxNoExpParticles;

	const db expParticleAttractionDelay = 1000.0;
	const ft expParticleEntropySpeed = 0.002f;
	const ft expParticleAttractionSpeed = 0.1f;
};

#endif