#pragma once

#include "Utils.h"
#include <xmmintrin.h>

class SomNode
{
public:
	 static int METRIC;
	 /*
	 0 - EUQLID
	 */

	SomNode(SomArray<float> &_weights, SomArray<float> &_coords, int _class = 0);
	~SomNode(void);

	void clearVotes();
	void initVotes(int size);
	void applyVotes();
//private:

	// weights of a node
	SomArray<float> mWeights;
	// coords of a node
	SomArray<float> mCoords; 
	int mClass;// class of a node
	int *mVotes;
	int mVotesSize;

	//METHODS ONLY FOR SOM NEURAL NETWORK:

	//train our node lol
	void train(const SomArray<float> &_weights, const float learningRule); 

	//distance beetwen vec and that node
	float distance(const SomArray<float> &_vec) const;

	inline float mse(const float *vec1, const float *vec2, int size) const; 

};

