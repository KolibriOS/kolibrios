/*

    KMatrix: Simulates moving matrix like in famous film.
    Copyright (C) 2021-2022  Vitaliy Krylov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Especially for memcpy()
#include <conio.h>
#include <sys/ksys.h>

#define bool unsigned char
#define true 1
#define false 0
#define nullptr 0
#define dword unsigned int

#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

enum BUTTONS {
	BTN_QUIT = 1
};

#define SYSFN04_FONT_W 8
#define SYSFN04_FONT_H 14

static inline void _ksys_set_input_mode(char mode) {
	asm_inline(
		"int $0x40"
		::"a"(66), "b"(1), "c"(mode)
	);
}

// Slightly edited implementation from glibc
unsigned int rand_r_seed = 0;
int rand_r() {
	unsigned int *seed = &rand_r_seed;
	unsigned int next = *seed;
	int result;

	next *= 1103515245;
	next += 12345;
	result = (unsigned int) (next / 65536) % 2048;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int) (next / 65536) % 1024;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int) (next / 65536) % 1024;

	*seed = next;

	return result;
}
//

//<locales>
/*
	STR - string;
	ERR - error;
	INST - instant (must be formatted;  mainly for alloc. errors when it's better not to use concat and e.g.);
*/

//all:
const char STR_PROGRAM_NAME[] = "KMatrix";

//en_US:
const char STR_PROGRAM_MOTD[] = "Matrix for KolibriOS";
const char STR_ERR_INST_CONCAT[] = "[KMatrix] concat(): No free RAM left, string corrupted.\n";
const char STR_ERR_INST_WRITEERRORF[] = "[KMatrix] writeErrorF(): Allocation error... | ";
const char STR_ERR_INST_QUEUE_QALLOCMAIN[] = "[KMatrix] Q_alloc(): Queue allocation failed.\n";
const char STR_ERR_INST_QUEUE_QALLOCDATA[] = "[KMatrix] Q_alloc(): Queue.data allocation failed.\n";
const char STR_ERR_INST_QUEUE_QPUSH[] = "[KMatrix] Q_push(): Data allocation failed.\n";
const char STR_ERR_QUEUE_QPOP[] = "Q_pop(): Wrong use.";
const char STR_ERR_QUEUE_QFRONT[] = "Q_front(): Wrong use.";
const char STR_ERR_CONSOLEINIT[] = "Unable to initialize console.";
const char STR_ERR_INST_MAIN_MROWALLOC[] = "[KMatrix] MatrixRow allocation error.\n";
const char STR_ERR_INST_MAIN_MROWSECALLOC[] = "[KMatrix] MatrixRow->sequence allocation error.\n";
const char STR_ERR_INST_MAIN_TCURALLOC[] = "[KMatrix] Failed to create transparent cursor.\n";
const char STR_ERR_MAIN_TCURLOAD[] = "Failed to load transparent cursor.";
const char STR_MSG_HELP_USAGE[] = "Usage: %s [OPTION]...\n";
const char* STR_MSG_HELP[] = {
	"Simulates moving matrix like in famous film.\n\n",

	"\t-F, --foreground\t<COLOR>\tset color of letters;\n",
	"\t\t\tavailable colors: black, red, green, brown,\n",
	"\t\t\tblue, purple, turqoise, white\n",
	"\t\t\tor (available only in graphic mode):\n",
	"\t\t\t<R,G,B>   For example: '-F 255,0,255'\n",
	"\t-B, --background\t<COLOR>\tset color of background;\n",
	"\t\t\tavailable colors: black, red, green, brown,\n",
	"\t\t\tblue, purple, turqoise, white\n",
	"\t\t\tor (available only in graphic mode):\n",
	"\t\t\t<R,G,B>   For example: '-B 255,0,255'\n",
	"\t-H, --highlight\t<COLOR>\tset color of lead symbols;\n",
	"\t\t\tavailable colors: black, red, green, brown,\n",
	"\t\t\tblue, purple, turqoise, white\n",
	"\t\t\tor (available only in graphic mode):\n",
	"\t\t\t<R,G,B>   For example: '-H 255,0,255'\n",
	"\t-S, --speed\t<VALUE>\tset speed of falling letters;\n",
	"\t\t\tit must be an integer and be more than 0;\n",
	"\t\t\tdetermines how many falls will be per second;\n",
	"\t\t\tnotice: formula of waiting is floor(100 / speed),\n",
	"\t\t\tit means that some speed values will give same looking\n",
	"\t@ss\t\trun this program as screensaver (normally you willn't need it)\n",
	"\t--ccopt\t\toptimize fg. color changing (will be faster but could be changed not fully)\n",
	"\t--shell\t\trun this program in shell\n",
	"\t--help\t\tdisplay this help and exit\n",
	"\t--version\t\toutput version information and exit\n\n",

	"If program launched in graphical mode, you can do this:\n",
	"\t[1]-[8] for color changing\n",
	"\t[~] -- switch what to change (FG/BG/HL)\n",
	"\t[+]/[-] for speed changing (there is 5 modes)\n",
	"\t[ESC] -- close this program\n\n",

	"Defaults when nothing has overriden:\n",
	"\t--foreground green\n",
	"\t--background black\n",
	"\t--highlight white\n",
	"\t--speed 10\n"
};
const char* STR_MSG_VERSION[] = {
	"KMatrix 1.2\n",
	"Copyright (C) 2021-2022 Vitaliy Krylov\n",
	"License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n",
	"This is free software: you are free to change and redistribute it.\n",
	"This program comes with ABSOLUTELY NO WARRANTY.\n\n",

	"It is in no way affiliated in any way with the movie \"The Matrix\", \"Warner Bros\" nor any of its affiliates in any way.\n\n",

	"Written by Vitaliy Krylov.\n"
};
const char STR_MSG_NOARG[] = "%s: option '%s' requires an argument\n";
const char STR_MSG_HELPADVICE[] = "Try '%s --help' for more information.\n";
const char STR_MSG_INVARG_COLOR[] = "%s: invalid color: '%s'\n";
const char STR_MSG_INVARG_SPEED[] = "%s: invalid speed: '%s'\n";
const char STR_MSG_UNRARG[] = "%s: unrecognized option '%s'\n";
const char STR_MSG_CON_RGB[] = "%s: it's not allowed to set RGB-color in console mode\n";
const char STR_MSG_CON_SS[] = "%s: it's not allowed to run in @ss mode in console mode\n";
//</locales>

