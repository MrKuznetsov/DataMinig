#pragma once

#include "Utils.h"

#include <vector>

struct BPLink;
struct BPLayer;

class BPNeuron
{
public:
	BPNeuron();
//	BPNeuron(BPLink *from, BPLink *to);
//private:
	std::vector<BPLink *> mLinksFrom;
	std::vector<BPLink *> mLinksTo;

	float mError;
	float mOutput;
};

struct BPLink
{
	BPLink();
	BPLink(BPNeuron *from, BPNeuron *to);
	BPNeuron *from;
	BPNeuron *to;

	float w;
	float dw;
};