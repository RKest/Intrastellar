#include "_config.h"

#include "Core/controler.h"
#include "player_character.h"
#include "exp_manager.h"
#include "weapons.h"
#include "enemy.h"
#include "card_deck.h"

#include "glm/gtx/string_cast.hpp"

int main()
{
	Text	::Construct("./Resources/Fonts/slkscr.ttf");
	Camera	::Construct(glm::vec3(0.0f, 0.0f, -CAMERA_DISTANCE), 70.0f, SCREEN_ASPECT, 0.01f, 1000.0f);

	const glm::vec3 expBarVertices[] = {{0,0,0}, {0,1,0}, {1,0,0}, {1,1,0}};
	const ui expBarIndices[] = {0, 2, 1, 2, 3, 1};
	const glm::vec3 weaponIconVertices[] = {{WEAPONS_ICON_DIMS, 0, 0}, {WEAPONS_ICON_DIMS, WEAPONS_ICON_DIMS, 0}, {0, WEAPONS_ICON_DIMS, 0}, {0, 0, 0}};
	const glm::vec2 weaponIconTexcoords[] = {{1, 1}, {1, 0}, {0, 0}, {0, 1}};
	const ui weaponIconIndices[] = {2, 1, 0, 0, 3, 2};

	MESH_PARAMS_FROM_PATH("./Resources/OBJs/player-character.obj", 		pcParams);
	MESH_PARAMS_FROM_PATH("./Resources/OBJs/blaster-projectile.obj", 	blasterProjParams);
	MESH_PARAMS_FROM_PATH("./Resources/OBJs/rocket-projectile.obj", 	rocketProjParams);
	MESH_PARAMS_FROM_PATH("./Resources/OBJs/exp-particle.obj", 			expParams);
	MESH_PARAMS_FROM_PATH("./Resources/OBJs/overlay.obj", 				overlayParams);
	MESH_PARAMS_FROM_PATH("./Resources/OBJs/enemy.obj", 				enemyMeshParams);
	MESH_PARAMS_FROM_PATH("./Resources/OBJs/card-border-1.obj", 		cardBorderParams);
	const UntexturedMeshParams expBarParams 	= {expBarVertices, expBarIndices, ARR_SIZE(expBarVertices), ARR_SIZE(expBarIndices)};
	const TexturedMeshParams weaponIconParams 	= {weaponIconVertices, weaponIconTexcoords, weaponIconIndices, ARR_SIZE(weaponIconVertices), ARR_SIZE(weaponIconIndices)};

	EnemyStats enemyStats = defaultEnemyStats;
	ExpManager expManager			(expParams, expBarParams);
	PlayerCharacter playerCharacter	(pcParams);
	WeaponsManager weaponsManager	(weaponIconParams, overlayParams, blasterProjParams, rocketProjParams);
	EnemyManager enemyManager		(enemyMeshParams, enemyStats, blasterProjParams);
	Controler controler				(IPlayerCharacter::Transform());
	CardDeck cardDeck				(overlayParams, cardBorderParams);

	const auto render = [&]
	{
		IDisplay::I.Clear(0.1f, 0.1f, 0.2f, 1.0f);
		enemyManager.Draw();
		playerCharacter.Draw();
		weaponsManager.Draw();
		Timer::RenderFPS();
		playerCharacter.RenderScore();
	};

	while (!IDisplay::I.IsClosed())
	{
		Timer::RecordFrame();
		glm::mat4 pcModel = IPlayerCharacter::Transform().Model();
		controler.CaptureKeyboardPresses(playerCharacter.IsAlive());
		controler.CaptureMouseMovement();
		Camera::Recalc();

		enemyManager.Spawn();
		weaponsManager.Update(pcModel, enemyManager.InstanceTransforms());
		playerCharacter.Update();
		if(expManager.HasThereBeenLevelUp())
		{
			g_playerStats.enemySpawnRate *= 0.5f;
			cardDeck.RollCards();
		}
		enemyManager.UpdateBehaviour(helpers::transformStdVector(pcParams, pcModel));
		render();
		expManager.UpdateExpParticles(pcModel);
		IDisplay::I.Update();

		while (cardDeck.AreCarsDrawn() && !IDisplay::I.IsClosed())
		{
			Timer::RecordFrame();
			render();
			cardDeck.DrawCards();
			IDisplay::I.Update();
		}

		usleep(10000);

		while (!playerCharacter.IsAlive() && !IDisplay::I.IsClosed())
		{
			Timer::RecordFrame();
			render();

			Text::Render("GAME OVER!", (static_cast<ft>(SCREEN_WIDTH) / 2.0f) - 300.0f, (static_cast<ft>(SCREEN_HEIGHT) / 2.0f), 2.0f, glm::vec3(1));
			Text::Render("Space to restart", (static_cast<ft>(SCREEN_WIDTH) / 2.0f) - 120.0f, (static_cast<ft>(SCREEN_HEIGHT) / 2.0f) - 60.0f, 0.5f, glm::vec3(1));

			IDisplay::I.Update();
			controler.CaptureKeyboardPresses(playerCharacter.IsAlive());

			if (playerCharacter.IsAlive())
			{
				expManager.Reset();
				enemyManager.Reset();
				weaponsManager.Reset();
				playerCharacter.Reset();
				IPlayerCharacter::Transform().Pos().x = 0;
				IPlayerCharacter::Transform().Pos().y = 0;
				Camera::Pos().x = 0;
				Camera::Pos().y = 0;
			}
		}
	}
	IDisplay::I.Destruct();

	return 0;
}