char* STR_PROGRAM_TITLENAME; // Init in main()
const char ss_path[] = "/sys/@ss";

// Normally could be output to stderr
void writeError(const char* text) {
	_ksys_debug_puts(text);
}
//Formatted error output
void writeErrorF(const char* text) {
	int t_size = strlen(STR_PROGRAM_NAME)+5+strlen(text); // = 1+strlen(STR_PROGRAM_NAME)+2+strlen(text)+1+1
	char* t = malloc(t_size);
	if (!t) {
		_ksys_debug_puts(STR_ERR_INST_WRITEERRORF);
		_ksys_debug_puts(text);
		_ksys_debug_puts("\n");
		return;
	}
	t[t_size-1] = '\0';
	sprintf(t, "[%s] %s\n", STR_PROGRAM_NAME, text);
	_ksys_debug_puts(t);
	free(t);
}

// Do not forget to free it
// If no RAM left, it will try to return "\0". If not, then 0...
char* concat(const char* s1, const char* s2) {
	const size_t len1 = strlen(s1);
	const size_t len2 = strlen(s2);
	char* res = (char*)malloc(len1 + len2 + 1);
	if (!res) {
		writeError(STR_ERR_INST_CONCAT);
		res = (char*)malloc(1);
		if (res) res[0] = '\0';
	} else {
		memcpy(res, s1, len1);
		memcpy(res + len1, s2, len2 + 1);
	}
	return res;
}

//<Queue>
typedef struct {
	unsigned int queue_size;
	unsigned int sizeoftype;
	void* data;
	unsigned int data_maxsize;
	unsigned int data_begin;
	unsigned int data_size;
} Queue;

//queue_size = 0 if want dynamic size choosing
Queue* Q_alloc(unsigned int sizeoftype, unsigned int queue_size) {
	Queue* this = (Queue*)malloc(sizeof(Queue));
	if (!this) {
		writeError(STR_ERR_INST_QUEUE_QALLOCMAIN);
		return nullptr;
	}
	this->sizeoftype = sizeoftype;
	this->data_begin = 0;
	this->data_size = 0;
	if (queue_size) this->queue_size = queue_size;
	else this->queue_size = 0x8000 / sizeoftype;
	this->data_maxsize = this->queue_size;
	this->data = (void*)malloc(this->data_maxsize * sizeoftype);
	if (!this->data) {
		writeError(STR_ERR_INST_QUEUE_QALLOCDATA);
		free(this);
		return nullptr;
	}
	return this;
}

