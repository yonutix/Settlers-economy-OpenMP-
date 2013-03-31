#ifndef _SERIAL_LIB_
#define _SERIAL_LIB_
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <omp.h>

#define GETDIST(x, y) (abs(x1 - x2) + abs(y1 - y2))

#define MIN(a,b)  ((((a)-(b))&0x80000000) >> 31)? (a) : (b)
#define MAX(a,b)  ((((a)-(b))&0x80000000) >> 31)? (b) : (a)

typedef unsigned int uint;
typedef unsigned char uchar;


/**
 * \struct Report Representh the four values
 */
struct Report{
	int resAmaxPrice;
	int resBmaxPrice;
	int resAProducers;
	int resBProducers;
};

/**
 * \class Year Represents the initial and final data for one
 */
class Year{
public:
	enum{RESA = 0, RESB = 1};
	enum{MAXPRICE = 100000, MINPRICE = 0x0};

	Year *previous;
	int **res;
	int **price;
	int **boudget;
	int n;
	int **boudgetInventory;
	int resProducers[2];
	int resMinPrice[2];
	int resMaxPrice[2];

	explicit Year(char* filename);
	Year(int n, Year* previous);
	~Year();

	/**
	 * \brief initialize the current year based on the previous
	 */
	void updateYear(Year* previous);

	/**
	 * \brief Calculates the losses or the wins for the current year
	 */
	void deployActivities();

	/**
	 * \brief Finds the lowest price for the opposite resource's type
	 */
	int getCheepestPrice(int x, int y);

	/**
	 * \brief Finds the lowest price of the same type
	 */
	int getCostRes(int x, int y);

	/**
	 * \brief Gets memory for the matrices with nxn elements
	 */
	void memAlloc(int n);

	/*
	 * \brief Returns the price of a resource including the distance price
	 */
	inline int getCost(int x1, int y1, int x2, int y2) { return (abs(x1 - x2) + abs(y1 - y2)) + price[x2][y2]; }
	
	/**
	 * \brief Returns the cost of the opposite resource
	 */
	inline int getCost(int x, int y) { return boudget[x][y] - boudgetInventory[x][y]; }
	
	/**
	 * \brief Write the results to a file
	 */
	static void exportResults(char* filename, Year* y);
};

#endif