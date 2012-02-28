#pragma once

/**
 * Klasa z parametrami algorytmu Pixel Matching.
 */
class MyHandBM
{
public:
	MyHandBM(void);
	~MyHandBM(void);

	int medianSmooth;
    int numberOfDisparities; // maximum disparity - minimum disparity (> 0)
};
