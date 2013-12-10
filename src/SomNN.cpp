#include "stdafx.h"
#include "SomNN.h"

int SomNN::NORMALIZE = SOMNORM_NONE;

SomNN::SomNN()
{

}

SomNN::SomNN(int dim,const SomArray<int> &dims, int weightsNumber)
{
	mDim = dim;
	mDims = dims;

	mWeightsNumber = weightsNumber;

	buildMap();
}

void SomNN::buildMap()
{
	SomArray<float> w, c;
	w.initalize(mWeightsNumber);

	c.initalize(mDim);
	for (int i = 0; i < mDim; i++)
		c[i] = 0.0f;

	int iDim = 0;

	srand((unsigned int)time(0));
	while (true)
	{
		for (int i = 0; i < mWeightsNumber; i++) 
		{                    
			int r = 0xFFF & rand();           
			//r -= 0x800;
            w[i] = (float)r / 4096.0f;
			//w[i] 
        }
		SomNode *pNode = new SomNode(w, c);
		mNodes.push_back(pNode);

		for (int i = 0; i < mDim; i++)
		{
			c[i]++;

			if (c[i] >= mDims[i])
				c[i] = 0.0f;
			else
				break;
		}
        float sum = 0.0f;
		for (int i = 0; i < mDim; i++)
			sum += c[i];
       if (sum == 0.0f) 
		   break;
	}
}


SomNN::~SomNN(void)
{

}

void SomNN::train(std::vector<SomArray<float> *> &data, float R, float learningRule)
{
	for (int i = 0; i < data.size(); i++)
	{
		if (i % 1000 == 0)
			printf("%i\n", i);
		SomArray<float> w = normalize(*data.at(i));
		SomNode *bmu = bmuNode(w);

		if (R <= 1.0f) //only bmu traning
			bmu->train(w, learningRule);
		else
		{
			for (int i = 0; i < mNodes.size(); i++)
			{
				float dist = 0.0f; //
				for (int p = 0; p < mDim; p++)     //dist = sqrt((x1-y1)^2 + (x2-y2)^2 + ...)  distance to node
					dist += (bmu->mCoords[p] - mNodes[i]->mCoords[p]) * (bmu->mCoords[p] - mNodes[i]->mCoords[p]);
                dist = sqrt(dist);

				if( FAST_LEARNING && R > dist)
					continue;
// TO DO TEST BETTER FUNCTION LOL
                float y = exp(-(1.0f * dist * dist) / (R * R));
				mNodes[i]->train(w, learningRule * y);

			}
		}
	}
}

void SomNN::computeNormalization(const std::vector<SomArray<float>> &data)
{
	if (NORMALIZE == SOMNORM_NONE)
		return;
	if (NORMALIZE == SOMNORM_MINMAX)
	{
		mMin.initalize(mWeightsNumber);
		mMax.initalize(mWeightsNumber);
		for (int i = 0; i < mWeightsNumber; i++)
		{
			mMin[i] = FLT_MAX;
			mMax[i] = -FLT_MAX;
		}

		for (int i = 0; i < data.size(); i++)
			for (int j = 0; j < mWeightsNumber; j++)
			{
				if (mMin[j] > data[i][j]) mMin[j] = data[i][j];
				if (mMax[j] < data[i][j]) mMax[j] = data[i][j];
			}

		for (int i = 0; i < mWeightsNumber; i++)
		{
			float min, max;
			min = -mMin[i];
			if (mMax[i] - mMin[i] != 0)
				max = 1.0f / (mMax[i] - mMin[i]);
			else
				max = 1.0f;
			mMin[i] = min;
			mMax[i] = max;

		}

	}
}
//vote scheme
void SomNN::clustering(const std::vector<SomArray<float> *> &data, std::vector<int> &classes, int mClassesCount)
{
	for (int i = 0; i < mNodes.size(); i++)
		mNodes[i]->initVotes(mClassesCount);

	while(true)
	{
		for (int i = 0; i < data.size(); i++)
		{
			if (classes[i] == 0)
				continue;
			SomNode *bmu = bmuZeroNode(*data[i]);
			if (bmu == NULL)
				return;
			bmu->mVotes[classes[i] - 1]++;

		if ( i% 100  ==0)
			printf("%i\n", i);
		}
		for (int i = 0; i < mNodes.size(); i++)
			mNodes[i]->applyVotes();

	}
}

//direct clustering lol i wanna use better scheme
void SomNN::clustering(const std::vector<SomArray<float> *> &data, std::vector<int> &classes)
{
	for (int i = 0; i < mNodes.size(); i++)
	{
		int index = 0;
		float d, mind = FLT_MAX;
		if ( i% 5  ==0)
			printf("%i\n", i);
		for (int j = 0; j < data.size(); j++)
		{
			if (classes[j] == 0)
				continue;
			d = mNodes[i]->distance((*data[j]));
			if (d < mind)
			{
				mind = d;
				index = j;
			}
		}
		mNodes[i]->mClass = classes[index];
	}
}

