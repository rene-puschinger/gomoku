#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define IDB_NEW_GAME 1001
#define IDB_DEMO 1002
#define IDB_ABOUT 1003
#define WNDCLASSNAME "WIN32GOMOKU"
#define WNDTITLE "GoMoku"
#define MOVETIME 2500
#define MAXDEPTH 20
#define SQUARE 20                   // size of the square
#define NUMROWS 20
#define NUMCOLS 20
#define NUMBLOCKS 26
#define TRANSSIZE 1600451           // transposition table size
#define SIZEX (NUMCOLS*SQUARE+1)    // x and y sizes of the window in pixels
#define SIZEY (NUMROWS*SQUARE+1)
#define LINECOL	RGB(100, 100, 100)  // line color for the desk
#define BGCOL RGB(219, 178, 113)    // background color of the desk
#define SPRITESIZE 17               // x or y size of sprite
#define CIRCLE 1                    // token for a circle
#define CROSS 2                     // token for a cross

/***********************************************************************************************/

void ClientResize(HWND hwnd, int nWidth, int nHeight) {
	RECT rcClient, rcWindow;
	POINT ptDiff;
	GetClientRect(hwnd, &rcClient);
	GetWindowRect(hwnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hwnd, rcWindow.left, rcWindow.top, nWidth + ptDiff.x, nHeight + ptDiff.y, TRUE);
}

/***********************************************************************************************/
/***********************************************************************************************/

class Application;                  // forward declaration

class Field {                       // the playing field consisting of crosses and circles
	struct {
		int item;
		bool pernament;
	} data[NUMCOLS+2][NUMROWS+2];
public:
	Field();
	int& at(int i, int j);          // returns item on index i,j
	bool& isPernament(int i, int j);
};

/***********************************************************************************************/

class Brain {
	struct Block {
		char string[10];
		int value;
	} blocks[NUMBLOCKS];
	Field* field;
	struct {
		unsigned hash;
		int value;
	} transTable[TRANSSIZE];   // the transposition table
	int bestI;
	int bestJ;  // the best position computed by the algorithm
	bool firstRun;
	struct {
		int i;
		int j;
	} bestCoords[MAXDEPTH+1];
	clock_t start;
	int bestPrice;
	bool isAdmissible(int i, int j);  // check if the [i,j]-position is admissible
	int payOff(int player);  // compute the pay-off for player
public:
	unsigned zobristCodes[NUMCOLS][NUMROWS][3];
	unsigned zobristKey;
	Brain(Field* field);
	/* the minimax algorithm with alpha-beta prunning */
	int minmax(int player, int depth, int maxDepth, int alpha, int beta);
	/* return the coordinates of the best move computed by the minimax algorithm */
	void getBestMove(int player, int* i, int* j);
	/* check a victory for player */
	bool isVictory(int player, int* vi, int* vj, int* direction);
	/* check a draw */
	bool isDraw();
	void initTransTable();
};

/***********************************************************************************************/

class Application {
	int gameCount;
	HWND hwnd;                      // handle of the main window
	HWND btNewGame;
	HWND btDemo;
	HWND btAbout;
	HDC hdc;                        // HDC of the main window
	HBRUSH brush;                   // brush used for painting the square table
	HPEN pen;                       // pen used for painting the square table
	bool idle;                      // true when not computing the best move
	int scoreCross;
	int scoreCircle;
	bool playingDemo;
	RECT oldrect;
	Field* field;
	Brain* brain;
	void putSprite(int x, int y, char* sprite);	// draw a sprite
	void putCircle(int i, int j);
	void putCross(int i, int j);
	void renderDesk();              // plot the playing desk, circles and crosses
	void clearDesk();
	void showScore();
	void showVictory(int vi, int vj, int direction);
	void playDemo();
	static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
public:
	Application(HINSTANCE hInstance, int nCmdShow);
	void run();
	~Application();
};

/***********************************************************************************************/
/***********************************************************************************************/

