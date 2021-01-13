#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <windows.h>
#include <conio.h>
#include "Dririx.h"

const std::vector<std::vector<char>> ruin_objects{ 
	{char(176), char(177), char(178)}, //ruin_object[0] - blocks 
	{char(175), char(143), 77}, //ruin_objects[1] - monsters
	{char(196)}, //ruin_objects[2] - front
	{char(202)}, //ruin_objects[3] - gun
	{char(84)}, //ruin_objects[4] - man
	{char(249)}, //ruin_objects[5] - missile1
	{'A'}, //ruin_objects[6] - misssele2(explousion)
	{char(43), char(42), char(64)}, // ruin_objects[7] - explosion
	{char(254)} // ruin_objects[8] - misssele3(blocks)
};
const char bg = 32;
const int length = 30, height = 40, explRadius = 6;
const pt sides[] = { {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };

std::mt19937 gen{ std::random_device()() };
std::uniform_int_distribution<int> dist(0, 99);

void setcur(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

int distance(const pt a, const pt b) {
	return round(sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)));
}

bool in_square(const pt& point, const pt& size_square) {
	return ((point.x < size_square.x) && (point.x > -1) && (point.y < size_square.y) && (point.y > -1)) ? true : false;
}

void show(const std::vector<std::vector<pixel>>& play_area) {
	std::string res = "";
	for (int j = height - 1; j > -1; --j) {
		for (int i = 0; i < length; ++i) {
			res.push_back(play_area[i][j].show());
		}
		res += "|\n";
	}
	for (int i = 0; i < length + 1; ++i)
		res.push_back('=');
	std::cout << res;
}

void mk_protect(std::vector<std::vector<pixel>>& play_area, int from, int to) {
	const int mStBlocks = ruin_objects[0].size() - 1;
	for (int i = 0; i < length; ++i)
		for (int j = from; j < to; ++j)
			play_area[i][j].set(0, mStBlocks, 0);
}

void mk_start_front(std::vector<std::vector<pixel>>& play_area, const size_t front_line, const size_t protection_thickness) {
	for (int i = 0; i < length / 2; ++i)
		play_area[i][front_line].set(2, 0, 0);

	play_area[length / 2][front_line].set(3, 0, 0);

	for (int i = (length / 2) + 1; i < length; ++i)
		play_area[i][front_line].set(2, 0, 0);

	int bl = front_line + 1, el = front_line + 1 + protection_thickness, size_of_type = ruin_objects[0].size() - 1;
	for (int i = 0; i < length; ++i) {
		for (int j = bl; j < el; ++j) {
			play_area[i][j].set(0, size_of_type, 0);
		}
	}
}

void set_monsters(std::vector<std::vector<pixel>>& play_area, const int percent_appear, const std::vector<int>& percent_monst) {
	const int size_percent = percent_monst.size();
	if (size_percent == ruin_objects[1].size()) {
		for (int i = 0; i < length; ++i) {
			if ((play_area[i][height - 1].empty()) && (dist(gen) < percent_appear)) {
				int dstgn = dist(gen);
				for (int j = 0; j < size_percent; ++j) {
					if (dstgn < percent_monst[j]) {
						play_area[i][height - 1].set(1, (size_percent - j) - 1, 0);
						break;
					}
				}
			}
		}
	}
	else
		stopp("set_monsterst() -> percent.size != " + std::to_string(ruin_objects[1].size()), 1);
}

int _move_pulls(std::vector<std::vector<pixel>>& play_area) {
	const pt square(length, height);
	int counter = 0;
	for (int j = height - 1; j > -1; --j) {
		for (int i = 0; i < length; ++i) {
			layer pix = play_area[i][j].get(1);

			if (pix.type == 5) {
				++counter;

				pt step = pt(i, j) + sides[4];
				play_area[i][j].del(1);
				if (in_square(step, square)) {
					play_area[step.x][step.y].set(pix.type, pix.strength, 1);
					layer pix_step = play_area[step.x][step.y].get(0);

					if (pix_step.type == 1) {
						play_area[step.x][step.y].crush(0);
						play_area[step.x][step.y].crush(1);
						pix_step = play_area[step.x][step.y].get(0);
						if (pix_step.type == -1)
							play_area[step.x][step.y].set(7, 0, 4);
					}
				}

			}
		}
	}
	return counter;
}

void _set_explosion(std::vector<std::vector<pixel>>& play_area, const int radius, pt epicenter) {
	pt start = epicenter - pt(radius, radius), finish = epicenter + pt(radius, radius);
	const int max_strength_exp = ruin_objects[7].size() - 1;

	start.x = (start.x < 0) ? 0 : start.x;
	start.y = (start.y < 0) ? 0 : start.y;
	finish.x = (finish.x >= length) ? length - 1 : finish.x;
	finish.y = (finish.y >= height) ? height - 1 : finish.y;

	for (int i = start.x; i <= finish.x; ++i)
		for (int j = start.y; j <= finish.y; ++j)
			if (distance(pt(i, j), epicenter) <= radius) {
				layer ground = play_area[i][j].get(0);
				play_area[i][j].set(7, max_strength_exp, 4);

				if (ground.type == 1)
					play_area[i][j].del(0);
				else
					play_area[i][j].crush(0);
			}
}

