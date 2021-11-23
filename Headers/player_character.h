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
	PlayerCharacter(const UntexturedMeshParams &pcParams);

	void Reset();
	void RenderScore();
	void Update();
	void Draw();

	inline IPlayerCharacter *Interface()  { return _pcInterface; };
	inline bool &IsAlive() 				  { return _isAlive; 	 }

	~PlayerCharacter();

private:
	friend struct IPlayerCharacter;
	Shader _pcShader{"Shaders/PC"};
	UntexturedMesh _pcMesh;
	TriBoundingBox _pcBoundingBox;
	Transform _pcTransform;
	IPlayerCharacter *_pcInterface;
	ftUni _pcAlphaValue{"alpha", 1.0f};

	bool _isAlive{true};
	bool _isInvincible{false};
	db _invincibilityDuration{400.0};

	Clock<> m_invincibilityClock;

	ui _enemiesShotCounter{};

	void _pcIntersection();
	constexpr ft _setAlpha(db remainingInvincibilityTime);
};

struct IPlayerCharacter
{
	IPlayerCharacter(PlayerCharacter *pcPtr) : _pcPtr(pcPtr) {};
	IPlayerCharacter(IPlayerCharacter *iPtr) : _pcPtr(iPtr->_pcPtr) {};
	auto &HitCb()			{ return pcHitCb;	 						}
	auto &Transform()		{ return _pcPtr->_pcTransform; 				}
	auto &BoundingBox()		{ return _pcPtr->_pcBoundingBox;			}

private:
	PlayerCharacter *_pcPtr;
	DECL_INST(pcHitCb,	   std::bind(&PlayerCharacter::_pcIntersection, _pcPtr));
};
