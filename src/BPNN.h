#pragma once

#include "BPNeuron.h"
#include "Utils.h"

#include <vector>
#include <string>
#include <fstream>

struct BPLayer
{
	std::vector<BPNeuron *> neurons;
};

class BPNN
{
public:
	BPNN();
	BPNN(int layersCount, const SomArray<int> &numPerLayer); 

	void init(int layersCount, const SomArray<int> &numPerLayer);
	SomArray<float> classify(const SomArray<float> &vec);

	void train(const SomArray<float> &vec, const SomArray<float> &answ, float error = 0.0f);

	inline float f(float x);

	void save(std::string fn);
	void load(std::string fn);
//private:
	BPLayer *mLayers;
	int mLayersCount;
	float mAlpha;
	float mRule;
//	
};