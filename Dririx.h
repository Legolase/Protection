#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

void stopp(const std::string& message, const int i) {
	std::cout << message;
	system("pause >nul");
	if (i != 0)
		exit(0);
}

struct pt {
	int x, y;

	pt(): x(0), y(0){}
	pt(const int a, const int b): x(a), y(b) {}
	pt operator+(const pt& a) const {
		return pt(this->x + a.x, this->y + a.y);
	}
	pt operator-(const pt& a) const {
		return pt(this->x - a.x, this->y - a.y);
	}
	void operator=(const pt& a) {
		this->x = a.x;
		this->y = a.y;
	}
	bool operator==(const pt& a) const {
		return ((this->x == a.x) && (this->y == a.y)) ? true : false;
	}
	bool operator!=(const pt& a) const {
		return ((this->x != a.x) || (this->y != a.y)) ? true : false;
	}
};

struct layer {
	int strength, type;
	char symbol;

	layer() : symbol(32), strength(-1), type(-1) {}
	layer(const char c, const int i, const int t) : symbol(c), strength(i), type(t) {}
	void set(const char c, const int i, const int t) {
		symbol = c;
		strength = i;
		type = t;
	}
	void operator=(const layer& a) {
		this->symbol = a.symbol;
		this->strength = a.strength;
		this->type = a.type;
	}
};

class pixel {
private:
	char background = char(32);
	const int size_layers = 3;
	const std::vector<std::vector<char>>* ruins;
	bool empt;
	std::vector<layer> layers;
public:
	pixel() : layers(size_layers), empt(true), ruins(0) {}
	pixel(const std::vector<std::vector<char>>* ruin_blocks): layers(size_layers), empt(true) { ruins = ruin_blocks; }
	void assign(const std::vector<std::vector<char>>* ruin_blocks) { ruins = ruin_blocks; }
	void set(const size_t a, const size_t b, const size_t level) {
		if ((a < (int)(*ruins).size()) && (b < (*ruins)[a].size())) {
			if (level < size_layers) {
				layers[level].set((*ruins)[a][b], b, a);
				empt = ((*ruins)[a][b] == background) ? empt : false;
			}
			else
				stopp("set() -> layers[level] not exist", 1);
		}
		else
			stopp("set -> *ruins[" + std::to_string(a) + "][" + std::to_string(b) + "] not exist", 1);
	}
	void del(const size_t level) {
		if (level < size_layers) {
			layers[level] = layer();
			empt = true;
			for (int i = 0; i < size_layers; ++i)
				if (layers[i].symbol != background) {
					empt = false;
					break;
				}
		}
		else
			stopp("del() -> level(" + std::to_string(level) + ") not exist", 1);
	}
	void clear() {
		layers = std::vector<layer>(size_layers);
		empt = true;
	}
	layer get(const size_t i) const {
		if (i < size_layers)
			return layers[i];
		else
			stopp("get(i) -> i > layers.size()", 1);
	}
	char show() const {
		if (empt)
			return background;
		for (int i = size_layers - 1; i > -1; --i)
			if (layers[i].symbol != background)
				return layers[i].symbol;
		return layers[0].symbol;
	}
	void operator=(const pixel& a) {
		this->ruins = a.ruins;
		this->empt = a.empt;
		this->layers = a.layers;
	}
	void crush(const size_t level) {
		if (level < size_layers) {
			if (layers[level].strength > 0)
				set(layers[level].type, layers[level].strength - 1, level);
			else
				del(level);
		}
		else
			stopp("crush() -> level(" + std::to_string(level) + ") not exist", 1);
	}
	bool empty() { return empt; }
};