#pragma once
#include "Core/shader.h"
#include "Core/camera.h"
#include "Core/mesh.h"
#include "Core/text.h"
#include "Core/timer.h"
#include "Core/helpers.h"
#include "Core/stats.h"
#include "Core/transform.h"
#include "Core/bounding_box.h"

#include <execution>
#include <algorithm>
#include <functional>
#include <vector>

using namespace std::placeholders;
struct IPlayerCharacter;

class PlayerCharacter
{
public:
	PlayerCharacter(helpers::Core &core, const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projectileParams);

	void Reset();
	void RenderScore();
	void Update(const std::vector<std::vector<glm::mat4>*> &enemyInstanceTransforms);
	void Draw();
	void Shoot();

	inline IPlayerCharacter *Interface()  { return _pcInterface; };
	inline bool &IsAlive() 				  { return _isAlive; 	 }

	~PlayerCharacter();

private:
	friend struct IPlayerCharacter;
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
	TriBoundingBox _pcBoundingBox;
	Transform _pcTransform;
	IPlayerCharacter *_pcInterface;
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
	const std::vector<ReqBoundingBox> &targetBoundingBoxes, const glm::mat4 &projection, ui &oldestProjIndex);
	constexpr ft _setAlpha(db remainingInvincibilityTime);
	glm::mat4 _moveProj(const std::vector<std::vector<glm::mat4>*> &enemyInstanceTransforms, const glm::mat4 &projTransform) const;

};

struct IPlayerCharacter
{
	IPlayerCharacter(PlayerCharacter *pcPtr) : _pcPtr(pcPtr) {};
	IPlayerCharacter(IPlayerCharacter *iPtr) : _pcPtr(iPtr->_pcPtr) {};
	auto &ProjHitCb()		{ return pcProjHitCb;						}
	auto &HitCb()			{ return pcHitCb;	 						}
	auto &ExternDraw()		{ return externDraw; 						}
	auto &Transform()		{ return _pcPtr->_pcTransform; 				}
	auto &BoundingBox()		{ return _pcPtr->_pcBoundingBox;			}
	auto &ProjTransforms()	{ return _pcPtr->_projInstanceTransforms; 	}

private:
	PlayerCharacter *_pcPtr;
	DECL_INST(pcProjHitCb, std::bind(&PlayerCharacter::_projHit, _pcPtr, _1));
	DECL_INST(pcHitCb,	   std::bind(&PlayerCharacter::_pcIntersection, _pcPtr));
	DECL_INST(externDraw,  std::bind(&PlayerCharacter::_externDraw, _pcPtr, _1, _2, _3, _4, _5, _6));
};
