#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <time.h>

#include "SomNode.h"

#define FAST_LEARNING 0

class SomNN
{
public:
	// 0 - none, 1 - minmax
	static int NORMALIZE;
	SomNN();
	//dim - count of coords, dims[i] - max coord 
	SomNN(int dim,const SomArray<int> &dims, int weightsNumber);

	~SomNN(void);

	void train(std::vector<SomArray<float> *> &data, float R, float learningRule);
	void computeNormalization(const std::vector<SomArray<float>> &data);
	//data[i] - vector of weights, classes[i] - class of data[i] 0 - means no class
	void clustering(const std::vector<SomArray<float> *> &data, std::vector<int> &classes);

	void clustering(const std::vector<SomArray<float> *> &data, std::vector<int> &classes, int mClassesCount);
	int classify(const SomArray<float> &vec);

	void save(std::string fn);
	void load(std::string fn);
	void distanceMap(SomArray<float> &map);
	void classMap(SomArray<float> &map);
//private:
	std::vector<SomNode *> mNodes;

	int mDim;
	SomArray<int> mDims;
	int mWeightsNumber;

	SomArray<float> mMin;//for min max normalization
	SomArray<float> mMax;


	SomNode *bmuNode(const SomArray<float> &vec);
	SomNode *bmuZeroNode(const SomArray<float> &vec);
	SomArray<float> normalize(const SomArray<float> &vec);
	//build node map
	void buildMap();
};