char circle[] = {
	00, 00, 00, 00, 00, 00, 06, 06, 06, 06, 06, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 06, 05, 05, 05, 05, 05, 05, 05, 06, 00, 00, 00, 00,
	00, 00, 00, 06, 05, 04, 04, 04, 04, 04, 04, 04, 05, 06, 00, 00, 00,
	00, 00, 06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06, 00, 00,
	00, 06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06, 00,
	00, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 00,
	06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06,
	06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06,
	06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06,
	06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06,
	06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06,
	00, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 00,
	00, 06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06, 00,
	00, 00, 06, 05, 04, 00, 00, 00, 00, 00, 00, 00, 04, 05, 06, 00, 00,
	00, 00, 00, 06, 05, 04, 04, 04, 04, 04, 04, 04, 05, 06, 00, 00, 00,
	00, 00, 00, 00, 06, 05, 05, 05, 05, 05, 05, 05, 06, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 00, 06, 06, 06, 06, 06, 00, 00, 00, 00, 00, 00,

};

char cross[] = {
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
	00, 01, 02, 03, 00, 00, 00, 00, 00, 00, 00, 00, 00, 03, 02, 01, 00,
	00, 02, 01, 02, 03, 00, 00, 00, 00, 00, 00, 00, 03, 02, 01, 02, 00,
	00, 03, 02, 01, 02, 03, 00, 00, 00, 00, 00, 03, 02, 01, 02, 03, 00,
	00, 00, 03, 02, 01, 02, 03, 00, 00, 00, 03, 02, 01, 02, 03, 00, 00,
	00, 00, 00, 03, 02, 01, 02, 03, 00, 03, 02, 01, 02, 03, 00, 00, 00,
	00, 00, 00, 00, 03, 02, 01, 02, 03, 02, 01, 02, 03, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 03, 02, 01, 02, 01, 02, 03, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 00, 03, 02, 01, 02, 03, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 03, 02, 01, 02, 01, 02, 03, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 03, 02, 01, 02, 03, 02, 01, 02, 03, 00, 00, 00, 00,
	00, 00, 00, 03, 02, 01, 02, 03, 00, 03, 02, 01, 02, 03, 00, 00, 00,
	00, 00, 03, 02, 01, 02, 03, 00, 00, 00, 03, 02, 01, 02, 03, 00, 00,
	00, 03, 02, 01, 02, 03, 00, 00, 00, 00, 00, 03, 02, 01, 02, 03, 00,
	00, 02, 01, 02, 03, 00, 00, 00, 00, 00, 00, 00, 03, 02, 01, 02, 00,
	00, 01, 02, 03, 00, 00, 00, 00, 00, 00, 00, 00, 00, 03, 02, 01, 00,
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
};

/***********************************************************************************************/
/***********************************************************************************************/

Field::Field() {
	for (int i = 0; i < NUMCOLS + 2; i++)
		for (int j = 0; j < NUMROWS + 2; j++) {
			data[i][j].item = 0;
		}
}

int& Field::at(int i, int j) {
	return data[i+1][j+1].item;
}

bool& Field::isPernament(int i, int j) {
	return data[i+1][j+1].pernament;
}

/***********************************************************************************************/

