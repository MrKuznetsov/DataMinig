#include "stdafx.h"
#include "SomNode.h"

int SomNode::METRIC = 0;


SomNode::SomNode(SomArray<float> &_weights, SomArray<float> &_coords, int _class)
{
	mWeights.initalize(_weights.size);
	for (int i = 0; i < _weights.size; i++)
		mWeights[i] = _weights[i];

	mCoords.initalize(_coords.size);
	for (int i = 0; i < _coords.size; i++)
		mCoords[i] = _coords[i];

	mClass = _class;
	mVotesSize = 0;
}


SomNode::~SomNode(void)
{

}

void SomNode::clearVotes()
{
	for (int i = 0; i < mVotesSize; i++)
		mVotes[i] = 0;
}

void SomNode::initVotes(int size)
{
	if (mVotesSize > 0)
		delete mVotes;
	mVotesSize = size;
	mVotes = new int[size];
	clearVotes();
}

void SomNode::applyVotes()
{
	if (mClass != 0 || mVotesSize < 1)
		return;
	int index = 0;
	int max = mVotes[0];
	for (int i = 1; i < mVotesSize; i++)
		if (mVotes[i] > max)
		{
			max = mVotes[i];
			index = i;
		}
	if (max > 0)
		mClass = index + 1; 
}

// PRIVATE

void SomNode::train(const SomArray<float> &_weights, const float learningRule)
{
	for (int i = 0; i < mWeights.size; i++)
		mWeights[i] += learningRule * (_weights[i] - mWeights[i]);
}

float SomNode::distance(const SomArray<float> &_vec) const
{
	float dist = 0.0f;
	switch(METRIC)
	{
	case (0) :// EUQLID
		//need optimization for high dim vectors
		if (mWeights.size > 3)
			dist = mse(_vec.data, mWeights.data, _vec.size);
		else
			for (int i = 0; i < mWeights.size; i++)
				dist += (_vec[i] - mWeights[i]) * (_vec[i] - mWeights[i]);
		return sqrt(dist);
	case(1)://SQR
		if (mWeights.size > 3)
			dist = mse(_vec.data, mWeights.data, _vec.size);
		else
			for (int i = 0; i < mWeights.size; i++)
				dist += (_vec[i] - mWeights[i]) * (_vec[i] - mWeights[i]);
		return dist;
		//TO DO OTHER METRICS LOL
	}
}

inline float SomNode::mse(const float *vec1, const float *vec2, int size) const 
{
        float z = 0.0f, fres = 0.0f;
        float ftmp[4];
        __m128 mv1, mv2, mres;
        mres = _mm_load_ss(&z);

        for (int i = 0; i < size / 4; i++) {
                mv1 = _mm_loadu_ps(&vec1[4*i]);
                mv2 = _mm_loadu_ps(&vec2[4*i]);
                mv1 = _mm_sub_ps(mv1, mv2);
                mv1 = _mm_mul_ps(mv1, mv1);
                mres = _mm_add_ps(mres, mv1);
        }
        if (size % 4) {                
                for (int i = size - size % 4; i < size; i++)
                        fres += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);
        }

        //mres = a,b,c,d
        mv1 = _mm_movelh_ps(mres, mres);   //a,b,a,b
        mv2 = _mm_movehl_ps(mres, mres);   //c,d,c,d
        mres = _mm_add_ps(mv1, mv2);       //res[0],res[1]

        _mm_storeu_ps(ftmp, mres);        

        return fres + ftmp[0] + ftmp[1];
}
