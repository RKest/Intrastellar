#pragma once
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
#include <vector>


using namespace std::placeholders;

class PlayerCharacter
{
public:
	PlayerCharacter(helpers::Core &core, const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projectileParams);

	void Reset();
	void RenderScore();
	void Update(const std::vector<std::vector<glm::mat4>*> &enemyInstanceTransforms);
	void Draw();
	void Shoot(const glm::mat4 &originTransform);

	inline Transform &PcTransform()  	  { return _pcTransform; }
	inline bool &IsAlive() 				  { return _isAlive; }
	inline auto &ProjTransforms() 	const { return _projInstanceTransforms; }
	inline auto ProjHitCb() 		 	  { return std::bind(&PlayerCharacter::_projHit, this, _1); }
	inline auto PCIntersectionCb() 	 	  { return std::bind(&PlayerCharacter::_pcIntersection, this); }
	inline auto ExternDrawCb() 		 	  { return std::bind(&PlayerCharacter::_externDraw, this, _1, _2, _3, _4, _5, _6); }

private:
	Camera &_camera;
	Text &_text;
	Timer &_timer;
    PlayerStats &_pcStats;
	Shader _projectileShader{"Shaders/Projectile"};
	Shader _pcShader{"Shaders/PC"};
	Shader _pcCardShader{"Shaders/PC_Card"};
	UntexturedMesh _pcMesh;
	UntexturedInstancedMesh _pcCardMesh;
	UntexturedInstancedMesh _projMesh;
	UntexturedInstancedMesh _projCardMesh;
	Transform _pcTransform;
	ftUni _pcAlphaValue{"alpha", 1.0f};

	bool _isAlive{true};
	bool _isInvincible{false};
	db _invincibilityDuration{400.0};
	ui _invincibilityClockId;

	std::vector<glm::mat4> _projInstanceTransforms;

	ui _oldestProjectileIndex{};
	ui _enemiesShotCounter{};

	void _projHit(const ui index);
	void _pcIntersection();
	void _externDraw(const std::vector<glm::mat4> &pcTransforms, std::vector<glm::mat4> &projTransforms, const std::vector<ui> &clockIds, 
	const std::vector<helpers::BoundingBox> &targetBoundingBoxes, const glm::mat4 &projection, ui &oldestProjIndex);
	constexpr ft _setAlpha(db remainingInvincibilityTime);
	glm::mat4 _moveProj(const std::vector<std::vector<glm::mat4>*> &enemyInstanceTransforms, const glm::mat4 &projTransform) const;

};