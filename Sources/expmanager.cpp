#include "expmanager.h"

ExpManager::ExpManager(Shader &shader, Camera &camera, Timer &timer, const UntexturedMeshParams &expMeshParams, 
	const ui customRandSeed, const ui maxNoExpParticles)
	: timer(timer), shader(shader), camera(camera), expMesh(expMeshParams, maxNoExpParticles), 
	customRand(customRandSeed), maxNoExpParticles(maxNoExpParticles)
{
}

void ExpManager::UpdateExpParticles(const glm::mat4 &pcModel)
{
	std::list<ui>::const_iterator i = expParticleClusterClockIds.cbegin();
	std::list<ui>::const_iterator ie = expParticleClusterClockIds.cend();
	while (i != ie)
	{
		if(timer.HeapIsItTime(*i))
		{
			timer.DestroyHeapClock(*i);
			expParticleClusterClockIds.erase(i);
			for (InstanceState * state : clockIdToInstanceStatePtrMap.at(*i))
				state->behaviour = ExpPartcleBehaviour::ATTRACTION;
			clockIdToInstanceStatePtrMap.erase(*i);
			/*if(!std::distance(ie, expParticleClusterClockIds.cend())) <--------- Not nescessery maybe untill it is 
			cause theoretically only one clock can go off per frame but also if last element is erased weird stuff happens*/
			break;
		}
		else
			++i;
	}

	const glm::vec2 pcPos = glm::vec2(pcModel * glm::vec4(0,0,0,1));
	std::list<InstanceState>::iterator j = instanceStates.begin();
	std::list<InstanceState>::iterator je = instanceStates.end();
	while (j != instanceStates.end())
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
			bool doDespawnParticle = glm::length(vecToPc) < timer.Scale(expParticleAttractionSpeed);
			if(doDespawnParticle)
			{
				instanceStates.erase(j);
				if(!std::distance(je, instanceStates.end()))
					break;
			}
			helpers::scale2dVec(vecToPc, timer.Scale(expParticleAttractionSpeed));
			const glm::mat4 localTranform = glm::translate(glm::vec3(vecToPc, 0));
			j->transform *= localTranform;
			++j;
		}
		else
			throw std::runtime_error("ERRORR:EXP_MANAGER: Wrong behaviour for the instance state");
	}

	const ui noInstances = instanceStates.size();
	std::vector<glm::mat4> instanceTransforms;
	instanceTransforms.reserve(noInstances);
	std::transform(instanceStates.begin(), instanceStates.end(), std::back_inserter(instanceTransforms), 
		[](InstanceState &state){ return state.transform; });
	
	shader.Bind();
	shader.Update(transform, camera);
	expMesh.SetInstanceCount(noInstances);
	expMesh.Update(&instanceTransforms[0], expMesh.InstancedBufferPosition());
	expMesh.Draw();
}

void ExpManager::CreateExpParticles(const glm::mat4 &originModel, const ui noParticles)
{
	if(instanceStates.size() < maxNoExpParticles)
	{
		ui heapClockId;
		timer.InitHeapClock(heapClockId, expParticleAttractionDelay);
		expParticleClusterClockIds.push_back(heapClockId);
	
		for (ui i = 0; i < noParticles; ++i)
		{
			if(instanceStates.size() == maxNoExpParticles)
				break;
			instanceStates.push_back({
				glm::translate(glm::vec3(helpers::randomDirVector(customRand, timer.Scale(expParticleEntropySpeed)), 0.0f)),
				originModel,
				ExpPartcleBehaviour::ENTROPY,
				heapClockId
			});
			if(!clockIdToInstanceStatePtrMap.count(heapClockId))
				clockIdToInstanceStatePtrMap.insert(std::pair(heapClockId, std::vector<InstanceState *>{&instanceStates.back()}));
			else
				clockIdToInstanceStatePtrMap.at(heapClockId).push_back(&instanceStates.back());

		}
	}
}

void ExpManager::Reset()
{
	for(ui clockId :  expParticleClusterClockIds)
		timer.DestroyHeapClock(clockId);
	expParticleClusterClockIds.clear();
	clockIdToInstanceStatePtrMap.clear();
	instanceStates.clear();
	currentExp = 0;
}
