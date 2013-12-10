#include "BPNN.h"

BPNN::BPNN()
{
	mLayersCount = 0;
	mLayers = NULL;
}

BPNN::BPNN(int layersCount, const SomArray<int> &numPerLayer)
{
	init(layersCount, numPerLayer);
}

void BPNN::init(int layersCount, const SomArray<int> &numPerLayer)
{
	if (layersCount < 2)
	{
		//ERRRRPROROR
		printf("TO LESS LAYERS\n");
		return;
	}
	mLayersCount = layersCount;

	mLayers = new BPLayer[mLayersCount];
	for (int i = 0; i < mLayersCount; i++)
		for (int j = 0; j < numPerLayer[i]; j++)
			mLayers[i].neurons.push_back(new BPNeuron());
	//INIT LINKS
	for (int l = 1; l < mLayersCount ; l++)
		for (int i = 0; i < mLayers[l].neurons.size(); i++)
		{
			for (int j = 0; j < mLayers[l - 1].neurons.size(); j++)
			{
				//BPLink *from = mLayers
				BPLink *link = new BPLink( mLayers[l - 1].neurons[j], mLayers[l].neurons[i]);
				mLayers[l].neurons[i]->mLinksFrom.push_back(link);
				mLayers[l - 1].neurons[j]->mLinksTo.push_back(link);
			}
			//add bias
			BPLink *link = new BPLink( NULL, mLayers[l].neurons[i]);
			mLayers[l].neurons[i]->mLinksFrom.push_back(link);
		}
}

SomArray<float> BPNN::classify(const SomArray<float> &vec)
{
	//input 
	for (int i = 0; i < mLayers[0].neurons.size(); i++)
		mLayers[0].neurons[i]->mOutput = (vec[i]);

	//other
	for (int l = 1; l < mLayersCount; l++)
		for (int i = 0; i < mLayers[l].neurons.size(); i++)
		{
			float out = 0;
			int size = mLayers[l].neurons[i]->mLinksFrom.size() - 1;
			for (int j = 0; j < size; j++)
				out +=  mLayers[l].neurons[i]->mLinksFrom[j]->w *  mLayers[l].neurons[i]->mLinksFrom[j]->from->mOutput;
			//bias
			out += mLayers[l].neurons[i]->mLinksFrom[size]->w;

			mLayers[l].neurons[i]->mOutput = f(out);
		}

	SomArray<float> out(mLayers[mLayersCount - 1].neurons.size());
	for (int i = 0; i < out.size; i++)
		out[i] = mLayers[mLayersCount - 1].neurons[i]->mOutput;
	
	return out;
}

void BPNN::train(const SomArray<float> &vec, const SomArray<float> &answ, float error)
{
	SomArray<float> res;
	res = classify(vec);
	float e = 0.0f;
	float out, delta;
	for (int i = 0; i < res.size; i++)
	{
		e += abs(vec[i] - res[i]);
		if (e > error)
			break;
	}

	if (e <= error)
		return;

	//BackPropogation
	//output layer
	for (int i = 0; i < mLayers[mLayersCount - 1].neurons.size(); i++)
	{
		out = mLayers[mLayersCount - 1].neurons[i]->mOutput;
		mLayers[mLayersCount - 1].neurons[i]->mError = out * (1.0f - out) * (answ[i] - out);
	}

	for (int l = mLayersCount - 2; l > 0; --l)
		for (int i = 0; i < mLayers[l].neurons.size(); i++)
		{
			out = mLayers[l].neurons[i]->mOutput;
			delta = 0.0f;

			for (int j = 0; j < mLayers[l].neurons[i]->mLinksTo.size(); j++)
				delta += mLayers[l].neurons[i]->mLinksTo[j]->w * mLayers[l].neurons[i]->mLinksTo[j]->to->mError;
			mLayers[l].neurons[i]->mError = out * (1.0f - out) * delta;
		}

	//do weights
	for (int l = 1; l < mLayersCount; l++)
		for (int i = 0; i < mLayers[l].neurons.size(); i++)
		{
			for (int j = 0; j < mLayers[l].neurons[i]->mLinksFrom.size() - 1; j++)
			{
				BPLink *pLink = mLayers[l].neurons[i]->mLinksFrom[j];
				float dw = mAlpha * pLink->dw + /*(1.0f - mAlpha) **/ mRule * pLink->from->mOutput * mLayers[l].neurons[i]->mError;
				pLink->dw = dw;
				pLink->w += dw;
			}
			//bias
			
			BPLink *pLink = mLayers[l].neurons[i]->mLinksFrom[mLayers[l].neurons[i]->mLinksFrom.size() - 1];
			float dw = mRule * mLayers[l].neurons[i]->mError;
			pLink->w += dw + mAlpha * pLink->dw;
			pLink->dw = dw;
		}

}

float BPNN::f(float x)
{
	return 1.0f / (1.0f + exp(float((-1.0f) * x)));
}

void BPNN::save(std::string fn)
{
	using namespace std;
	fstream f(fn, ios::out | ios::binary);
	int s = 333;
	f.write((char *)&s, 4);//magic number

	f.write((char *)&mAlpha, 4);//alpha coef
	f.write((char *)&mRule, 4);//rule coef

	f.write((char *)&mLayersCount, 4);//layers count
	for (int i = 0; i < mLayersCount; i++)//neurons per layer
	{
		int size = mLayers[i].neurons.size();
		f.write((char *)&size, 4);
	}
	for (int i = 1; i < mLayersCount; i++)//weights
		for (int j = 0; j < mLayers[i].neurons.size(); j++)
			for (int k = 0; k < mLayers[i].neurons[j]->mLinksFrom.size(); k++)
			{
				f.write((char *)&mLayers[i].neurons[j]->mLinksFrom[k]->w, 4);
				f.write((char *)&mLayers[i].neurons[j]->mLinksFrom[k]->dw, 4);
			}
	f.close();
}

void BPNN::load(std::string fn)
{
	using namespace std;
	fstream f(fn, ios::in | ios::binary);
	int s = 333;
	f.read((char *)&s, 4);//magic number

	f.read((char *)&mAlpha, 4);//alpha coef
	f.read((char *)&mRule, 4);//rule coef

	f.read((char *)&mLayersCount, 4);//layers count
	SomArray<int> numPerLayer(mLayersCount);
	for (int i = 0; i < mLayersCount; i++)//neurons per layer
	{
		int size;
		f.read((char *)&size, 4);
		numPerLayer[i] = size;
	}
	init(mLayersCount, numPerLayer);

	for (int i = 1; i < mLayersCount; i++)//weights
		for (int j = 0; j < mLayers[i].neurons.size(); j++)
			for (int k = 0; k < mLayers[i].neurons[j]->mLinksFrom.size(); k++)
			{
				float w, dw;
				f.read((char *)&w, 4);
				f.read((char *)&dw, 4);
				mLayers[i].neurons[j]->mLinksFrom[k]->w = w;
				mLayers[i].neurons[j]->mLinksFrom[k]->dw = dw;
			}
	f.close();
}