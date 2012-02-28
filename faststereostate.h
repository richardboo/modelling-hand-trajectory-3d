#pragma once

/**
 * Klasa z parametrami algorytmu FAST Feature Matching.
 */
class FastStereoState
{
public:
	FastStereoState(void);
	~FastStereoState(void);

	int featuresNr;
	int featuresTheshold;
};