void Q_free(Queue* this) {
	free(this->data);
	free(this);
}

//$value -- it is a reference to what you want to add
void Q_push(Queue* this, const void* value) {
	if (this->data_size == this->data_maxsize) {
		void* olddata = this->data;
		this->data = (void*)malloc((this->data_maxsize + this->queue_size) * this->sizeoftype);
		if (!this->data) {
			writeError(STR_ERR_INST_QUEUE_QPUSH);
			this->data = olddata;
			return;
		}

		int data_i = 0;
		for (int i = this->data_begin; i < this->data_maxsize; ++i) memcpy(this->data + this->sizeoftype * (data_i++), olddata + this->sizeoftype * i, this->sizeoftype);
		for (int i = 0; i < this->data_begin; ++i) memcpy(this->data + this->sizeoftype * (data_i++), olddata + this->sizeoftype * i, this->sizeoftype);

		free(olddata);
		this->data_maxsize += this->queue_size;
		this->data_begin = 0;
	}
	if (this->data_begin + this->data_size < this->data_maxsize) memcpy(this->data + this->sizeoftype * (this->data_begin + this->data_size), value, this->sizeoftype);
	else memcpy(this->data + this->sizeoftype * (this->data_begin + this->data_size - this->data_maxsize), value, this->sizeoftype);
	++this->data_size;
}

void Q_pop(Queue* this, void* returnValue) {
	if (this->data_size > 0) {
		if (returnValue) memcpy(returnValue, this->data + this->sizeoftype * this->data_begin, this->sizeoftype);
		++this->data_begin;
		if (this->data_begin == this->data_maxsize)
			this->data_begin = 0;
		--this->data_size;
	} else writeErrorF(STR_ERR_QUEUE_QPOP);
}

//WARNING: Be sure not to pass nullptr
void Q_front(Queue* this, void* returnValue) {
	if (this->data_size > 0) memcpy(returnValue, this->data + this->sizeoftype * this->data_begin, this->sizeoftype);
	else writeErrorF(STR_ERR_QUEUE_QFRONT);
}

unsigned int Q_size(Queue* this) {
	return this->data_size;
}

//NOTICE: Doesn't free used memory. Will use it for next pushes.
void Q_clear(Queue* this) {
	this->data_begin = 0;
	this->data_size = 0;
}

//WARNING: Be sure not to pass nullptr
void Q_at(Queue* this, int i, void* returnValue) {
	if (this->data_begin + i < this->data_maxsize)
		memcpy(returnValue, this->data + this->sizeoftype * (this->data_begin + i), this->sizeoftype);
	else
		memcpy(returnValue, this->data + this->sizeoftype * (this->data_begin + i - this->data_maxsize), this->sizeoftype);
}
//</Queue>

void consoleInit() {
	if (con_init()) { // Init fail
		writeErrorF(STR_ERR_CONSOLEINIT);
		exit(2);
	}
	(*con_set_title)(STR_PROGRAM_TITLENAME);
}

void consoleExit() {
	(*con_exit)(0);
}

int getColorByID(int id) {
	switch (id) {
		case 0:
			return 0x000000;
		case 1:
			return 0x800000;
		case 2:
			return 0x008000;
		case 3:
			return 0x808000;
		case 4:
			return 0x000080;
		case 5:
			return 0x800080;
		case 6:
			return 0x008080;
		case 7:
			return 0xc0c0c0;
	}
}

//perc should be from 0 to 1
uint32_t getColorBetween(float perc, uint32_t col1, uint32_t col2) {
	uint8_t* col1c = (uint8_t*)&col1;// { a, r, g, b }
	uint8_t* col2c = (uint8_t*)&col2;
	uint32_t res = col1;
	uint8_t* resc = (uint8_t*)&res;
	for (int i = 0; i < 4; ++i) resc[i] += ((int)col2c[i]-(int)col1c[i])*perc;
	return res;
}

typedef struct {
	int pos;
	int length;
	int gradLen;
	char* sequence; // if first element is '\0' then its an empty row
} MatrixRow;

#include <time.h>

