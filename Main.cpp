#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <windows.h>
#include <conio.h>
#include "Dririx.h"

//ruin_object[0] - blocks, ruin_object[1] - monsters, ruin_object[2] - front, ruin_object[3] - gun, ruin_object[4] - man, ruin_object[5] - missile1, ruin_object[6] - misssele2(explousion), ruin_objects[7] - explosion
const std::vector<std::vector<char>> ruin_objects{ {char(176), char(176), char(176), char(177), char(177), char(177), char(178), char(178), char(178)}, {char(175), char(143), 77}, {char(205)}, {char(207)}, {char(84)}, {char(249)}, {'A'}, {char(43), char(42), char(64)} };
const char bg = 32;
const int length = 20, height = 40, explRadius = 6;
const pt sides[] = { {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };

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

void mk_protect(std::vector<std::vector<pixel>>& play_area, const size_t front_line, const size_t protection_thickness) {
	for (int i = 0; i < length / 2; ++i)
		play_area[i][front_line].set(2, 0, 0);

	play_area[length / 2][front_line].set(3, 0, 0);
	play_area[length / 2][front_line - 1].set(4, 0, 0);

	for (int i = (length / 2) + 1; i < length; ++i)
		play_area[i][front_line].set(2, 0, 0);

	int bl = front_line + 1, el = front_line + 1 + protection_thickness, size_of_type = ruin_objects[0].size() - 1;
	for (int i = 0; i < length; ++i) {
		for (int j = bl; j < el; ++j) {
			play_area[i][j].set(0, size_of_type, 0);
		}
	}
}

int _move_pulls(std::vector<std::vector<pixel>>& play_area) {
	int counter = 0;
	for (int j = height - 1; j > -1; --j) {
		for (int i = 0; i < length; ++i) {
			layer pix = play_area[i][j].get(1);

			if (pix.type == 5) {
				++counter;

				pt step = pt(i, j) + sides[4];
				play_area[i][j].del(1);
				if (in_square(step, pt(length, height))) {
					layer pix_step = play_area[step.x][step.y].get(0);

					if (pix_step.type == 1) {
						play_area[step.x][step.y].crush(0);
						play_area[step.x][step.y].set(pix.type, pix.strength, 1);
						play_area[step.x][step.y].crush(1);
					}
					else
						play_area[step.x][step.y].set(pix.type, pix.strength, 1);
				}

			}
		}
	}
	return counter;
}

bool _move_monster(std::vector<std::vector<pixel>>& play_area, int &count_monsters) {
	count_monsters = 0;
	for (int i = 0; i < length; ++i) {
		for (int j = 0; j < height; ++j) {
			layer pix = play_area[i][j].get(0);

			if (pix.type == 1) {
				++count_monsters;

				pt step = pt(i, j) + sides[0];
				if (in_square(step, pt(length, height))) {
					layer pix_step = play_area[step.x][step.y].get(0);

					if (pix_step.type == -1) {
						play_area[i][j].del(0);
						play_area[step.x][step.y].set(pix.type, pix.strength, 0);
						
						pix_step = play_area[step.x][step.y].get(1);
						if (pix_step.type == 5) {
							play_area[step.x][step.y].crush(0);
							play_area[step.x][step.y].crush(1);
						}
						pix_step = play_area[step.x][step.y].get(2);
						if (pix_step.type == 6) {
							const int max_strength_exp = ruin_objects[7].size() - 1;
							pt start = step - pt(explRadius, explRadius), end = step + pt(explRadius + 1, explRadius + 1), square(length, height);
							for (int x = start.x; x < end.x; ++x)
								for (int y = start.y; y < end.y; ++y)
									if (in_square(pt(x, y), square) && (distance(step, pt(x, y)) <= explRadius)) {
										layer newl = play_area[x][y].get(0);
										if (newl.type == 1)
											play_area[x][y].del(0);
										play_area[x][y].set(7, max_strength_exp, 3);
									}
							play_area[step.x][step.y].crush(2);
						}
					}
					else if (pix_step.type == 1) {}
					else
						play_area[step.x][step.y].crush(0);
				}
				else {
					setcur(length / 2 - 4, height / 2);
					stopp("Game over", 0);
					return true;
				}
			}
		}
	}
	return false;
}

int _move_rakets(std::vector<std::vector<pixel>>& play_area) {
	int counter = 0;
	for (int j = height - 1; j > -1; --j) {
		for (int i = 0; i < length; ++i) {
			layer pix = play_area[i][j].get(2);

			if (pix.type == 6) {
				++counter;

				pt step = pt(i, j) + sides[4];
				play_area[i][j].del(2);
				if (in_square(step, pt(length, height))) {
					layer pix_step = play_area[step.x][step.y].get(0);

					if (pix_step.type == 1) {
						const int max_strength_exp = ruin_objects[7].size() - 1;
						pt start = step - pt(explRadius, explRadius), end = step + pt(explRadius+ 1, explRadius + 1), square(length, height);
						for (int x = start.x; x < end.x; ++x)
							for (int y = start.y; y < end.y; ++y)
								if (in_square(pt(x, y), square) && (distance(step, pt(x, y)) <= explRadius)) {
									layer newl = play_area[x][y].get(0);
									if (newl.type == 1)
										play_area[x][y].del(0);
									play_area[x][y].set(7, max_strength_exp, 3);
								}
						play_area[step.x][step.y].set(pix.type, pix.strength, 2);
						play_area[step.x][step.y].crush(2);
					}
					else
						play_area[step.x][step.y].set(pix.type, pix.strength, 2);
				}
			}
		}
	}
	return counter;
}

void _crush_expl(std::vector<std::vector<pixel>>& play_area) {
	for (int i = 0; i < length; ++i) {
		for (int j = 0; j < height; ++j) {
			layer pix = play_area[i][j].get(3);

			if (pix.type == 7)
				play_area[i][j].crush(3);
		}
	}
}

int main() {
	int speed = 75;
	std::vector<std::vector<pixel>> play_area(length, std::vector<pixel>(height, (&ruin_objects)));
	const int spd_pull_max = 1 * speed, spd_monster_max = 25 * speed, 
		spd_raket_max = 4 * speed, spd_expl_max = 25 * speed,
		persent_of_monsters = 4, front_line = 1, protect = 5;
	const int max_pulls = 3, max_monsters = 15, max_expl = 1;
	int pcounter = 0, mcounter = 0, rcounter = 0, ecounter = 0, gun_line = length / 2, count_pulls = 0, count_monsters = 0, count_rackets = 0;
	bool end_game = false;
	std::mt19937 gen{ std::random_device()() };
	std::uniform_int_distribution<int> dist(0, 99);

	int length_wave = 100, length_pause = 40, counter_waves = 0;
	bool wave = true;

	const int max_strength_pulls = (int)ruin_objects[5].size()-1, max_strength_rakets = (int)ruin_objects[6].size() - 1;

	mk_protect(play_area, front_line, protect);

	while (!end_game) {
		show(play_area);
		setcur(0, 0);

		if (++pcounter > spd_pull_max) {
			count_pulls = _move_pulls(play_area);
			pcounter = 0;
		}
		if (++mcounter > spd_monster_max) {
			end_game = _move_monster(play_area, count_monsters);
			mcounter = 0;
			if ((wave) && (count_monsters < max_monsters)) {
				for (int i = 0; i < length; ++i) {
					if ((play_area[i][height - 1].empty()) && (dist(gen) < persent_of_monsters)) {
						int distgen = dist(gen);
						if (distgen < 5) {
							play_area[i][height - 1].set(1, 2, 0);
						}
						else if (distgen < 25)
							play_area[i][height - 1].set(1, 1, 0);
						else
							play_area[i][height - 1].set(1, 0, 0);
					}
				}
			}

			if (wave) {
				if (--length_wave < 0) {
					length_wave = 100;
					wave = false;
				}
			}
			else {
				if (--length_pause < 0) {
					length_pause = 40;
					wave = true;
				}
			}
		}
		if (++rcounter > spd_raket_max) {
			rcounter = 0;
			count_rackets = _move_rakets(play_area);
		}
		if (++ecounter > spd_expl_max) {
			ecounter = 0;
			_crush_expl(play_area);
		}

		if (_kbhit()) {
			const char mv = _getch();
			layer pix = play_area[gun_line][front_line].get(0);
			if (((mv == 'a') || (mv == 'd')) && (pix.type == 3)) {
				pt step = pt(gun_line, front_line) + ((mv == 'a') ? sides[6] : sides[2]);
				if (in_square(step, pt(length, height))) {
					layer step_pix = play_area[step.x][step.y].get(0);
					if (step_pix.type == 2) {
						play_area[gun_line][front_line].set(2, 0, 0);
						play_area[gun_line][front_line - 1].del(0);
						gun_line = step.x;
						play_area[gun_line][front_line].set(3, 0, 0);
						play_area[gun_line][front_line - 1].set(4, 0, 0);
					}
				}
			}
			else if ((mv == 'n') && (pix.type == 3) && (count_pulls < max_pulls))
				play_area[gun_line][front_line].set(5, max_strength_pulls, 1);
			else if ((mv == 'm') && (pix.type == 3) && (count_rackets < max_expl))
				play_area[gun_line][front_line].set(6, max_strength_rakets, 2);
		}

	}
}