int SomNN::classify(const SomArray<float> &vec)
{
	SomNode *pNode = bmuNode(vec);
	return pNode->mClass;
}

void SomNN::save(std::string fn)
{
	using namespace std;
	fstream f(fn, ios::out | ios::binary);
	int s = 666;
	f.write((char *)&s, 4);//magic number
	//std::vector<SomNode *> mNodes;
//	int mWeightsNumber;
	f.write((char *)&mDim, 4);//dim size
	for (int i = 0; i < mDim; i++)
		f.write((char *)&mDims[i], 4);//dims

	f.write((char *)&mWeightsNumber, 4);//weights size
	for (int i = 0; i < mNodes.size(); i++)
	{
		f.write((char *)&mNodes[i]->mClass, 4);//class
		for (int j = 0; j < mWeightsNumber; j++)
			f.write((char *)&mNodes[i]->mWeights[j], 4);//weights
	}
	f.close();
}

void SomNN::load(std::string fn)
{
	using namespace std;
	fstream f(fn, ios::in | ios::binary);
	int tmp = 0;
	f.read((char *)&tmp, 4);//magic number

	f.read((char *)&mDim, 4);//dim size
	mDims.initalize(mDim);
	for (int i = 0; i < mDim; i++)
		f.read((char *)&mDims[i], 4);//dims

	f.read((char *)&mWeightsNumber, 4);//weights size

	buildMap();
	for (int i = 0; i < mNodes.size(); i++)
	{
		f.read((char *)&mNodes[i]->mClass, 4);//class
		for (int j = 0; j < mWeightsNumber; j++)
			f.read((char *)&mNodes[i]->mWeights[j], 4);//weights
	}
	f.close();
}

void SomNN::distanceMap(SomArray<float> &map)
{
	map.initalize(mNodes.size());
	int index = 0;
	float mind = 1.5f;
    for (int i = 0; i < mDims[0]; i++)
		for (int j = 0; j < mDims[1]; j++)   
		{
			SomNode *pNode = mNodes[index];
			int nNum = 0;
			float dist = 0.0f;
			for (int k = 0; k < mNodes.size();k++)
			{
				SomNode *pMNode = mNodes[k];
				if (pMNode == pNode)
					continue;
				float tmp = 0.0f;
				for (int x = 0; x < mDim; x++)
					tmp += pow(pNode->mCoords[x] - pMNode->mCoords[x], 2.0f);
				tmp = sqrt(tmp);
				if (tmp <= mind)
				{
					nNum++;
					dist += pNode->distance(pMNode->mWeights);
				}

			}
			dist /= (float)nNum;
			map[index++] = dist;
		}
}

void SomNN::classMap(SomArray<float> &map)
{
	map.initalize(mNodes.size());
	int index = 0;
    for (int i = 0; i < mDims[0]; i++)
		for (int j = 0; j < mDims[1]; j++)   
		{
			int c = mNodes[index]->mClass;
			map[index++] = c;
		}
}

SomArray<float> SomNN::normalize(const SomArray<float> &vec)
{
	SomArray<float> data;
	if (NORMALIZE == SOMNORM_NONE)
		return vec;

	if (NORMALIZE == SOMNORM_MINMAX)
	{
		data = vec;
		for (int i = 0; i < mWeightsNumber; i++)
			data[i] = (0.9f - 0.1f) * (vec[i] + mMin[i]) * mMax[i] + 0.1f;  

	}
	if (NORMALIZE == SOMNORM_IMAGE)
	{
		data = vec;
		for (int i = 0; i < mWeightsNumber; i++)
			data[i] = vec[i] / 255.0f; 

	}
	return data;
}


SomNode *SomNN::bmuNode(const SomArray<float> &vec)
{
	SomNode *bmu = mNodes[0];
	float dist = bmu->distance(vec);

	for (int i = 1; i < mNodes.size(); i++)
	{
		float tmp = mNodes[i]->distance(vec);
		if (tmp < dist)
		{
			bmu = mNodes[i];
			dist = tmp;
		}
	}
	return bmu;
}

SomNode *SomNN::bmuZeroNode(const SomArray<float> &vec)
{
	SomNode *bmu = mNodes[0];
	float dist = bmu->distance(vec);

	for (int i = 1; i < mNodes.size(); i++)
		if (mNodes[i]->mClass == 0)
		{
			float tmp = mNodes[i]->distance(vec);
			if (tmp < dist)
			{
				bmu = mNodes[i];
			dist = tmp;
			}
		}
	if (bmu->mClass != 0)
		return NULL;
	return bmu;
}