void set_blocks(std::vector<std::vector<pixel>>& play_area, const int radius, pt epicenter) {
	pt start = epicenter - pt(radius, radius), finish = epicenter + pt(radius, radius);
	const int max_strength_blocks = ruin_objects[0].size() - 1;

	start.x = (start.x < 0) ? 0 : start.x;
	start.y = (start.y < 1) ? 1 : start.y;
	finish.x = (finish.x >= length) ? length - 1 : finish.x;
	finish.y = (finish.y >= (height-1)) ? height - 2 : finish.y;

	for (int i = start.x; i <= finish.x; ++i)
		for (int j = start.y; j <= finish.y; ++j)
			if (distance(pt(i, j), epicenter) <= radius) {
				layer ground = play_area[i][j].get(0);

				if ((ground.type == -1) || (ground.type == 0))
					play_area[i][j].set(0, max_strength_blocks, 0);
			}
}

int _move_monster(std::vector<std::vector<pixel>>& play_area, bool& end_game/*, pt& first_zombie*/) {
	const pt square(length, height);
	int count_monsters = 0;
	end_game = false;

	//bool get_f = false;

	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < length; ++i) {
			layer pix = play_area[i][j].get(0);

			if (pix.type == 1) {
				/*if (!get_f) {
					get_f = true;
					first_zombie = pt(i, j);
				}*/
				++count_monsters;

				pt step = pt(i, j) + sides[0];
				if (step.y >= 0) {
					bool go = false;
					layer pix_step = play_area[step.x][step.y].get(0);

					if (pix_step.type == -1)
						go = true;
					else {
						pt step_side = pt(i, j) + sides[2 + ((dist(gen) % 2) * 4)];
						pt step_ahead = step_side + sides[0];
						if ((step_side.x > -1) && (step_side.x < length)) {
							layer pix_side = play_area[step_side.x][step_side.y].get(0), pix_ahead = play_area[step_ahead.x][step_ahead.y].get(0);
							if ((pix_side.type == -1) && (pix_ahead.type == -1)) {
								go = true;
								step = step_side;
								pix_step = play_area[step.x][step.y].get(0);
							}
						}
					}

					if (go) {
						play_area[i][j].del(0);
						play_area[step.x][step.y].set(pix.type, pix.strength, 0);

						pix_step = play_area[step.x][step.y].get(1);
						if (pix_step.type == 5) {
							play_area[step.x][step.y].crush(0);
							play_area[step.x][step.y].crush(1);
							pix_step = play_area[step.x][step.y].get(0);
							if (pix_step.type == -1)
								play_area[step.x][step.y].set(7, 0, 4);
						}

						pix_step = play_area[step.x][step.y].get(2);
						if (pix_step.type == 8) {
							play_area[step.x][step.y].crush(2);
							set_blocks(play_area, explRadius, step);
						}

						pix_step = play_area[step.x][step.y].get(3);
						if (pix_step.type == 6) {
							_set_explosion(play_area, explRadius, step);
							play_area[step.x][step.y].crush(3);
						}
					}
					else if (pix_step.type == 1){}
					else
						play_area[step.x][step.y].crush(0);
				}
				else {
					end_game = true;
					return count_monsters;
				}
			}
		}
	}
	return count_monsters;
}

int _move_rakets(std::vector<std::vector<pixel>>& play_area) {
	int counter = 0;
	for (int j = height - 1; j > -1; --j) {
		for (int i = 0; i < length; ++i) {
			layer pix = play_area[i][j].get(3);

			if (pix.type == 6) {
				++counter;

				pt step = pt(i, j) + sides[4];
				play_area[i][j].del(3);
				if (in_square(step, pt(length, height))) {
					layer pix_step = play_area[step.x][step.y].get(0);

					play_area[step.x][step.y].set(pix.type, pix.strength, 3);
					if (pix_step.type == 1) {
						_set_explosion(play_area, explRadius, step);
						play_area[step.x][step.y].crush(3);
					}
				}
			}
		}
	}
	return counter;
}

void _crush_expl(std::vector<std::vector<pixel>>& play_area) {
	for (int i = 0; i < length; ++i) {
		for (int j = 0; j < height; ++j) {
			layer pix = play_area[i][j].get(4);

			if (pix.type == 7)
				play_area[i][j].crush(4);
		}
	}
}

