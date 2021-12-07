#include "game.hpp"
#include "constants.hpp"
#include "log.hpp"

#include <random>
#include <ctime>

using namespace std;
using namespace hlt;

const float THRESHOLD = 0.2;




MapCell MaxQuartile(GameMap map, Position* base, float threshold) {

	MapCell* max = nullptr;
	int xbase;
	int ybase;
	if (base->x < map.width / 2)
	{
		xbase = 0;

	}
	else
	{
		xbase = (int)(map.width / 2);
	}
	if (base->y < map.height / 2)
	{
		ybase = 0;
	}
	else
	{
		ybase = (int)(map.height / 2);
	}

	max = &map.cells[xbase][ybase];
	for (int i = xbase; i < map.width; i++)
	{
		for (int j = ybase; j < map.height; j++)
		{
			if (map.cells[i][j].halite >= max->halite)
			{
				max = &map.cells[i][j];
			}
		}
	}
	log::log("max x: " + to_string(max->position.x) + ", y : " + to_string(max->position.y) + ", halite:" + to_string(max->halite));



	return *max;

}


bool IsCellEnough(int maxHalite, MapCell cell) {
	return cell.halite * THRESHOLD <= maxHalite;
}

int main(int argc, char* argv[]) {
	unsigned int rng_seed;
	if (argc > 1) {
		rng_seed = static_cast<unsigned int>(stoul(argv[1]));
	}
	else {
		rng_seed = static_cast<unsigned int>(time(nullptr));
	}
	mt19937 rng(rng_seed);

	Game game;
	int wait = 2;


	// At this point "game" variable is populated with initial map data.
	// This is a good place to do computationally expensive start-up pre-processing.
	// As soon as you call "ready" function below, the 2 second per turn timer will start.
	game.ready("Myrite");

	log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");


	for (;;) {
		game.update_frame();
		shared_ptr<Player> me = game.me;
		unique_ptr<GameMap>& game_map = game.game_map;


		vector<Command> command_queue;
		MapCell max = MaxQuartile(*game_map.get(), &me->shipyard.get()->position, 0.2);
		for (const auto& ship_iterator : me->ships) {



			shared_ptr<Ship> ship = ship_iterator.second;
			int rd = rng() % 2;

			if (ship->halite < constants::MAX_HALITE * 0.7)
			{
				if (game_map->at(ship)->halite > max.halite * 0.2 || game_map->at(ship)->halite * 0.10 > ship->halite)
				{
					command_queue.push_back(ship->stay_still());
					break;
				}
				bool assigned = false;
				for (const auto& surroundings : ship->position.get_surrounding_cardinals())
				{
					if (game_map->at(surroundings)->halite * 0.10 > ship->halite)
					{
						assigned = true;
						if (ship->position.x < surroundings.x && !game_map->at(Position(ship->position.x + 1, ship->position.y))->is_occupied())
							command_queue.push_back(ship->move(Direction::EAST));
						else if ((ship->position.x > surroundings.x && !game_map->at(Position(ship->position.x - 1, ship->position.y))->is_occupied()))
							command_queue.push_back(ship->move(Direction::WEST));
						else if ((ship->position.y < surroundings.y && !game_map->at(Position(ship->position.x, ship->position.y + 1))->is_occupied()))
							command_queue.push_back(ship->move(Direction::SOUTH));
						else if ((ship->position.y > surroundings.y && !game_map->at(Position(ship->position.x + 1, ship->position.y - 1))->is_occupied()))
							command_queue.push_back(ship->move(Direction::NORTH));
						break;
					}
				}
				if (assigned)
				{
					continue;
				}
				if (ship->position.x < max.position.x && !game_map->at(Position(ship->position.x + 1, ship->position.y))->is_occupied())
					command_queue.push_back(ship->move(Direction::EAST));
				else if ((ship->position.x > max.position.x && !game_map->at(Position(ship->position.x - 1, ship->position.y))->is_occupied()))
					command_queue.push_back(ship->move(Direction::WEST));
				else if ((ship->position.y < max.position.y && !game_map->at(Position(ship->position.x, ship->position.y + 1))->is_occupied()))
					command_queue.push_back(ship->move(Direction::SOUTH));
				else if ((ship->position.y > max.position.y && !game_map->at(Position(ship->position.x + 1, ship->position.y - 1))->is_occupied()))
					command_queue.push_back(ship->move(Direction::NORTH));
				else
					command_queue.push_back(ship->stay_still());
				
			}
			else
			{
				if (game_map->at(ship)->halite * 0.10 > ship->halite)
				{
					command_queue.push_back(ship->stay_still());
					break;
				}
				
				if (ship->position.x < me->shipyard->position.x && !game_map->at(Position(ship->position.x + 1, ship->position.y))->is_occupied())
					command_queue.push_back(ship->move(Direction::EAST));
				else if ((ship->position.x > me->shipyard->position.x && !game_map->at(Position(ship->position.x - 1, ship->position.y))->is_occupied()))
					command_queue.push_back(ship->move(Direction::WEST));
				else if ((ship->position.y < me->shipyard->position.y && !game_map->at(Position(ship->position.x , ship->position.y +1))->is_occupied()))
					command_queue.push_back(ship->move(Direction::SOUTH));
				else if ((ship->position.y > me->shipyard->position.y && !game_map->at(Position(ship->position.x , ship->position.y-1))->is_occupied()))
					command_queue.push_back(ship->move(Direction::NORTH));
				else
					command_queue.push_back(ship->stay_still());
			}



		}

		if (
			game.turn_number <= 200 &&
			me->halite >= constants::SHIP_COST &&
			!game_map->at(me->shipyard)->is_occupied() &&
			me->ships.size() < 4 && wait == 2)
		{
			command_queue.push_back(me->shipyard->spawn());
			wait = 0;
		}
		else if (wait < 2)
		{
			wait++;
		}

		if (!game.end_turn(command_queue)) {
			break;
		}
	}

	return 0;
}


