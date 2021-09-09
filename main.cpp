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
	helpers::Core core{camera, text, timer, playerStats};
	ExpManager expManager(core, expParams, expBarParams);
	EnemyManager enemyManager(enemyShader, core, enemyMeshParams);
	PlayerCharacter playerCharacter(core, pcParams, projectileParams);
	Controler controler(display, camera, timer, playerCharacter.PcTransform());
	CardDeck cardDeck(enemyShader, core, overlayParams, cardBorderParams, enemyMeshParams, playerCharacter.ExternDrawCb());

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
		playerCharacter.Update(enemyManager.InstanceTransforms());
		enemyManager.RecordCollisions(playerCharacter.ProjTransforms(), playerCharacter.ProjHitCb(), expManager.CreateExpParticlesCb());
		enemyManager.RecordPCIntersection(helpers::transformStdVector(pcParams, pcModel), playerCharacter.PCIntersectionCb());
		if(expManager.HasThereBeenLevelUp())
			cardDeck.RollCards();
			
		render();
		expManager.UpdateExpParticles(pcModel);
		display.Update();

		while (cardDeck.AreCarsDrawn() && !display.IsClosed())
		{
			timer.RecordFrame();
			render();
			cardDeck.DrawCards();
			display.Update();
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


/*
Temat: Normalizacja
Normalizacja to proces upraszczania bazy danych w taki sposób, aby osiągneła ona postać optymalną.
Normalizację wykonuje się na etapie projektowania modelu fizycznego danych, w celu unknięcia anomalii - błędów lub niespójności w bazie danych (w tym również redundancji).

	-----------------------------------------------------------
	| Nazwa 	 |	 Adres 		|     Artyków     	| Cena	  |		
	-----------------------------------------------------------
	| Jubilat	 | Ul. Polna 3	| Ketchup			| 3,15	  |
	| Jubliat	 | Ul. Polna 3  | Majonez			| 6,50	  |
	----------------------------------------------------------

W tabeli sklepy występuje;
	1. Redundancja danych np. adres sklepu
	2. Anomalia wprowadzania danych np. niemożliwe jest dodanie sklepu, jeśli nie wprowadzimu przynajmniej jednogeo produktu.
	3. Anomalia aktualizacji danych np. z chwilą przeniesienia sklepu do innej lokalizacji konieczne będzie zmodyfikowanie danych w wielu rekordach.
	4. Anomalia usuwania danych np. usunięcie produktów spowoduje usunięcie adresu oraz nazwy.

Pierwsza postać normalna 1NF
Każdy rekord dowolnej tabeli z bazy przechowuje informację o pojedynczym obiekcie, nie zawiera kolekcji, ma klucz główny, a jej komponenty sa wartościami atomowymi 
(niepodzielnymi). Pierwsza postac normalna nakłada na tabele bazy danych najważniejszy wymóg: table musi być relacją

	-----------------------------------------------------------
	| Klient	 | Zamównienie  | Zakupiony Towar     		  |		
	-----------------------------------------------------------
	| Jan		 | 1			| Bluza, pasek, spodnie		  |
	| Wojciech	 | 2			| Skarpetki, buty, koszula	  |
	| Grzegrz	 | 3			| Rękawice, spodnie			  |
	----------------------------------------------------------

Druga postać normalna 2NF
Każda tabela przechowuje dane dot. tylko jednej klasy obiektów.

	--------------------------------------------------------------------------
	| ID lekarza | ID Specjalizacji | Nazwa specjalizacji | Nazwisko lekarza |
	--------------------------------------------------------------------------
	| 1			 | 1				| Internista		 | Nowak			 |
	| 2			 | 1				| Internista		 | Luicz			 |
	| 3			 | 2				| Kardiolog		 	 | Burski			 |
	| 4			 | 3				| Neurolog			 | Kowalska			 |
	--------------------------------------------------------------------------

Trzecia postać normalna 3NF
Atrybuty wtóre (zależne od innych atrybutów) są zależne bezpośrednio i wyłącznie od klucza podstawowego tabeli tzn. atrybuty wtóren nie moga być zależne od innych 
atrybutów wtórnych

	----------------------------------------------------------------------------------
	| ID lekarza | Nazwisko		    | Gabinet			 | Numer telefonu do gabinetu|
	----------------------------------------------------------------------------------
	| 1			 | Nowak			| 13				 | 34769376			 		 |
	| 2			 | Luicz			| 24				 | 43603487			 		 |
	| 3			 | Burski			| 54			 	 | 98735698			 		 |
	| 4			 | Kowalska			| 53				 | 84379824			 		 |
	----------------------------------------------------------------------------------

Wypożyczalnia(id_filmu, tytuł filmu, reżyser, wypożyczający, adres, data wypożyczenia)
*/