int main(int argc, char** argv) {
	char* t = concat(STR_PROGRAM_NAME, " -- ");
	STR_PROGRAM_TITLENAME = concat(t, STR_PROGRAM_MOTD);
	free(t);

	int fgcolor = 32, bgcolor = 40, hlcolor = 37, matrixDelay = 10;
	uint32_t fgcolorrgb = 0, bgcolorrgb = 0, hlcolorrgb = 0;
	bool runInShell = false;
	bool ssRun = false;
	bool ccopt = false;
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
			consoleInit();
			printf(STR_MSG_HELP_USAGE, argv[0]);
			for (int j = 0; j < sizeof(STR_MSG_HELP)/sizeof(STR_MSG_HELP[0]); ++j) puts(STR_MSG_HELP[j]);
			consoleExit();
			return 0;
		} else if (!strcmp(argv[i], "--version")) {
			consoleInit();
			for (int j = 0; j < sizeof(STR_MSG_VERSION)/sizeof(STR_MSG_VERSION[0]); ++j) puts(STR_MSG_VERSION[j]);
			consoleExit();
			return 0;
		} else if (!strcmp(argv[i], "--foreground") || !strcmp(argv[i], "-F")) {
			if (i + 1 < argc) {
				if (!strcmp(argv[i+1], "black")) {
					fgcolor = 30;
				} else if (!strcmp(argv[i+1], "red")) {
					fgcolor = 31;
				} else if (!strcmp(argv[i+1], "green")) {
					fgcolor = 32;
				} else if (!strcmp(argv[i+1], "brown")) {
					fgcolor = 33;
				} else if (!strcmp(argv[i+1], "blue")) {
					fgcolor = 34;
				} else if (!strcmp(argv[i+1], "purple")) {
					fgcolor = 35;
				} else if (!strcmp(argv[i+1], "turqoise")) {
					fgcolor = 36;
				} else if (!strcmp(argv[i+1], "white")) {
					fgcolor = 37;
				} else {
					uint32_t rValue = -1, gValue = -1, bValue = -1;
					if (sscanf(argv[i+1], "%d,%d,%d", &rValue, &gValue, &bValue) != 3 ||
							rValue < 0 || rValue > 255 ||
							gValue < 0 || gValue > 255 ||
							bValue < 0 || bValue > 255) {
						consoleInit();
						printf(STR_MSG_INVARG_COLOR, argv[0], argv[i+1]);
						consoleExit();
						return 1;
					}
					fgcolor = -1;
					fgcolorrgb = bValue | (gValue << 8) | (rValue << 16);
				}
				i += 1;
			} else {
				consoleInit();
				printf(STR_MSG_NOARG, argv[0], argv[i]);
				printf(STR_MSG_HELPADVICE, argv[0]);
				consoleExit();
				return 1;
			}
		} else if (!strcmp(argv[i], "--background") || !strcmp(argv[i], "-B")) {
			if (i + 1 < argc) {
				if (!strcmp(argv[i+1], "black")) {
					bgcolor = 40;
				} else if (!strcmp(argv[i+1], "red")) {
					bgcolor = 41;
				} else if (!strcmp(argv[i+1], "green")) {
					bgcolor = 42;
				} else if (!strcmp(argv[i+1], "brown")) {
					bgcolor = 43;
				} else if (!strcmp(argv[i+1], "blue")) {
					bgcolor = 44;
				} else if (!strcmp(argv[i+1], "purple")) {
					bgcolor = 45;
				} else if (!strcmp(argv[i+1], "turqoise")) {
					bgcolor = 46;
				} else if (!strcmp(argv[i+1], "white")) {
					bgcolor = 47;
				} else {
					uint32_t rValue = -1, gValue = -1, bValue = -1;
					if (sscanf(argv[i+1], "%d,%d,%d", &rValue, &gValue, &bValue) != 3 ||
							rValue < 0 || rValue > 255 ||
							gValue < 0 || gValue > 255 ||
							bValue < 0 || bValue > 255) {
						consoleInit();
						printf(STR_MSG_INVARG_COLOR, argv[0], argv[i+1]);
						consoleExit();
						return 1;
					}
					bgcolor = -1;
					bgcolorrgb = bValue | (gValue << 8) | (rValue << 16);
				}
				i += 1;
			} else {
				consoleInit();
				printf(STR_MSG_NOARG, argv[0], argv[i]);
				printf(STR_MSG_HELPADVICE, argv[0]);
				consoleExit();
				return 1;
			}
		} else if (!strcmp(argv[i], "--highlight") || !strcmp(argv[i], "-H")) {
			if (i + 1 < argc) {
				if (!strcmp(argv[i+1], "black")) {
					hlcolor = 30;
				} else if (!strcmp(argv[i+1], "red")) {
					hlcolor = 31;
				} else if (!strcmp(argv[i+1], "green")) {
					hlcolor = 32;
				} else if (!strcmp(argv[i+1], "brown")) {
					hlcolor = 33;
				} else if (!strcmp(argv[i+1], "blue")) {
					hlcolor = 34;
				} else if (!strcmp(argv[i+1], "purple")) {
					hlcolor = 35;
				} else if (!strcmp(argv[i+1], "turqoise")) {
					hlcolor = 36;
				} else if (!strcmp(argv[i+1], "white")) {
					hlcolor = 37;
				} else {
					uint32_t rValue = -1, gValue = -1, bValue = -1;
					if (sscanf(argv[i+1], "%d,%d,%d", &rValue, &gValue, &bValue) != 3 ||
							rValue < 0 || rValue > 255 ||
							gValue < 0 || gValue > 255 ||
							bValue < 0 || bValue > 255) {
						consoleInit();
						printf(STR_MSG_INVARG_COLOR, argv[0], argv[i+1]);
						consoleExit();
						return 1;
					}
					hlcolor = -1;
					hlcolorrgb = bValue | (gValue << 8) | (rValue << 16);
				}
				i += 1;
			} else {
				consoleInit();
				printf(STR_MSG_NOARG, argv[0], argv[i]);
				printf(STR_MSG_HELPADVICE, argv[0]);
				consoleExit();
				return 1;
			}
		} else if (!strcmp(argv[i], "--speed") || !strcmp(argv[i], "-S")) {
			if (i + 1 < argc) {
				int speedValue = 0;
				if (!sscanf(argv[i+1], "%d", &speedValue) || speedValue <= 0) {
					consoleInit();
					printf(STR_MSG_INVARG_SPEED, argv[0], argv[i+1]);
					consoleExit();
					return 1;
				}
				matrixDelay = 100 / speedValue;
				i += 1;
			} else {
				consoleInit();
				printf(STR_MSG_NOARG, argv[0], argv[i]);
				printf(STR_MSG_HELPADVICE, argv[0]);
				consoleExit();
				return 1;
			}
		} else if (!strcmp(argv[i], "--shell")) {
			runInShell = true;
		} else if (!strcmp(argv[i], "@ss")) {
			ssRun = true;
		} else if (!strcmp(argv[i], "--ccopt")) {
			ccopt = true;
		} else {
			consoleInit();
			printf(STR_MSG_UNRARG, argv[0], argv[i]);
			printf(STR_MSG_HELPADVICE, argv[0]);
			consoleExit();
			return 1;
		}
	}

	if (runInShell && (fgcolor == -1 || bgcolor == -1 || hlcolor == -1)) {
		consoleInit();
		printf(STR_MSG_CON_RGB, argv[0]);
		printf(STR_MSG_HELPADVICE, argv[0]);
		consoleExit();
		return 1;
	}

	if (runInShell && ssRun) {
		consoleInit();
		printf(STR_MSG_CON_SS, argv[0]);
		printf(STR_MSG_HELPADVICE, argv[0]);
		consoleExit();
		return 1;
	}

	int bakedTable_size = '~'-'!';
	char bakedTable[bakedTable_size];
	for (char c = '!'; c < '\\'; ++c) bakedTable[c-'!'] = c;
	for (char c = '\\'+1; c <= '~'; ++c) bakedTable[c-'!'-1] = c;

	rand_r_seed = time(NULL);

	int colorType = 0; // { FG = 0, BG = 1, HL = 2 }
	int delayTypesSize = 5;
	int delayTypes[] = { 33, 22, 10, 4, 1};
	int delayTypeIdx = -1;

	// I divide it in two parts because we don't want some additional operations in while(true)   (checkings if it's running in shell)
	if (runInShell) {
		consoleInit();
		int initCurHeight = (*con_get_cursor_height)();
		(*con_set_cursor_height)(0);
		printf("\033[%d;%dm", fgcolor, bgcolor);
		for (int i = 0; i < CONSOLE_WIDTH*CONSOLE_HEIGHT; ++i) putc(' ');
		bool lastWasHighlighted = false;

		Queue* q[CONSOLE_WIDTH/2];
		for (int x = 0; x < CONSOLE_WIDTH/2; ++x) q[x] = Q_alloc(sizeof(MatrixRow*), 0);
		MatrixRow* mr;
		while (true) {
			for (int x = 0; x < CONSOLE_WIDTH/2; ++x) {
				if (Q_size(q[x])) {
					Q_at(q[x], 0, &mr);
					if (mr->pos + 1 == CONSOLE_HEIGHT) {
						Q_pop(q[x], 0);
						free(mr->sequence);
 						free(mr);
					}
				}
				for (int i = 0; i < Q_size(q[x]); ++i) {
					Q_at(q[x], i, &mr);
					int idx = mr->pos + mr->length - 1;
					if (idx < CONSOLE_HEIGHT) {
						(*con_set_cursor_pos)(x*2, idx);
						char tc = (mr->sequence[0] == '\0') ? ' ' : mr->sequence[idx];
						if (!lastWasHighlighted) putc(tc);
						else { printf("\033[%dm%c", fgcolor, tc); lastWasHighlighted = 0; }
					}
					++(mr->pos);
					idx = mr->pos + mr->length - 1;
					if (idx < CONSOLE_HEIGHT) {
						(*con_set_cursor_pos)(x*2, idx);
						char tc = (mr->sequence[0] == '\0') ? ' ' : mr->sequence[idx];
						if (lastWasHighlighted) putc(tc);
						else { printf("\033[%dm%c", hlcolor, tc); lastWasHighlighted = 1; }
					}
				}

				bool needToAdd = !Q_size(q[x]);
				if (!needToAdd) {
					Q_at(q[x], Q_size(q[x]) - 1, &mr);
					needToAdd = (mr->pos > 0);
				}
				if (needToAdd) {
					bool mrseq0 = 0;
					if (Q_size(q[x])) mrseq0 = (mr->sequence[0] != '\0');
					mr = (MatrixRow*)malloc(sizeof(MatrixRow));
					if (!mr) {
						writeError(STR_ERR_INST_MAIN_MROWALLOC);
						continue;
					}
					mr->length = (rand_r() % CONSOLE_HEIGHT) + 1;
					mr->pos = 1 - mr->length;
					if (!mrseq0 && (rand_r() % 3) == 2) { // 33% chance of being row with symbols
						mr->sequence = (char*)malloc(CONSOLE_HEIGHT);
						if (!mr->sequence) {
							writeError(STR_ERR_INST_MAIN_MROWSECALLOC);
							free(mr);
							continue;
						}
						for (int i = 0; i < CONSOLE_HEIGHT; ++i)
							mr->sequence[i] = bakedTable[rand_r() % bakedTable_size];
					} else {
						mr->sequence = (char*)malloc(1);
						if (!mr->sequence) {
							writeError(STR_ERR_INST_MAIN_MROWSECALLOC);
							free(mr);
							continue;
						}
 						mr->sequence[0] = '\0';
					}
					Q_push(q[x], &mr);
					(*con_set_cursor_pos)(x*2, 0);
					char tc = (mr->sequence[0] == '\0') ? ' ' : mr->sequence[0];
					if (lastWasHighlighted) putc(tc);
					else { printf("\033[%dm%c", hlcolor, tc); lastWasHighlighted = 1; }
				}
			}

			_ksys_delay(matrixDelay);
		}

		//I know it will not be executed.. but it better be executed when something sends STOP-like call when there will be better console.obj and others
		printf("\033[0m"); // Turning BG and FG to normal
		(*con_set_cursor_height)(initCurHeight);
		consoleExit();
		return 0;
	} else {
		if (fgcolor != -1) fgcolorrgb = getColorByID(fgcolor % 10);
		if (bgcolor != -1) bgcolorrgb = getColorByID(bgcolor % 10);
		if (hlcolor != -1) hlcolorrgb = getColorByID(hlcolor % 10);

		ksys_pos_t screenSize = _ksys_screen_size(); // WARNING: If user changes screen resolution while this program is running, here probably could occur bug
		screenSize.x += 1; screenSize.y += 1; // Yep, that's really needed :/

		if (ssRun) _ksys_set_event_mask(0x27); // Enable mouse events
		_ksys_set_input_mode(1); // Enable scancode mode

		// Make the cursor be transparent
		char* transpCur = (char*)calloc(32*32, 1);
		if (!transpCur) writeError(STR_ERR_INST_MAIN_TCURALLOC);
		else {
			uint32_t loadedCurHandle = _ksys_load_cursor(transpCur, 2);
			free(transpCur);
			if (!loadedCurHandle) writeErrorF(STR_ERR_MAIN_TCURLOAD);
			else _ksys_set_cursor(loadedCurHandle);
		}

		int console_width = screenSize.x / SYSFN04_FONT_W;
		int console_height = screenSize.y / SYSFN04_FONT_H;

		char ss[2];
		ss[1] = '\0';

		Queue* q[console_width/2];
		for (int x = 0; x < console_width/2; ++x) q[x] = Q_alloc(sizeof(MatrixRow*), 0);
		MatrixRow* mr;

		int event;
		char needsRedrawing = 0; // { NO = 0, YES = 1, ONLY_TEXT = 2 }
		while (true) {
			while ((event = _ksys_check_event()) != KSYS_EVENT_NONE) {
				switch (event) {
					case KSYS_EVENT_REDRAW:
						_ksys_create_window(0, 0, screenSize.x, screenSize.y, STR_PROGRAM_TITLENAME, bgcolorrgb, 0x31);
						needsRedrawing = 1;
						break;
					case KSYS_EVENT_BUTTON:
						switch (_ksys_get_button()) {
							case BTN_QUIT:
								return 0;
						}
						break;
					case KSYS_EVENT_MOUSE:
						if (ssRun) {
							_ksys_exec(ss_path, "");
							return 0;
						}
						break;
					case KSYS_EVENT_KEY:
						if (ssRun) {
							_ksys_exec(ss_path, "");
							return 0;
						}
						ksys_oskey_t kc = _ksys_get_key();
						if (kc.code == 1) // [ESC]
							return 0;
						else if (kc.code >= 2 && kc.code <= 9) { // from [1] to [8]
							switch (colorType) {
								case 0: { // FG
									fgcolor = 30 + (kc.code - 2);
									uint32_t oldcolorrgb = fgcolorrgb;
									fgcolorrgb = getColorByID(kc.code - 2);
									if (oldcolorrgb != fgcolorrgb) {
										if (ccopt) { if (!needsRedrawing) needsRedrawing = 2; }
										else needsRedrawing = 1;
									}
									break;
								}
								case 1: { // BG
									bgcolor = 40 + (kc.code - 2);
									uint32_t oldcolorrgb = bgcolorrgb;
									bgcolorrgb = getColorByID(kc.code - 2);
									if (oldcolorrgb != bgcolorrgb) needsRedrawing = 1;
									break;
								}
								case 2: // HL
									hlcolor = 30 + (kc.code - 2);
									hlcolorrgb = getColorByID(kc.code - 2);
									break;
							}
						} else if (kc.code == 12 || kc.code == 13) { // [-] or [+]
							if (delayTypeIdx == -1) {
								delayTypeIdx = 0;
								while (delayTypeIdx < delayTypesSize && matrixDelay < delayTypes[delayTypeIdx]) ++delayTypeIdx;
								if (delayTypeIdx == delayTypesSize) --delayTypeIdx;
							}
							if (kc.code == 12) { // Decrease
								if (delayTypeIdx > 0) --delayTypeIdx;
							} else { // Increase
								if (delayTypeIdx + 1 < delayTypesSize) ++delayTypeIdx;
							}
							matrixDelay = delayTypes[delayTypeIdx];
						} else if (kc.code == 41) { // [~]
							colorType++;
							if (colorType == 3) colorType = 0;
						}
						break;
					default:
						break;
				}
			}

			if (needsRedrawing) {
				if (needsRedrawing == 1) _ksys_draw_bar(0, 0, screenSize.x, screenSize.y, bgcolorrgb);
				for (int x = 0; x < console_width/2; ++x) {
					for (int i = 0; i < Q_size(q[x]); ++i) {
						Q_at(q[x], i, &mr);
						if (mr->sequence[0] != '\0') {
							int y = mr->pos;
							if (y < 0) y = 0;
							for (; y < mr->pos + mr->gradLen && y < console_height; ++y) {
								ss[0] = mr->sequence[y];
								_ksys_draw_text(ss, x*2*SYSFN04_FONT_W, y*SYSFN04_FONT_H, 0, 0x90000000 | getColorBetween(((float)y - mr->pos + 1) / (mr->gradLen + 1), bgcolorrgb, fgcolorrgb));
							}
							for (; y < mr->pos + mr->length - 1 && y < console_height; ++y) {
								ss[0] = mr->sequence[y];
								_ksys_draw_text(ss, x*2*SYSFN04_FONT_W, y*SYSFN04_FONT_H, 0, 0x90000000 | fgcolorrgb);
							}
							if (y == mr->pos + mr->length - 1 && y < console_height) {
								ss[0] = mr->sequence[y];
								_ksys_draw_text(ss, x*2*SYSFN04_FONT_W, y*SYSFN04_FONT_H, 0, 0x90000000 | hlcolorrgb);
							}
						}
					}
				}
				needsRedrawing = 0;
			}

			for (int x = 0; x < console_width/2; ++x) {
				if (Q_size(q[x])) {
					Q_at(q[x], 0, &mr);
					if (mr->pos + 1 == console_height) {
						Q_pop(q[x], 0);
						free(mr->sequence);
						free(mr);
					}
				}
				for (int i = 0; i < Q_size(q[x]); ++i) {
					Q_at(q[x], i, &mr);
					++(mr->pos);
					int idx;
					if (mr->sequence[0] != '\0') {
						idx = mr->pos;
						if (idx < 0) idx = 0;
						for (; idx < mr->pos + mr->gradLen && idx < console_height; ++idx) {
							if (!ccopt) _ksys_draw_bar(x*2*SYSFN04_FONT_W, idx*SYSFN04_FONT_H, SYSFN04_FONT_W, SYSFN04_FONT_H, bgcolorrgb);
							ss[0] =  mr->sequence[idx];
							_ksys_draw_text(ss, x*2*SYSFN04_FONT_W, idx*SYSFN04_FONT_H, 0, 0x90000000 | getColorBetween(((float)idx - mr->pos + 1) / (mr->gradLen + 1), bgcolorrgb, fgcolorrgb));
	 					}
						idx = mr->pos + mr->length - 2;
						if (idx < console_height) {
							if (!ccopt) _ksys_draw_bar(x*2*SYSFN04_FONT_W, idx*SYSFN04_FONT_H, SYSFN04_FONT_W, SYSFN04_FONT_H, bgcolorrgb);
							ss[0] =  mr->sequence[idx];
							_ksys_draw_text(ss, x*2*SYSFN04_FONT_W, idx*SYSFN04_FONT_H, 0, 0x90000000 | fgcolorrgb);
	 					}
 					}
					idx = mr->pos + mr->length - 1;
					if (idx < console_height) {
						_ksys_draw_bar(x*2*SYSFN04_FONT_W, idx*SYSFN04_FONT_H, SYSFN04_FONT_W, SYSFN04_FONT_H, bgcolorrgb);
						if (mr->sequence[0] != '\0') {
							ss[0] =  mr->sequence[idx];
							_ksys_draw_text(ss, x*2*SYSFN04_FONT_W, idx*SYSFN04_FONT_H, 0, 0x90000000 | hlcolorrgb);
						}
					}
				}

				bool needToAdd = !Q_size(q[x]);
				if (!needToAdd) {
					Q_at(q[x], Q_size(q[x]) - 1, &mr);
					needToAdd = (mr->pos > 0);
				}
				if (needToAdd) {
					bool mrseq0 = 0;
					if (Q_size(q[x])) mrseq0 = (mr->sequence[0] != '\0');
					mr = (MatrixRow*)malloc(sizeof(MatrixRow));
					if (!mr) {
						writeError(STR_ERR_INST_MAIN_MROWALLOC);
						continue;
					}
					mr->length = (rand_r() % console_height) + 1;
					if (mr->length < 3) mr->gradLen = 0; //especially for len=2 (to avoid render twicely)
					else                mr->gradLen = rand_r() % (mr->length / 2 + 1); // "mr->length / 2" probably can be changed to "mr->length - mr->length / 3" if you want to encounter maximum 2/3 tinted rows
					mr->pos = 1 - mr->length;
					if (!mrseq0 && (rand_r() % 3) == 2) { // 33% chance of being row with symbols
						mr->sequence = (char*)malloc(console_height);
						if (!mr->sequence) {
							writeError(STR_ERR_INST_MAIN_MROWSECALLOC);
							free(mr);
							continue;
						}
						for (int i = 0; i < console_height; ++i)
							mr->sequence[i] = bakedTable[rand_r() % bakedTable_size];
					} else {
						mr->sequence = (char*)malloc(1);
						if (!mr->sequence) {
							writeError(STR_ERR_INST_MAIN_MROWSECALLOC);
							free(mr);
							continue;
						}
						mr->sequence[0] = '\0';
					}
					Q_push(q[x], &mr);
					_ksys_draw_bar(x*2*SYSFN04_FONT_W, 0, SYSFN04_FONT_W, SYSFN04_FONT_H, bgcolorrgb);
					if (mr->sequence[0] != '\0') {
						ss[0] =  mr->sequence[0];
 						_ksys_draw_text(ss, x*2*SYSFN04_FONT_W, 0, 0, 0x90000000 | hlcolorrgb);
					}
				}
			}

			_ksys_delay(matrixDelay);
		}
	}
	return 0;
}
