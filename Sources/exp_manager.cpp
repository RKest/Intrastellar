#include "exp_manager.h"

ExpManager::ExpManager(Camera &camera, Timer &timer, const UntexturedMeshParams &expMeshParams, 
	const UntexturedMeshParams &expBarMeshParams, const ui customRandSeed, const ui maxNoExpParticles)
	: _expParticleShader("./Shaders/Exp", UNIFORMS), _expBarShader("./Shaders/ExpBar", UNIFORMS), _timer(timer), _camera(camera), 
	_expMesh(expMeshParams, maxNoExpParticles), _customRand(customRandSeed), _maxNoExpParticles(maxNoExpParticles), _expBarMesh(expMeshParams)
{
}

void ExpManager::UpdateExpParticles(const glm::mat4 &pcModel)
{
	_hasThereBeenLevelUp = false;
	std::list<ui>::const_iterator i = _expParticleClusterClockIds.cbegin();
	std::list<ui>::const_iterator ie = _expParticleClusterClockIds.cend();
	while (i != ie)
	{
		if(_timer.HeapIsItTime(*i))
		{
			_timer.DestroyHeapClock(*i);
			_expParticleClusterClockIds.erase(i);
			for (InstanceState * state : _clockIdToInstanceStatePtrMap.at(*i))
				state->behaviour = ExpPartcleBehaviour::ATTRACTION;
			_clockIdToInstanceStatePtrMap.erase(*i);
			/*if(!std::distance(ie, expParticleClusterClockIds.cend())) <--------- Not nescessery maybe untill it is 
			cause theoretically only one clock can go off per frame but also if last element is erased weird stuff happens*/
			break;
		}
		else
			++i;
	}

	const glm::vec2 pcPos = glm::vec2(pcModel * glm::vec4(0,0,0,1));
	std::list<InstanceState>::iterator j = _instanceStates.begin();
	std::list<InstanceState>::iterator je = _instanceStates.end();
	while (j != _instanceStates.end())
	{
		if(j->behaviour == ExpPartcleBehaviour::ENTROPY)
		{
			j->transform *= j->randomEntropyDirection;
			++j;
		}
		else if (j->behaviour == ExpPartcleBehaviour::ATTRACTION)
		{
			const glm::vec2 particlePos = glm::vec2(j->transform * glm::vec4(0,0,0,1));
			glm::vec2 vecToPc = pcPos - particlePos;
			bool doDespawnParticle = glm::length(vecToPc) < _timer.Scale(_expParticleAttractionSpeed);
			if(doDespawnParticle)
			{
				_instanceStates.erase(j);
				_updateExpBar();
				if(++_currentExp == _nextLevelExpTreshold)
				{
					_hasThereBeenLevelUp = true;
					_prevLevelExpTreshold = _nextLevelExpTreshold;
					_nextLevelExpTreshold *= _levelThresholdMultiplayer;
				}

				if(!std::distance(je, _instanceStates.end()))
					break;
			}
			helpers::scale2dVec(vecToPc, _timer.Scale(_expParticleAttractionSpeed));
			const glm::mat4 localTranform = glm::translate(glm::vec3(vecToPc, 0));
			j->transform *= localTranform;
			++j;
		}
		else
			throw std::runtime_error("ERRORR:EXP_MANAGER: Wrong behaviour for the instance state");
	}

	const ui noInstances = _instanceStates.size();
	std::vector<glm::mat4> instanceTransforms;
	instanceTransforms.reserve(noInstances);
	std::transform(_instanceStates.begin(), _instanceStates.end(), std::back_inserter(instanceTransforms), 
		[](InstanceState &state){ return state.transform; });
	
	helpers::render(_expParticleShader, _expMesh, instanceTransforms.data(), noInstances, _blankTransform, _camera.ViewProjection());
	helpers::render(_expBarShader, _expBarMesh, _blankTransform, _expBarProjection);
}

void ExpManager::CreateExpParticles(const glm::mat4 &originModel, const ui noParticles)
{
	if(_instanceStates.size() < _maxNoExpParticles)
	{
		ui heapClockId;
		_timer.InitHeapClock(heapClockId, _expParticleAttractionDelay);
		_expParticleClusterClockIds.push_back(heapClockId);

		for (ui i = 0; i < noParticles; ++i)
		{
			if(_instanceStates.size() == _maxNoExpParticles)
				break;
			_instanceStates.push_back({
				glm::translate(glm::vec3(helpers::randomDirVector(_customRand, _timer.Scale(_expParticleEntropySpeed)), 0.0f)),
				originModel,
				ExpPartcleBehaviour::ENTROPY,
				heapClockId
			});
			if(!_clockIdToInstanceStatePtrMap.count(heapClockId))
				_clockIdToInstanceStatePtrMap.insert(std::pair(heapClockId, std::vector<InstanceState *>{&_instanceStates.back()}));
			else
				_clockIdToInstanceStatePtrMap.at(heapClockId).push_back(&_instanceStates.back());

		}
	}
}

void ExpManager::Reset()
{
	for(ui clockId :  _expParticleClusterClockIds)
		_timer.DestroyHeapClock(clockId);
	_expParticleClusterClockIds.clear();
	_clockIdToInstanceStatePtrMap.clear();
	_instanceStates.clear();
	_currentExp = 0;
	_nextLevelExpTreshold = 100;
	_prevLevelExpTreshold = 0;
}

void ExpManager::_updateExpBar()
{
	ui expBarWidth = helpers::squishedIntToScreenWidth(_prevLevelExpTreshold, _nextLevelExpTreshold, _currentExp);
	_expBarPositions[2] = glm::vec3(expBarWidth, 0, 0);
	_expBarPositions[3] = glm::vec3(expBarWidth, 1, 0);
	_expBarMesh.Update(_expBarPositions.data(), _expBarPositions.size());
}
