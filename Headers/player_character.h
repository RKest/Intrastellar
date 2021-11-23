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

	inline bool &IsAlive() { return _isAlive; }

private:
	friend struct IPlayerCharacter;
	Shader 			_pcShader				{ "Shaders/PC" };
	UntexturedMesh 	_pcMesh;
	TriBoundingBox 	_pcBoundingBox;
	Transform 		_pcTransform;
	ftUni 			_pcAlphaValue			{ "alpha", 1.0f };

	bool 			_isAlive				{ true };
	bool 			_isInvincible			{ false };
	db 				_invincibilityDuration	{ 400.0 };
	Clock<> 		m_invincibilityClock;
	ui 				_enemiesShotCounter{};

	void 			_pcIntersection();
	constexpr ft 	_setAlpha(db remainingInvincibilityTime);
};

struct IPlayerCharacter
{
	static inline void Init(PlayerCharacter *pcPtr)
	{
		m_pcPtr = pcPtr;
		pcHitCb = []{ IPlayerCharacter::m_pcPtr->_pcIntersection(); };
	}
	static inline auto &HitCb()			{ return pcHitCb;	 			  }
	static inline auto &Transform()		{ return m_pcPtr->_pcTransform;   }
	static inline auto &BoundingBox()	{ return m_pcPtr->_pcBoundingBox; }

private:
	inline static PlayerCharacter *m_pcPtr;
	inline static std::function<void()> pcHitCb;
};
