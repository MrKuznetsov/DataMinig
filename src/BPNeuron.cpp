#include "BPNeuron.h"
#include "BPNN.h"

BPNeuron::BPNeuron()
{
	mError = 0;
	mOutput = 0;
}
/*
BPNeuron::BPNeuron(BPLink *from, BPLink *to)
{
	mError = 0;
	mOutput = 0;
	mLinkFrom = from;
	mLinkTo = to;
}
*/

BPLink::BPLink()
{
	w = 0.0001f;
	dw = 0.0f;
}

BPLink::BPLink(BPNeuron *_from, BPNeuron *_to)
{
	int ww = 0xFFF & rand();
    ww -= 0x800;
	w = (float)(rand())/(RAND_MAX/2) - 1;//float(ww) / 2048.0f / 10024.0f;
	dw = 0.0f;

	from = _from;
	to = _to;
}