Brain::Brain(Field* field) {
	this->field = field;
	strcpy(blocks[0].string, " pp $"); blocks[0].value = 200;
	strcpy(blocks[1].string, " ppp $"); blocks[1].value = 5000;
	strcpy(blocks[2].string, "pppp $"); blocks[2].value = 8000;
	strcpy(blocks[3].string, "oppp $"); blocks[3].value = 100;
	strcpy(blocks[4].string, " pppo$"); blocks[4].value = 100;
	strcpy(blocks[5].string, " pp p $"); blocks[5].value = 5000;
	strcpy(blocks[6].string, " p pp $"); blocks[6].value = 5000;
	strcpy(blocks[7].string, "ppppp$"); blocks[7].value = 1000000;
	strcpy(blocks[8].string, " pppp$"); blocks[8].value = 8000;
	strcpy(blocks[9].string, "p ppp$"); blocks[9].value = 8000;
	strcpy(blocks[10].string, "pp pp$"); blocks[10].value = 8000;
	strcpy(blocks[11].string, "ppp p$"); blocks[11].value = 8000;
	strcpy(blocks[12].string, " p p $"); blocks[12].value = 200;
	strcpy(blocks[13].string, " oo $"); blocks[13].value = -300;
	strcpy(blocks[14].string, " ooo $"); blocks[14].value = -7000;
	strcpy(blocks[15].string, "oooo $"); blocks[15].value = -13000;
	strcpy(blocks[16].string, "pooo $"); blocks[16].value = -200;
	strcpy(blocks[17].string, " ooop$"); blocks[17].value = -200;
	strcpy(blocks[18].string, " oo o $"); blocks[18].value = -7000;
	strcpy(blocks[19].string, " o oo $"); blocks[19].value = -7000;
	strcpy(blocks[20].string, "ooooo$"); blocks[20].value = -1200000;
	strcpy(blocks[21].string, " oooo$"); blocks[21].value = -13000;
	strcpy(blocks[22].string, "o ooo$"); blocks[22].value = -13000;
	strcpy(blocks[23].string, "oo oo$"); blocks[23].value = -13000;
	strcpy(blocks[24].string, "ooo o$"); blocks[24].value = -13000;
	strcpy(blocks[25].string, " o o $"); blocks[25].value = -300;
	srand((unsigned) time(NULL));
	for (int i = 0; i < NUMCOLS; i++)
		for (int j = 0; j < NUMROWS; j++) 
			for (int k = 0; k < 3; k++)
				zobristCodes[i][j][k] = rand() | (rand() << 15);
}

void Brain::initTransTable() {
	for (int i = 0; i < TRANSSIZE; i++) {
		transTable[i].hash = 0xffffffff;
	}
	zobristKey = 0;
	for (int i = 0; i < NUMCOLS; i++) {
		for (int j = 0; j < NUMROWS; j++) {
			zobristKey ^= zobristCodes[i][j][0];
		}
	}
}

bool Brain::isAdmissible(int i, int j) {
	return (field->at(i, j) == 0 &&
		(  field->at(i-1,j-1) != 0 || field->at(i, j-1) != 0 || field->at(i+1,j-1) != 0
		|| field->at(i-1,j) != 0 || field->at(i+1,j) != 0
		|| field->at(i-1,j+1) != 0 || field->at(i, j+1) != 0 || field->at(i+1,j+1) != 0
		));
}

bool Brain::isVictory(int player, int* vi, int* vj, int* direction) {
	int k;
	for (int i = 0; i < NUMCOLS; i++) {
		for (int j = 0; j < NUMROWS; j++) {
			for (k = 0; k < 5 && i + k < NUMCOLS && field->at(i + k, j) == player; k++);
			if (k == 5) {
				if (vi) *vi = i;
				if (vj) *vj = j;
				if (direction) *direction = 1;
				return true;
			}
			for (k = 0; k < 5 && j + k < NUMROWS && field->at(i, j + k) == player; k++);
			if (k == 5) {
				if (vi) *vi = i;
				if (vj) *vj = j;
				if (direction) *direction = 2;
				return true;
			}
			for (k = 0; k < 5 && i + k < NUMCOLS && j + k < NUMROWS && field->at(i + k, j + k) == player; k++);
			if (k == 5) {
				if (vi) *vi = i;
				if (vj) *vj = j;
				if (direction) *direction = 3;
				return true;
			}
			for (k = 0; k < 5 && i - k > 0 && j + k < NUMROWS && field->at(i - k, j + k) == player; k++);
			if (k == 5) {
				if (vi) *vi = i;
				if (vj) *vj = j;
				if (direction) *direction = 4;
				return true;
			}
		}
	}
	return false;
}

bool Brain::isDraw() {
	for (int i = 0; i < NUMCOLS; i++) {
		for (int j = 0; j < NUMROWS; j++) {
			if (isAdmissible(i, j)) return false;
		}
	}
	Sleep(1000);
	return true;
}

