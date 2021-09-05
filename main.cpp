#include "_config.h"

#include "Core/controler.h"
#include "player_character.h"
#include "exp_manager.h"
#include "enemy.h"
#include "card_deck.h"

#include "glm/gtx/string_cast.hpp"

int main(int argc, char **argv)
{
	Display display(SCREEN_WIDTH, SCREEN_HEIGHT, "Intrastellar");
	Camera camera(glm::vec3(0, 0, -20), 70.0f, display.Aspect(), 0.01f, 1000.0f);

	Shader enemyShader("./Shaders/Enemy");

	const glm::vec3 pcVertices[] = {{0, 0.5, 0}, {-0.5, -0.5, 0}, {0.5, -0.5, 0}};
	const ui pcIndices[] = {2, 1, 0};
	const glm::vec3 projectileVertices[] = {{-0.1, 0, 0}, {0.1, 0, 0}, {-0.1, 0.3, 0}, {0.1, 0.3, 0}};
	const ui projectileIndices[] = {2, 1, 0, 3, 1, 2};
	const glm::vec3 enemyVertices[] = {{0.5, -0.5, 0}, {0.5, 0.5, 0}, {-0.5, 0.5, 0}, {-0.5, -0.5, 0}};
	const ui enemyIndices[] = {2, 1, 0, 0, 3, 2};
	const glm::vec3 expVertices[] = {{0.1, 0, 0}, {0.1, 0.1, 0}, {0, 0.1, 0}, {0, 0, 0}};
	const ui expIndices[] = {2, 1, 0, 0, 3, 2};
	const glm::vec3 overlayVertices[] = {{100, -100, 0}, {100, 100, 0}, {-100, 100, 0}, {-100, -100, 0}};
	const ui overlayIndices[] = {0, 1, 2, 2, 3, 0};
	const glm::vec3 expBarVertices[] = {{0,0,0}, {0,1,0}, {1,0,0}, {1,1,0}};
	const ui expBarIndices[] = {0, 2, 1, 2, 3, 1};

	const UntexturedMeshParams pcParams = {pcVertices, pcIndices, ARR_SIZE(pcVertices), ARR_SIZE(pcIndices)};
	const UntexturedMeshParams enemyMeshParams = {enemyVertices, enemyIndices, ARR_SIZE(enemyVertices), ARR_SIZE(enemyIndices)};
	const UntexturedMeshParams projectileParams = {projectileVertices, projectileIndices, ARR_SIZE(projectileVertices), ARR_SIZE(projectileIndices)};
	const UntexturedMeshParams expParams = {expVertices, expIndices, ARR_SIZE(expVertices), ARR_SIZE(expIndices)};
	const UntexturedMeshParams overlayParams = {overlayVertices, overlayIndices, ARR_SIZE(overlayVertices), ARR_SIZE(overlayIndices)};
	MESH_PARAMS_FROM_PATH("./Resources/OBJs/card-border-1.obj", cardBorderParams);
	const UntexturedMeshParams expBarParams = {expBarVertices, expBarIndices, ARR_SIZE(expBarVertices), ARR_SIZE(expBarIndices)};

	Stats playerStats = defaultStats;
	Text text("./Resources/Fonts/slkscr.ttf", SCREEN_WIDTH, SCREEN_HEIGHT);
	Timer timer(text, playerStats);
	ExpManager expManager(camera, timer, expParams, expBarParams, CUSTOM_RAND_SEED, 50);
	EnemyManager enemyManager(enemyShader, camera, timer, playerStats, enemyMeshParams, CUSTOM_RAND_SEED, MAX_NO_ENEMIES);
	PlayerCharacter playerCharacter(pcParams, projectileParams, playerStats, camera, text, timer);
	Controler controler(display, camera, timer, playerCharacter.PcTransform());
	CardDeck cardDeck(enemyShader, text, timer, playerStats, overlayParams, cardBorderParams, enemyMeshParams, playerCharacter.ExternDrawCb());

	ui counter = 1;

	const auto render = [&]
	{
		display.Clear(0.1, 0.1, 0.2, 1.0);
		enemyManager.Draw();
		playerCharacter.RenderScore();
		playerCharacter.Draw();
		timer.RenderFPS();
	};

	while (!display.IsClosed())
	{
		timer.RecordFrame();
		glm::mat4 pcModel = playerCharacter.PcTransform().Model();
		controler.CaptureKeyboardPresses(playerCharacter.IsAlive());
		controler.CaptureMouseMovement();

		enemyManager.UpdateBehaviour(pcModel);
		if (timer.IsItTime(Timer::ClocksEnum::SPAWN_CLOCK))
			enemyManager.Spawn(pcModel);

		if (timer.IsItTime(Timer::ClocksEnum::SHOT_CLOCK))
			playerCharacter.Shoot(pcModel);
		playerCharacter.Update();
		enemyManager.RecordCollisions(playerCharacter.ProjTransforms(), playerCharacter.ProjHitCallback(), expManager.CreateExpParticlesCb());
		enemyManager.RecordPCIntersection(helpers::transformStdVector(pcParams, pcModel), playerCharacter.PCHitCallback());

		render();
		expManager.UpdateExpParticles(pcModel);
		if(expManager.HasThereBeenLevelUp())
			cardDeck.RollCards();

		display.Update();
		counter++;

		while (cardDeck.AreCarsDrawn() && !display.IsClosed())
		{
			timer.RecordFrame();
			render();
			cardDeck.DrawCards();
			display.Update();
			counter++;
		}

		while (!playerCharacter.IsAlive() && !display.IsClosed())
		{
			timer.RecordFrame();
			render();

			text.Render("GAME OVER!", (static_cast<ft>(SCREEN_WIDTH) / 2.0f) - 300.0f, (static_cast<ft>(SCREEN_HEIGHT) / 2.0f), 2.0f, glm::vec3(1));
			text.Render("Space to restart", (static_cast<ft>(SCREEN_WIDTH) / 2.0f) - 120.0f, (static_cast<ft>(SCREEN_HEIGHT) / 2.0f) - 60.0f, 0.5f, glm::vec3(1));

			display.Update();
			controler.CaptureKeyboardPresses(playerCharacter.IsAlive());

			if (playerCharacter.IsAlive())
			{
				expManager.Reset();
				enemyManager.Reset();
				playerCharacter.Reset();
				playerCharacter.PcTransform().Pos().x = 0;
				playerCharacter.PcTransform().Pos().y = 0;
				camera.Pos().x = 0;
				camera.Pos().y = 0;
			}
		}
	}

	return 0;
}