int _move_blraket(std::vector<std::vector<pixel>>& play_area) {
	const pt square(length, height);
	int counter = 0;
	for (int j = height - 1; j > -1; --j) {
		for (int i = 0; i < length; ++i) {
			layer pix = play_area[i][j].get(2);

			if (pix.type == 8) {
				++counter;

				play_area[i][j].del(2);
				pt step = pt(i, j) + sides[4];
				if (in_square(step, square)) {
					play_area[step.x][step.y].set(pix.type, pix.strength, 2);

					layer pix_step = play_area[step.x][step.y].get(0);
					if (pix_step.type == 1) {
						play_area[step.x][step.y].crush(2);
						set_blocks(play_area, explRadius, step);
					}
				}
			}
		}
	}
	return counter;
}

void _move_gun(std::vector<std::vector<pixel>>& play_area, pt& location, bool side) {
	layer pix = play_area[location.x][location.y].get(0);

	if (pix.type == 3) {
		pt step = location + sides[2 + (side * 4)], square(length, height);
		
		if (in_square(step, square)) {
			pix = play_area[step.x][step.y].get(0);
			
			if (pix.type == 2) {
				const int max_stength_front = ruin_objects[2].size() - 1;
				play_area[location.x][location.y].set(2, max_stength_front, 0);
				play_area[step.x][step.y].set(3, 0, 0);
				location = step;
			}
		}
	}
}

int main() {
	int speed = 40;

	const int spd_pull = 1 * speed, spd_monster = 25 * speed, spd_raket = 4 * speed, spd_expl = 7 * speed, spd_blraket = 5 * speed, spd_robot = 5 * speed; // speeds
	
	const int max_pulls = 3, max_monsters = 10, max_raket  = 1, max_blraket = 1; // limits of movable objects

	const int front_line = 1, protect = 4; // additional start constants

	const int size_wave = 150, size_pause = 70, regeneration = 8;

	const int max_strength_pulls = (int)ruin_objects[5].size() - 1,
		max_strength_rakets = (int)ruin_objects[6].size() - 1,
		max_strength_blrakets = (int)ruin_objects[8].size() - 1;

	//pt first_zombie;
	while (true) {
		int pcounter = 3, mcounter = 0, rcounter = 0, ecounter = 0, blrcounter = 0, robcounter = 0,
			count_pulls = 0, count_monsters = 0, count_rackets = 0, count_blraket = 0; // [pmre'brk']counter of pace, after usual counters

		int gun_line = length / 2; //place of appearance gun by x side
		bool endgame = false;
		int counter_wave = size_wave, counter_pause = size_pause, counter_waves = 0;
		bool wave = true;

		std::vector<std::vector<pixel>> play_area(length, std::vector<pixel>(height, (&ruin_objects)));
		mk_start_front(play_area, front_line, protect);

		while (!endgame) {
			show(play_area);
			setcur(0, 0);

			if (++mcounter > spd_monster) {
				mcounter = 0;
				count_monsters = _move_monster(play_area, endgame/*, first_zombie*/);
				if (endgame) {
					setcur(length / 2 - 4, height / 2);
					stopp("Game over", 0);
					system("cls");
					break;
				}
				if ((wave) && (count_monsters < max_monsters)) {
					set_monsters(play_area, 4, { 5, 30, 100 });
				}

				if ((wave) && (--counter_wave < 0)) {
					counter_wave = 150;
					wave = false;
					++counter_waves;
				}
				else if (--counter_pause < 0) {
					counter_pause = 70;
					wave = true;
				}
			}
			if (++pcounter > spd_pull) {
				count_pulls = _move_pulls(play_area);
				pcounter = 0;
			}
			if (++ecounter > spd_expl) {
				ecounter = 0;
				_crush_expl(play_area);
			}
			if (++rcounter > spd_raket) {
				rcounter = 0;
				count_rackets = _move_rakets(play_area);
			}
			if (++blrcounter > spd_blraket) {
				blrcounter = 0;
				count_blraket = _move_blraket(play_area);
			}
			if (++robcounter > spd_robot) {
				robcounter = 0;
				layer pix = play_area[gun_line][front_line].get(0);
				if (pix.type == 3) {
					if ((GetKeyState('A') & 0x80) || (GetKeyState(VK_LEFT) & 0x80)) {
						pt loc(gun_line, front_line);
						_move_gun(play_area, loc, 1);
						gun_line = loc.x;
					}
					if ((GetKeyState('D') & 0x80) || (GetKeyState(VK_RIGHT) & 0x80)) {
						pt loc(gun_line, front_line);
						_move_gun(play_area, loc, 0);
						gun_line = loc.x;
					}
					if ((GetKeyState('B') & 0x80) && (count_pulls < max_pulls))
						play_area[gun_line][front_line].set(5, max_strength_pulls, 1);
					if ((GetKeyState('M') & 0x80) && (count_blraket < max_blraket))
						play_area[gun_line][front_line].set(8, max_strength_blrakets, 2);
					else if ((GetKeyState('N') & 0x80) && (count_rackets < max_raket))
						play_area[gun_line][front_line].set(6, max_strength_rakets, 3);
				}
			}

			if ((counter_waves > regeneration) && (count_monsters == 0)) {
				counter_waves = 0;
				mk_protect(play_area, 2, 6);
			}

		}
	}
}