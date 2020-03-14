#include <iostream>
#include <string>
#include <ncurses.h>
#include <cstdio>
#include <sys/wait.h>
#include "xjx.h"
#include "utils.h"
using namespace std;

const string& mcToName(uint mc) {
	static string empty = "";
	if(XJX::mcToName.count(mc) == 0) return empty;
	return XJX::mcToName[mc].first;
}

void printw(const string& s) {
	printw(s.c_str());
}

void printAll() {
	clear();

	printw("==== Buses ====\n");
	printw("Addr: " + to_string(XJX::address_bus) + '\n');
	printw("Data: " + to_string(XJX::data_bus) + '\n');

	printw("===== RAM =====\n");
	for(uint i = 0; i < 11; ++i) {
		uint rel = i + XJX::address_bus - 5;

		// print indicator
		if(i == 5) {
			printw("> ");
		} else {
			printw("  ");
		}

		// print value
		if(rel >= XJX::ram.size()) {
			printw("---\n");
		} else {
			printw(to_string(rel) + ": " + to_string(XJX::ram[rel]) + '\n');
		}
	}

	printw("===== CPU =====\n");
	printw("INS: " + to_string(XJX::ins) + '\n');
	printw("PC:  " + to_string(XJX::program_counter) + '\n');

	printw("== MicroCode ==\n");
	for(uint i = 0; i < 7; ++i) {
		uint rel = i + XJX::mc_addr - 3;

		// print indicator
		if(i == 3) {
			printw("> ");
		} else {
			printw("  ");
		}

		// print value
		if(rel >= XJX::microcode.size()) {
			printw("---\n");
		} else {
			string asm_val = "";
			for(auto it = XJX::asm_reference.begin(); it != XJX::asm_reference.end(); ++it) {
				if(it->second == rel) asm_val = to_string(it->first);
			}
			printw(to_string(rel) + ": "
				 + to_string(XJX::microcode[rel]) + ' '
				 + mcToName(XJX::microcode[rel]) + ' '
				 + (asm_val.empty() ? "" : "(ASM " + asm_val + ')') + '\n');
		}
	}

	printw("===== MUR =====\n");
	printw("Addr: " + to_string(XJX::mur[0]) + '\n');
	printw("MOP:  " + to_string(XJX::mur[1]) + ' ' + mcToName(XJX::mur[1]) + '\n');
	printw("Asm:  " + to_string(XJX::mur[2]) + '\n');

	printw("===== ALU =====\n");
	printw("Acc: " + to_string(XJX::acc) + '\n');

	refresh();
}

void printBlinkenlightsText() {
	mvprintw(0, 0, "Addr     ");
	mvprintw(1, 0, "Data     ");
	mvprintw(2, 0, "RAM      ");
	mvprintw(3, 0, "INS      ");
	mvprintw(4, 0, "PC       ");
	mvprintw(5, 0, "MCPOS    ");
	mvprintw(6, 0, "MC	     ");
	mvprintw(7, 0, "MUR-Addr ");
	mvprintw(8, 0, "MUR-MOP  ");
	mvprintw(9, 0, "MUR-Asm  ");
	mvprintw(10, 0, "ACC      ");
}

void printBinaryLights(uint num, int y) {
	mvprintw(y, 26, "     ");
	mvprintw(y, 26, "%u", num);

	uint lo = XJX::lo(num);
	uint hi = XJX::hi(num);

	for(int x = 24; x >= 15; --x) {
		int pair = lo % 2;
		attron(COLOR_PAIR(pair));
		mvaddch(y, x, 'O');
		attroff(COLOR_PAIR(pair));
		lo >>= 1;
	}

	for(int x = 13; x >= 9; --x) {
		int pair = hi % 2;
		attron(COLOR_PAIR(pair));
		mvaddch(y, x, 'O');
		attroff(COLOR_PAIR(pair));
		hi >>= 1;
	}
}

void printBlinkenlightsMain() {
	printBinaryLights(XJX::address_bus, 0);
	printBinaryLights(XJX::data_bus, 1);
	printBinaryLights(XJX::ram[XJX::address_bus], 2);
	printBinaryLights(XJX::ins, 3);
	printBinaryLights(XJX::program_counter, 4);
	printBinaryLights(XJX::mc_addr, 5);
	printBinaryLights(XJX::microcode[XJX::mc_addr], 6);
	printBinaryLights(XJX::mur[0], 7);
	printBinaryLights(XJX::mur[1], 8);
	printBinaryLights(XJX::mur[2], 9);
	printBinaryLights(XJX::acc, 10);
}

int main(int argc, char** argv) {
	// init ncurses
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);

	if(argc == 1) {
		if(!Utils::loadXJX("standard.xjx")) {
			cout << "standard.xjx not found!" << endl;
			exit(1);
		}
	} else if(argc == 4) {
		if(!Utils::loadXJX(argv[1])) {
			cout << argv[1] << " not found!" << endl;
			exit(1);
		}
		XJX::in_fifo_path = argv[2];
		XJX::out_fifo_path = argv[3];
		XJX::setupIO();
	} else {
		cout << "Invalid number of arguments!" << endl;
		exit(1);
	}

	bool running = true;
	int delay = 50;
	// start clock when loaded as module
	bool clock_running = argc != 1;
	if(argc == 4) {
		timeout(delay);
	}

	printBlinkenlightsText();
	while(running) {
//		printAll();
		printBlinkenlightsMain();
		int command = getch();
		if(command == '.' || command == ERR) {
			if(!XJX::execMOP()) running = false;
		} else if(command == 'q') running = false;
		else if(command == ' ') {
			do {
				if(!XJX::execMOP()) {
					running = false;
					break;
				}
			}
			while(XJX::mc_addr != 1);
		} else if(command == 'p') {
			clock_running = !clock_running;
			if(clock_running) timeout(delay);
			else timeout(-1);
		} else if(command == '+') {
			delay -= 50;
			timeout(delay);
		} else if(command == '-') {
			delay += 50;
			timeout(delay);
		}
	}

	endwin();
	printf("Waiting for children to die...\n");
	wait(nullptr);
	printf("Exit code: %u\n", XJX::lo(XJX::ins));
}
