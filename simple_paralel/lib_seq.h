#ifndef _SERIAL_LIB_
#define _SERIAL_LIB_
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <omp.h>
#include <time.h>

#define GETDIST(x, y) (abs(x1 - x2) + abs(y1 - y2))

typedef unsigned int uint;
typedef unsigned char uchar;

class Year{
public:
	enum{RESA = 0, RESB = 1};
	enum{MAXPRICE = 10000, MINPRICE = 0};
	Year *previous;
	int **res;
	int **price;
	int **boudget;
	int n;
	int **boudgetInventory;
	int resProducers[2];
	int resMaxPrice[2];
	explicit Year(int n, Year* previous);
	explicit Year(char* filename);
	~Year();
	void deployActivities();
	int getCheepestPrice(int x, int y);
	inline int getCost(int x1, int y1, int x2, int y2) { return (abs(x1 - x2) + abs(y1 - y2)) + price[x2][y2]; }
	inline int getCost(int x, int y) { return boudget[x][y] - boudgetInventory[x][y]; }
	static void exportResults(char* filename, std::vector<Year*> &v);
};
#endif
