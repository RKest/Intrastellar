#include "exp_manager.h"

ExpManager::ExpManager(helpers::Core &core, const UntexturedMeshParams &expMeshParams, const UntexturedMeshParams &expBarMeshParams)
	: _customRand(CUSTOM_RAND_SEED), _expMesh(expMeshParams, MAX_EXP_PART_NO), _expBarMesh(expBarMeshParams)
{
}

void ExpManager::UpdateExpParticles(const glm::mat4 &pcModel)
{
	_hasThereBeenLevelUp = false;
	auto b = _expParticleClusterClockIds.begin();
	auto i = b;
	auto ie = _expParticleClusterClockIds.end();
	while (i != ie)
	{
		if(i->Inspect(i->ClockId()))
		{
			_expParticleClusterClockIds.erase(i);
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
			bool doDespawnParticle = glm::length(vecToPc) < Timer::Scale(_expParticleAttractionSpeed);
			if(doDespawnParticle)
			{
				_instanceStates.erase(j);
				_updateExpBar();
				if(++_currentExp == _nextLevelExpTreshold)
				{
					_hasThereBeenLevelUp = true;
					_prevLevelExpTreshold = _nextLevelExpTreshold;
					_nextLevelExpTreshold = decl_cast(_nextLevelExpTreshold, 
						decl_cast(_levelThresholdMultiplayer, _nextLevelExpTreshold) * _levelThresholdMultiplayer);
				}

				if(!std::distance(je, _instanceStates.end()))
					break;
			}
			const glm::vec2 scaledVecToPc = helpers::scale2dVec(vecToPc, decl_cast(scaledVecToPc.x, Timer::Scale(_expParticleAttractionSpeed)));
			const glm::mat4 localTranform = glm::translate(glm::vec3(scaledVecToPc, 0));
			j->transform *= localTranform;
			++j;
		}
		else
		{
			throw std::runtime_error("ERRORR:EXP_MANAGER: Wrong behaviour for the instance state");
		}
	}

	const size_t noInstances = _instanceStates.size();
	std::vector<glm::mat4> instanceTransforms;
	instanceTransforms.reserve(noInstances);
	std::transform(_instanceStates.begin(), _instanceStates.end(), std::back_inserter(instanceTransforms), 
		[](InstanceState &state){ return state.transform; });
	
	helpers::render(_expParticleShader, _expMesh, instanceTransforms.data(), noInstances, _blankTransform, Camera::ViewProjection());
	helpers::render(_expBarShader, _expBarMesh, _blankTransform, _expBarProjection);
}

void ExpManager::CreateExpParticles(const glm::mat4 &originModel, const ui noParticles)
{
	if(_instanceStates.size() < MAX_EXP_PART_NO)
	{
		_expParticleClusterClockIds.emplace_back(_expParticleAttractionDelay, [this](ui clockId){
			for (auto state : _clockIdToInstanceStatePtrMap.at(clockId))
				state->behaviour = ExpPartcleBehaviour::ATTRACTION;
		});
		const ui clockId = _expParticleClusterClockIds.back().ClockId();
		_clockIdToInstanceStatePtrMap.emplace(clockId, std::vector<InstanceState *>());
		for (ui i = 0; i < noParticles; ++i)
		{
			if(_instanceStates.size() == MAX_EXP_PART_NO)
				break;
			_instanceStates.push_back(initInstanceState(originModel));
			_clockIdToInstanceStatePtrMap.at(clockId).push_back(&_instanceStates.back());

		}
	}
}

void ExpManager::Reset()
{
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
	_expBarMesh.Update(_expBarPositions.data(), static_cast<ui>(_expBarPositions.size()));
}
