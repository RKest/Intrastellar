#ifndef SHOOTER_H
#define SHOOTER_H

#include "Core/transform.h"
#include "Core/shader.h"
#include "Core/camera.h"
#include "Core/mesh.h"
#include "Core/text.h"
#include "Core/timer.h"
#include "Core/helpers.h"
#include "Core/stats.h"

#include <execution>
#include <algorithm>
#include <functional>


using namespace std::placeholders;

class PlayerCharacter
{
public:
	PlayerCharacter(helpers::Core &core, const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projectileParams);

	void Reset();
	void RenderScore();
	void Update();
	void Draw();
	void Shoot(const glm::mat4 &originTransform);

	inline Transform &PcTransform()  	  { return _pcTransform; }
	inline bool &IsAlive() 				  { return _isAlive; }
	inline auto &ProjTransforms() 	const { return _projInstanceTransforms; }
	inline auto ProjHitCb() 		 	  { return std::bind(&PlayerCharacter::_projHit, this, _1); }
	inline auto PCIntersectionCb() 	 	  { return std::bind(&PlayerCharacter::_pcIntersection, this); }
	inline auto ExternDrawCb() 		 	  { return std::bind(&PlayerCharacter::_externDraw, this, _1, _2, _3, _4, _5, _6); }

private:
	Shader _projectileShader;
	Shader _pcShader;
	Shader _pcCardShader;
	UntexturedMesh _pcMesh;
	UntexturedInstancedMesh _pcCardMesh;
	UntexturedInstancedMesh _projMesh;
	UntexturedInstancedMesh _projCardMesh;
	Camera &_camera;
	Text &_text;
	Timer &_timer;
    Stats &_pcStats;
	Transform _pcTransform;

	bool _isAlive = true;
	bool _isInvincible = false;
	db _invincibilityDuration = 400.0;
	ui _invincibilityClockId;

	std::vector<glm::mat4> _projInstanceTransforms;

	ui _oldestProjectileIndex = 0;
	ui _enemiesShotCounter = 0;

	void _projHit(const ui index);
	void _pcIntersection();
	void _externDraw(const std::vector<glm::mat4> &pcTransforms, std::vector<glm::mat4> &projTransforms, const std::vector<ui> &clockIds, 
	const std::vector<helpers::BoundingBox> &targetBoundingBoxes, const glm::mat4 &projection, ui &oldestProjIndex);

};

#endif