/*

#include <iostream>
#include <array>
#include <utility>

template <typename T, typename Cont, std::size_t ...I>
void f(T first, Cont *cont, std::index_sequence<I...>)
{
    std::cout << std::forward<Cont>(cont[I]...) << '\n';
}


int main()
{
    int a[3] = {1,2,3};
    f(1, a);
}
https://www.py4u.net/discuss/78588
*/

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

(id_wypożyczalni, 	 ulica_wypożyczalni, numer_budynku, 	   miejscowość_wypożyczalni, kod_pocztowy_wypożyczalni		)
(id_reżysera, 		 imie_reżysera,      nazwisko_reżysera																)
(id_filmu, 		  	 id_wypożyczalni, 	 id_reżysera,		   tytuł_filmu												)
(id_wypożyczającego, id_wypożyczalni, 	 imie_wypożyczającego, nazwisko_wypożyczającego, wypożyczenia_wypożyczającego	)
(id_wypożyczenia, 	 id_wypożyczalni, 	 id_filmu, 			   id_wypożyczającego, 		 data_wypożyczenia				)


T: SQL

Podzbiory SQL:
	SQL DML: Data Manipulation Language	(INSERT, UPDATE, DELETE)
	SQL DDL: Data Definition Language 	(CREATE, DROP, ALTER)
	SQL DCL: Data Control Language 		(GRANT, REVOKE)
	SQL DQL: Data Query Language		(SELECT)

SQL Funkcje agregujące:
	- COUNT
	- SUM
	- AVG
	- MAX
	- MIN
np. SELECT COUNT(nazwa_kolumny) FROM nazwa_tabeli

https://arkusze.pl/maturalne/informatyka-2021-maj-matura-rozszerzona-2.pdf

6.1 SELECT DISTINCT kraj, COUNT(*) FROM gracze WHERE data_dolaczenia LIKE "2018%" GROUP BY kraj ORDER BY 2 ASC LIMIT 5;
6.2 SELECT SUM(strzal) FROM klasy WHERE nazwa LIKE "%elf%" GROUP BY nazwa;
6.3 SELECT id_gracza FROM jednostki WHERE nazwa NOT LIKE "artlerzysta" ORDER BY ASC;
6.4 SELECT DISTINCT nazwa, COUNT(*) FROM jednostki JOIN klasy ON klasy.nazwa=jednostki.nazwa AND ( ABS(lok_x - 100) + ABS(lok_y - 100) <= szybkosc );

*/