int Brain::payOff(int player) {
	int result = 0;
	int opponent = (player == CIRCLE) ? CROSS : CIRCLE;
	for (int k = 0; k < NUMBLOCKS; k++) {
		for (int i = 0; i < NUMCOLS; i++) {
			for (int j = 0; j < NUMROWS; j++) {
				for (int l = 0; l < 10; l++) {
					if (i + l >= NUMCOLS) break;
					if (blocks[k].string[l] == ' ' && field->at(i + l, j) == 0);
					else if (blocks[k].string[l] == 'p' && field->at(i + l, j) == player);
					else if (blocks[k].string[l] == 'o' && field->at(i + l, j) == opponent);
					else if (blocks[k].string[l] == '$') {
						result += blocks[k].value;
						break;
					}
					else break;
				}
				for (int l = 0; l < 10; l++) {
					if (j + l >= NUMROWS) break;
					if (blocks[k].string[l] == ' ' && field->at(i, j + l) == 0);
					else if (blocks[k].string[l] == 'p' && field->at(i, j + l) == player);
					else if (blocks[k].string[l] == 'o' && field->at(i, j + l) == opponent);
					else if (blocks[k].string[l] == '$') {
						result += blocks[k].value;
						break;
					}
					else break;
				}
				for (int l = 0; l < 10; l++) {
					if (i - l < 0) break;
					if (j + l >= NUMROWS) break;
					if (blocks[k].string[l] == ' ' && field->at(i - l, j + l) == 0);
					else if (blocks[k].string[l] == 'p' && field->at(i - l, j + l) == player);
					else if (blocks[k].string[l] == 'o' && field->at(i - l, j + l) == opponent);
					else if (blocks[k].string[l] == '$') {
						result += blocks[k].value;
						break;
					}
					else break;
				}
				for (int l = 0; l < 10; l++) {
					if (i + l >= NUMCOLS) break;
					if (j + l >= NUMROWS) break;
					if (blocks[k].string[l] == ' ' && field->at(i + l, j + l) == 0);
					else if (blocks[k].string[l] == 'p' && field->at(i + l, j + l) == player);
					else if (blocks[k].string[l] == 'o' && field->at(i + l, j + l) == opponent);
					else if (blocks[k].string[l] == '$') {
						result += blocks[k].value;
						break;
					}
					else break;
				}
			}
		}
	}
	return result + (rand() % 30);
}

