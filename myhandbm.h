#pragma once

class MyHandBM
{
public:
	MyHandBM(void);
	~MyHandBM(void);

	int SADWindowSize; // ~5x5..21x21
    int numberOfDisparities; // maximum disparity - minimum disparity (> 0)
};
