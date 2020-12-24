#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <windows.h>
#include "Dririx.h"

//ruin_object[0] - blocks, ruin_object[1] - monsters, ruin_object[2] - front, ruin_object[3] - gun, ruin_object[4] - man, ruin_object[5] - missile1
const std::vector<std::vector<char>> ruin_objects{ {char(176), char(177), char(178)}, {'0', '1', '2', '3'}, {char(205)}, {char(202)}, {char(84)}, {char(143)} }; 
const char bg = 32;
const int length = 20, height = 40;
const pt sides[] = { {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };

int distance(const int x, const int y, const int a, const int b) {
	return round(sqrt(pow(x - a, 2) + pow(y - b, 2)));
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

	int bl = front_line + 1, el = front_line + 1 + protection_thickness;
	for (int i = 0; i < length; ++i) {
		for (int j = bl; j < el; ++j){
			play_area[i][j].set(0, 2, 0);
		}
	}
}

void _move_pulls(std::vector<std::vector<pixel>>& play_area) {
	for (int i = 0; i < length; ++i) {
		for (int j = 0; j < height; ++j) {
			layer pix = play_area[i][j].get(1);
			
			if (pix.symbol == ruin_objects[5][0]) {

			}
		}
	}
}

int main() {
	std::vector<std::vector<pixel>> play_area(length, std::vector<pixel>(height, (&ruin_objects)));
	mk_protect(play_area, 1, 3);
	show(play_area);
}