int Brain::minmax(int player, int depth, int maxDepth, int alpha, int beta) {
	static unsigned msgCnt = 0;
	if (msgCnt++ % 400 == 0) {
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				exit(0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	int opponent = (player == CIRCLE) ? CROSS : CIRCLE;
	if (depth == maxDepth) {
		if (transTable[zobristKey % TRANSSIZE].hash == zobristKey)
			return transTable[zobristKey % TRANSSIZE].value;
		int result = payOff(depth % 2 == 0 ? player : opponent);
		transTable[zobristKey % TRANSSIZE].hash = zobristKey;
		transTable[zobristKey % TRANSSIZE].value = result;
		return result;
	}
	if (clock() - start > MOVETIME) {
		return INT_MIN;
	}
	int price;
	int optI;
	int optJ;
	if (firstRun && depth == maxDepth - 1)
		firstRun = false;
	if (firstRun && maxDepth > 1 && isAdmissible(bestCoords[depth].i, bestCoords[depth].j)) {
		int ii = bestCoords[depth].i;
		int jj = bestCoords[depth].j;
		field->at(ii, jj) = player;
		field->isPernament(ii, jj) = false;
		zobristKey ^= zobristCodes[ii][jj][0];
		zobristKey ^= zobristCodes[ii][jj][player];
		if (depth % 2 == 0) {
			price = minmax(opponent, depth + 1, maxDepth, alpha, beta);
			if (price > alpha) {
				alpha = price;
				optI = ii;
				optJ = jj;
			}
			if (alpha >= beta) {
				field->at(ii, jj) = 0;
				zobristKey ^= zobristCodes[ii][jj][player];
				zobristKey ^= zobristCodes[ii][jj][0];
				if (depth == 0 && price > bestPrice) {
					bestPrice = price;
					bestI = optI;
					bestJ = optJ;
				}
				return alpha;
			}
		} else {
			price = minmax(opponent, depth + 1, maxDepth, alpha, beta);
			if (price < beta) {
				beta = price;
				optI = ii;
				optJ = jj;
			}
			if (alpha >= beta) {
				field->at(ii, jj) = 0;
				zobristKey ^= zobristCodes[ii][jj][player];
				zobristKey ^= zobristCodes[ii][jj][0];
				return beta;
			}
		}
		field->at(ii, jj) = 0;
		zobristKey ^= zobristCodes[ii][jj][player];
		zobristKey ^= zobristCodes[ii][jj][0];
		if (depth == 0 && price > bestPrice) {
			bestPrice = price;
			bestI = optI;
			bestJ = optJ;
		}
	}
	for (int ii = 0; ii < NUMCOLS; ii++) {
		for (int jj = 0; jj < NUMROWS; jj++) {
			if (!isAdmissible(ii, jj)) continue;
			if (maxDepth > 1 && ii == bestCoords[depth].i && jj == bestCoords[depth].j) continue;
			field->at(ii, jj) = player;
			field->isPernament(ii, jj) = false;
			zobristKey ^= zobristCodes[ii][jj][0];
			zobristKey ^= zobristCodes[ii][jj][player];
			if (depth % 2 == 0) {
				price = minmax(opponent, depth + 1, maxDepth, alpha, beta);
				if (price > alpha) {
					alpha = price;
					optI = ii;
					optJ = jj;
					bestCoords[depth].i = ii;
					bestCoords[depth].j = jj;
				}
				if (alpha >= beta) {
					field->at(ii, jj) = 0;
					zobristKey ^= zobristCodes[ii][jj][player];
					zobristKey ^= zobristCodes[ii][jj][0];
					if (depth == 0 && price > bestPrice) {
						bestPrice = price;
						bestI = optI;
						bestJ = optJ;
					}
					return alpha;
				}
			} else {
				price = minmax(opponent, depth + 1, maxDepth, alpha, beta);
				if (price < beta) {
					beta = price;
					optI = ii;
					optJ = jj;
					bestCoords[depth].i = ii;
					bestCoords[depth].j = jj;
				}
				if (alpha >= beta) {
					field->at(ii, jj) = 0;
					zobristKey ^= zobristCodes[ii][jj][player];
					zobristKey ^= zobristCodes[ii][jj][0];
					return beta;
				}
			}
			field->at(ii, jj) = 0;
			zobristKey ^= zobristCodes[ii][jj][player];
			zobristKey ^= zobristCodes[ii][jj][0];
			if (depth == 0 && price > bestPrice) {
				bestPrice = price;
				bestI = optI;
				bestJ = optJ;
			}
		}
	}
	return (depth % 2 == 0) ? alpha : beta;
}

void Brain::getBestMove(int player, int* i, int* j) {
	start = clock();
	initTransTable();
	bestPrice = INT_MIN;
	int d;
	for (d = 1; clock() - start < MOVETIME && d <= MAXDEPTH; d++) {
		firstRun = true;
		minmax(player, 0, d, INT_MIN, INT_MAX);
	}
	*i = bestI;
	*j = bestJ;
}

/***********************************************************************************************/

Application::Application(HINSTANCE hInstance, int nCmdShow) {
	scoreCross = 0;
	scoreCircle = 0;
	gameCount = 0;
	playingDemo = false;
	oldrect.left = 0;
	oldrect.top = 0;
	oldrect.right = SQUARE + 1;
	oldrect.left = SQUARE + 1;
	field = new Field();
	brain = new Brain(field);
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	wc.lpfnWndProc = Application::staticWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	char* wndClassName = WNDCLASSNAME;
	wc.lpszClassName = wndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wc); // todo: exception if result == null
	hwnd = CreateWindowEx(
		0,
		WNDCLASSNAME,
		WNDTITLE,
		WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, SIZEX, SIZEY,
		NULL, NULL, hInstance, this
	); // todo: exception if hwnd == null
	ClientResize(hwnd, SIZEX, SIZEY + 50);
	UpdateWindow(hwnd);
	ShowWindow(hwnd, nCmdShow);
	hdc = GetDC(hwnd);
	brush = CreateSolidBrush(BGCOL);
	pen = CreatePen(PS_SOLID, 1, LINECOL);
	renderDesk();
	showScore();
	idle = true;
}

LRESULT CALLBACK Application::staticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Application* app;
	if (msg == WM_CREATE) {
		app = (Application*) ((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR) app);
	} else {
		app = (Application*) GetWindowLongPtr(hwnd, GWL_USERDATA);
		if (!app) return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	app->hwnd = hwnd;
	return app->wndProc(msg, wParam, lParam);
}

LRESULT Application::wndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	switch(msg) {
		case WM_CTLCOLORSTATIC: {
			SetTextColor((HDC)wParam, RGB(0,0,0));
			return (LRESULT) CreateSolidBrush(RGB(255,255,255)); }
		case WM_CREATE: {
			btNewGame = CreateWindowEx(NULL, "BUTTON", "New game", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 16, 411, 100, 30, hwnd, (HMENU) IDB_NEW_GAME, NULL, NULL);
			btDemo = CreateWindowEx(NULL, "BUTTON", "Demo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 411, 60, 30, hwnd, (HMENU) IDB_DEMO, NULL, NULL);
			btAbout = CreateWindowEx(NULL, "BUTTON", "About", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 203, 411, 70, 30, hwnd, (HMENU) IDB_ABOUT, NULL, NULL);
			return 0; }
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDB_NEW_GAME:
					gameCount = 0;
					playingDemo = false;
					clearDesk();
					scoreCircle = 0;
					scoreCross = 0;
					showScore();
					break;
				case IDB_DEMO:
					playingDemo = true;
					clearDesk();
					scoreCircle = 0;
					scoreCross = 0;
					playDemo();
					break;
				case IDB_ABOUT:
					MessageBox(hwnd, "Gomoku v. 1.0. (c) 2009 Rene Puchinger", "Information", MB_ICONINFORMATION | MB_OK);
					break;
			}
			return 0; }
		case WM_MOUSEMOVE: {
			if (!idle) return 0;
			int i = (int) floor((float) GET_X_LPARAM(lParam)/SQUARE);  
			int j = (int) floor((float) GET_Y_LPARAM(lParam)/SQUARE);
			if (i >= NUMCOLS || j >= NUMROWS) return 0;
			int x = (int) floor((float) GET_X_LPARAM(lParam)/SQUARE)*SQUARE;  
			int y = (int) floor((float) GET_Y_LPARAM(lParam)/SQUARE)*SQUARE;
			RECT rect = {x, y, x + SQUARE + 1, y + SQUARE + 1};
			HBRUSH tmpbrush = CreateSolidBrush(LINECOL);
			FrameRect(hdc, &oldrect, tmpbrush);
			if (x < NUMCOLS*SQUARE && y < NUMROWS*SQUARE) FrameRect(hdc, &rect, (HBRUSH) GetStockObject(WHITE_BRUSH));
			oldrect = rect;
			DeleteObject(tmpbrush);
			return 0; }
		case WM_LBUTTONDOWN: {
			if (!idle) return 0;
			int i = (int) floor((float) GET_X_LPARAM(lParam)/SQUARE);  
			int j = (int) floor((float) GET_Y_LPARAM(lParam)/SQUARE);
			if (i == NUMCOLS || j == NUMROWS) return 0;
			if (field->at(i, j) != 0) return 0;
			putCircle(i, j);
			idle = false;
			int vi, vj, direction;
			if (brain->isDraw()) {
				clearDesk();
				if (++gameCount % 2 == 1)
					putCross((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
			}
			if (brain->isVictory(CIRCLE, &vi, &vj, &direction)) {
				showVictory(vi, vj, direction);
				scoreCircle++;
				clearDesk();
				showScore();
				if (++gameCount % 2 == 1)
					putCross((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
			} else {
				brain->getBestMove(CROSS, &i, &j);
				putCross(i, j);
			}
			if (brain->isDraw()) {
				clearDesk();
				if (++gameCount % 2 == 1)
					putCross((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
			}
			if (brain->isVictory(CROSS, &vi, &vj, &direction)) {
				showVictory(vi, vj, direction);
				scoreCross++;
				clearDesk();
				showScore();
				if (++gameCount % 2 == 1)
					putCross((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
			}
			idle = true;
			return 0; }
		case WM_PAINT: {
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);			
			return 0; }
		case WM_ACTIVATE: {
			renderDesk();
			showScore();
			return 0; }
		case WM_MOVE: {
			renderDesk();
			showScore();
			return 0; }
		case WM_CLOSE: 
		case WM_DESTROY: { PostQuitMessage(0); return 0; }
		default: break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);}

void Application::putCircle(int i, int j) {
	putSprite(i*(SQUARE+1)+2-i, j*(SQUARE+1)+2-j, circle);
	field->at(i,j) = CIRCLE;
	field->isPernament(i, j) = true;
	brain->zobristKey ^= brain->zobristCodes[i][j][0];
	brain->zobristKey ^= brain->zobristCodes[i][j][CIRCLE];
}

void Application::putCross(int i, int j) {
	putSprite(i*(SQUARE+1)+2-i, j*(SQUARE+1)+2-j, cross);
	field->at(i,j) = CROSS;
	field->isPernament(i, j) = true;
	brain->zobristKey ^= brain->zobristCodes[i][j][0];
	brain->zobristKey ^= brain->zobristCodes[i][j][CROSS];
}

void Application::run() {
	MSG msg;
	while (TRUE) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(50);
	}
 }

void Application::renderDesk() {
	SelectObject(hdc, brush);
	SelectObject(hdc, pen);
	for (int i = 0; i < SIZEX; i += SQUARE)
		for (int j = 0; j < SIZEY - SQUARE; j += SQUARE) {
			Rectangle(hdc, i, j, i + SQUARE + 1, j + SQUARE + 1);
		}
	for (int i = 0; i < NUMCOLS; i++)
		for (int j = 0; j < NUMROWS; j++) {
			if (field->at(i,j) == CROSS && field->isPernament(i, j))
				putCross(i,j);
			else if (field->at(i,j) == CIRCLE && field->isPernament(i, j))
				putCircle(i,j);
		}
}

void Application::clearDesk() {
	SelectObject(hdc, brush);
	SelectObject(hdc, pen);
	for (int i = 0; i < SIZEX; i += SQUARE)
		for (int j = 0; j < SIZEY - SQUARE; j += SQUARE) {
			Rectangle(hdc, i, j, i + SQUARE + 1, j + SQUARE + 1);
		}
	for (int i = 0; i < NUMCOLS; i++)
		for (int j = 0; j < NUMROWS; j++) {
			field->at(i,j) = 0;
		}
	brain->initTransTable();
}

void Application::putSprite(int x, int y, char *sprite) {
	int j = 0;
	for (int c = 0; c < SPRITESIZE * SPRITESIZE; c++) {
		if (sprite[c] == 1)
			SetPixel(hdc, x + c % SPRITESIZE, y + j, RGB(200, 0, 0));
		if (sprite[c] == 2)
			SetPixel(hdc, x + c % SPRITESIZE, y + j, RGB(230, 0, 0));
		if (sprite[c] == 3)
			SetPixel(hdc, x + c % SPRITESIZE, y + j, RGB(100, 0, 0));
		if (sprite[c] == 4)
			SetPixel(hdc, x + c % SPRITESIZE, y + j, RGB(80, 80, 220));
		if (sprite[c] == 5)
			SetPixel(hdc, x + c % SPRITESIZE, y + j, RGB(100, 100, 250));
		if (sprite[c] == 6)
			SetPixel(hdc, x + c % SPRITESIZE, y + j, RGB(80, 80, 220));
		if (((c+1) % SPRITESIZE) == 0) j++;
	}
}

void Application::showScore() {
	static HWND lblCircle = NULL;
	static HWND lblCross = NULL;
	char strCircle[100] = ": ";
	char strCross[100] = ": ";
	itoa(scoreCircle, &strCircle[2], 10);
	itoa(scoreCross, &strCross[2], 10);
	putSprite(298, 417, circle);
	if (!lblCircle) 
		lblCircle = CreateWindowEx(0, "STATIC", strCircle, WS_CHILD | WS_VISIBLE, 318, 416, 30, 30, hwnd, NULL, NULL, NULL);
	else
		SendMessage(lblCircle, WM_SETTEXT, 0, (LPARAM) strCircle);
	putSprite(350, 417, cross);
	if (!lblCross)
		lblCross = CreateWindowEx(0, "STATIC", strCross, WS_CHILD | WS_VISIBLE, 370, 416, 30, 30, hwnd, NULL, NULL, NULL);
	else
		SendMessage(lblCross, WM_SETTEXT, 0, (LPARAM) strCross);
}

void Application::showVictory(int vi, int vj, int direction) {
	HBRUSH tmpbrush = CreateSolidBrush(LINECOL);
	FrameRect(hdc, &oldrect, tmpbrush);
	DeleteObject(tmpbrush);
	for (int l = 0; l < 10; l++)
		for (int k = 0; k < 5; k++) {
			int x, y;
			if (direction == 1) {
				x = (vi + k) * SQUARE;  
				y = (vj) * SQUARE;
			}
			else if (direction == 2) {
				x = (vi) * SQUARE;  
				y = (vj + k) * SQUARE;
			}
			else if (direction == 3) {
				x = (vi + k) * SQUARE;  
				y = (vj + k) * SQUARE;
			}
			else {
				x = (vi - k) * SQUARE;  
				y = (vj + k) * SQUARE;
			}
			RECT rect = {x, y, x + SQUARE + 1, y + SQUARE + 1};
			FrameRect(hdc, &rect, (HBRUSH) GetStockObject(WHITE_BRUSH));
			Sleep(40);
		}
}

void Application::playDemo() {
	int i, j;
	int cnt = 0;
	idle = false;
	while (playingDemo) {
		if (cnt++ == 0)
			putCircle((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
		else {
			brain->getBestMove(CIRCLE, &i, &j);
			putCircle(i, j);
		}
		int vi, vj, direction;
		if (brain->isDraw()) {
			clearDesk();
			if (++gameCount % 2 == 1)
				putCross((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
		}
		if (brain->isVictory(CIRCLE, &vi, &vj, &direction)) {
			showVictory(vi, vj, direction);
			scoreCircle++;
			clearDesk();
			showScore();
			if (++gameCount % 2 == 1)
				putCross((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
			else
				putCircle((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
		} else {
			brain->getBestMove(CROSS, &i, &j);
			putCross(i, j);
		}
		if (brain->isDraw()) {
			clearDesk();
			if (++gameCount % 2 == 1)
				putCross((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
		}
		if (brain->isVictory(CROSS, &vi, &vj, &direction)) {
			showVictory(vi, vj, direction);
			scoreCross++;
			clearDesk();
			showScore();
			if (++gameCount % 2 == 1)
				putCross((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
			else
				putCircle((rand() % (NUMCOLS - 10)) + 5, (rand() % (NUMROWS - 10)) + 5);
		}
	}
	idle = true;
}

Application::~Application() {
	DeleteObject(brush);
	ReleaseDC(hwnd, hdc);
	if (brain) delete brain;
	if (field) delete field;
}

/***********************************************************************************************/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	Application* app = new Application(hInstance, nCmdShow);
	app->run();
	delete app;
	return 0;
}