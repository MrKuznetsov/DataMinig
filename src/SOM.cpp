// SOM.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Utils.h"
#include "SomNode.h"
#include "SomNN.h"
#include "BPNN.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <math.h>
#define cimg_use_jpeg 1
#include "..\..\CImg\CImg.h"

const int IMGSIZE = 100;/// DONT CHANGE IT
const int SOM_ITTERATION = 50;
const int BP_ITTERATIONS = 50;
const int IMAGESCOUNT = 10000;

using namespace cimg_library;
using namespace std;

int readInt(fstream &f)
{
	char a[4], b[4];
	f.read(a, sizeof(int));
	for (int i = 0; i < 4; i++)
		b[3-i] = a[i];
	int c = *((int *)b);
	return c;
}

void readImageData(string fn, vector<SomArray<float> *> &data)
{
	fstream f(fn, ios::in | ios::binary);
	f.seekg(0, ios::beg);
	readInt(f);
	//readInt(f);
	int size = readInt(f);//60000
	if (IMAGESCOUNT < size)
		size = IMAGESCOUNT;
	int w = readInt(f);
	int h = readInt(f);
	for (int i = 0; i < size; i++)
	{
		SomArray<float> *pVec = new SomArray<float>(w * h);
		for (int j = 0; j < w * h; j++)
		{
			unsigned char c = 0;
			f.read((char *)&c, 1);
			(*pVec)[j] = c / 255.0f;
		}
		data.push_back(pVec);
	//	showVector(*pVec, w, h);
		//system("pause");
	}
	f.close();
	cout << "TRAIN IMAGES DATA LOADED\n";
//	cout << w << " " << h <<"\n";
}

void readLabelData(string fn, vector<SomArray<float> *> &data)
{
	fstream f(fn, ios::in | ios::binary);
	f.seekg(0, ios::beg);
	readInt(f);
	int size = readInt(f);//60000
	for (int i = 0; i < size; i++)
	{
		SomArray<float> *pVec = new SomArray<float>(10);
		for (int j = 0; j < 10; j++)
			(*pVec)[j] = 0;
			unsigned char c = 0;
			f.read((char *)&c, 1);
			(*pVec)[c] = 1;
			data.push_back(pVec);
	}
	f.close();
	cout << "TRAIN LABEL DATA LOADED\n";
}


void readLabelSomData(string fn, vector<int> &data)
{
	fstream f(fn, ios::in | ios::binary);
	f.seekg(0, ios::beg);
	readInt(f);
	int size = readInt(f);//60000
	for (int i = 0; i < size; i++)
	{
		unsigned char c = 0;
		f.read((char *)&c, 1);
		data.push_back(c+1);
	}
	f.close();
	cout << "TRAIN LABEL DATA LOADED\n";
}

/****************************************************
/*           CONVERT IMAGES TO SOM FORMAT
/****************************************************/
/*
	resize 250x250 to 100x100 (IMGSIZE * IMGSIZE)
*/

void ConvertDataForSom()//(vector<SomArray<float> *> &data, vector<int> &test)
{
	vector<string> mens;
	vector<string> wemens;

	//find all jpg files in dirrectories
	_WIN32_FIND_DATAA FindFileData;
	HANDLE hf;
	hf=FindFirstFileA("..\\Train\\1\\*.jpg", &FindFileData);
	if (hf!=INVALID_HANDLE_VALUE)
	{
		do
		{
			string fn =  FindFileData.cFileName;
			mens.push_back(fn);
		}
		while (FindNextFileA(hf,&FindFileData)!=0);
		FindClose(hf);
	}
	hf=FindFirstFileA("..\\Train\\2\\*.jpg", &FindFileData);
	if (hf!=INVALID_HANDLE_VALUE)
	{
		do
		{
			string fn =  FindFileData.cFileName;
			wemens.push_back(fn);
		}
		while (FindNextFileA(hf,&FindFileData)!=0);
		FindClose(hf);
	}

	//convert files to 100x100 images data
	cout << "START WOMEN" << endl;
	for (int i = 0; i < wemens.size();i++)
	{
		string fn = "..\\Train\\2\\" + wemens[i];
		CImg<unsigned char> src(fn.c_str());
		src.resize(-40, -40, -100, -100, 5);

		fn = "..\\Train\\2r\\" + to_string(i)+".dat";
		fstream f(fn, ios::out | ios::binary);
		for (int i = 0; i < IMGSIZE * IMGSIZE; i++)
		{
			int x = i / IMGSIZE;
			int y = i % IMGSIZE;

			int r = src(x,y,0,0);
			int g = src(x,y,0,1);
			int b = src(x,y,0,2);
			byte c = int((r + g + b) / 3);
			f.write((char *)&c, 1);
		}
		f.close();
	}
	cout << "START MAN" << endl;
	for (int i = 0; i < mens.size();i++)
	{
		string fn = "..\\Train\\1\\" + mens[i];
		CImg<unsigned char> src(fn.c_str());
		src.resize(-40, -40, -100, -100, 5);

		fn = "..\\Train\\1r\\" + to_string(i)+".dat";
		fstream f(fn, ios::out | ios::binary);
		for (int i = 0; i < IMGSIZE * IMGSIZE; i++)
		{
			int x = i / IMGSIZE;
			int y = i % IMGSIZE;

			int r = src(x,y,0,0);
			int g = src(x,y,0,1);
			int b = src(x,y,0,2);
			byte c = int((r + g + b) / 3);
			f.write((char *)&c, 1);
		}
		f.close();
	}
	cout << "END CONVERTING" << endl;

}
/****************************************************
/*                LOAD SOM DATA
/****************************************************/

void LoadDataForSom(vector<SomArray<float> *> &data, vector<int> &test)
{
	int MENSCOUNT = 8582;
	int WOMENSCOUNT = 2942;

	int mi = 0;//man index
	int wi = 0;//woman index
	srand(time(0));
	cout << "LOAD DATA FOR SEX RECOGNATION BEGIN\n";
	while(true)
	{
		if ((mi + wi) % 100 == 0)
			cout << mi << " " << wi << endl;

		int who = rand() % 4 == 0 ? 1 : 0;//0 - men, 1 - women
		if (mi >= MENSCOUNT && wi >= WOMENSCOUNT)
			break;
		if (who == 0 && mi >= MENSCOUNT)
			who = 1;
		if (who == 1 && wi >= WOMENSCOUNT)
			who = 0;
		int &index = who == 0 ? mi : wi;
		string fn = "..\\Train\\"+to_string(who + 1)+"r"+"\\"+ to_string(index) + ".dat";
		index += 1;//rand() % 18;

		fstream f(fn, ios::in | ios::binary);
		byte img[IMGSIZE * IMGSIZE];
		f.read((char *)img, IMGSIZE * IMGSIZE);
		f.close();
		test.push_back(who);
		//ok now we getting 10x10 part of images as data vector
		///	int x = i / IMGSIZE;
		///	int y = i % IMGSIZE;
		for (int i = 0; i < 10; i++)
			for (int j = 0; j < 10; j++)
			{
				int ii = 0;
				SomArray<float> *pVec = new SomArray<float>(10 * 10);
				for (int x = 0; x < 10; x++)
					for (int y = 0; y < 10; y++)
					{
						int xx = i * 10 + x;
						int yy = j * 10 + y;
						int index = xx * 100 + yy;
						(*pVec)[ii] = img[index] / 255.0f;
						ii++;
					}
				data.push_back(pVec);
			}
	}
	cout << "LOAD ENDED\n";
}

void showMap(SomArray<float> map)
{
	float min = map[0];
	float max = map[0];
	for (int i = 1; i < map.size; i++)
	{
		if (max < map[i])
			max = map[i];
		if (min > map[i])
			min = map[i];
	}
//	SomArray<float> m(map.size * 100);

	for (int i = 0; i < map.size; i++)
		map[i] = 1 - (map[i] - min) / (max - min + 0.001);

	showVector(map, sqrt(map.size), sqrt(map.size));

}

/****************************************************
/*            SOM FOR DEC DIMENSION
/****************************************************/
void doSexSOM()
{
	cout << "START SEX TRAIN SOM\n";
	vector<SomArray<float> *> itrain;
	vector<int> ltrain;
	LoadDataForSom(itrain, ltrain);	
	cout <<  itrain.size() << endl;

	SomArray<int> s;
	s.initalize(2);
	s[0] = 50;
	s[1] = 50;
	SomNode::METRIC = 1;
	SomNN *pMap = new SomNN(2, s, 10 * 10);

	float R = 25.0f;
	float rule = 0.9f;
	float N = SOM_ITTERATION;

	for (int i = 0; i < N; i++)
	{
		float r0 = R * exp(-10.0f * (i * i) / (N * N));
		float nrule = rule * exp(-10.0f * (i * i) / (N * N));
		pMap->train(itrain, r0, nrule);
		cout << i  << " R " << r0 << " nrule " << nrule << endl;		
	}
	pMap->save("..\\Train\\sex.som");
	cout << "TRANING IS DONE\n";
}

void doSexSomCluster()
{
	SomNN *pMap = new SomNN();
	pMap->load("..\\Train\\sex.som");
	int x = 0;
	int y = 0;
	int classesCount = 25;

	cout << "CLUSTERING BEGIN";
	vector<SomArray<float> *> data;
	vector<int> lable;
	//get random classes 
	int index  = 1;
	for (int j = 0; j < pMap->mNodes.size(); j++)
	{
		//12 = 560
		//13 = 597
		//11 = 564
		//12 11 = 527
		if (pMap->mNodes[j]->mCoords[0] > 0 && pMap->mNodes[j]->mCoords[1] > 0)
		if ((int)pMap->mNodes[j]->mCoords[0] % 11 == 0 && (int)pMap->mNodes[j]->mCoords[1] % 12 == 0)
		{
			SomArray<float> *vec = new SomArray<float>(10 * 10);
			*vec = (pMap->mNodes[j]->mWeights);
			data.push_back(vec);
			lable.push_back(index++);
		}
	}
	pMap->clustering(data, lable);
	cout << "Count of class is "<< index << endl;
	pMap->save("..\\Train\\sexC.som");

	SomArray<float> map;
	pMap->distanceMap(map);
	showMap(map);
	pMap->classMap(map);
	showMap(map);	
}

int getClass(const SomArray<float> &vec)
{
	int index = 0;
	float max = vec[0];
	for (int i = 1; i < vec.size; i++)
		if (vec[i] > max)
		{
			max = vec[i];
			index = i;
		}
	return index;
}

void doBpSex()
{
	SomNN *pMap = new SomNN();
	pMap->load("..\\Train\\sexC.som");

	srand((unsigned int)time(0));

	SomArray<int> layers(3);
	layers[0] = 10 * 10;//input layer
	layers[1] = 30;//mid layer
	layers[2] = 2;//output layer
	BPNN *pBP = new BPNN(3, layers);
	pBP->mAlpha = 0.1;
	pBP->mRule = 0.3;

	vector<SomArray<float> *> itrain;
	vector<int> ltrain;
	LoadDataForSom(itrain, ltrain);	
	
	cout << "BP CONVERT IMAGES FROM SOM TO BP VECTOR BEGIN\n";

	vector<SomArray<float> *> t;
	vector<SomArray<float> *> l;

	for (int i = 0; i < 10000; i++)
	{
		SomArray<float> *vec = new SomArray<float>(100);
		SomArray<float> *res = new SomArray<float>(2);
		for (int j = 0; j < 100; j++)
		{
			int c = pMap->classify(*itrain[i * 100 + j]);
			(*vec)[j] = (c - 1) / 16.0f;
		}
		(*res)[0] = 0;
		(*res)[1] = 0;
		(*res)[ltrain[i]] = 1;
		t.push_back(vec);
		l.push_back(res);
		if (i % 1000 == 0)
			cout << i << endl;
	}
	cout << "BP CONVERT ENDED\nTRAIN BEGIN\n";


	for (int k = 0; k < BP_ITTERATIONS; k++)
	{
		for (int i = 0; i < 8000; i++)
			pBP->train(*t[i], *l[i], 0.001f);
		cout << "Itteration: " << k << endl;
	}
	pBP->save("..\\Train\\sex.bp");

	cout << "TRAIN ENDED\nTEST BEGIN\n";
	int errr = 0;
	for (int i = 8000; i < 10000; i++)
	{
		SomArray<float> ans(2);
		ans = pBP->classify(*t[i]);
		if (getClass(ans) != getClass(*l[i]))
			errr++;
	}
	float accuracy = errr / 2000.0f;
	cout << "ERROR IS " << accuracy * 100.0f << "%" << endl;
	system("pause");
}


/****************************************************
/*              SOM FOR NUMBERS
/****************************************************/

void doSOM()
{
	cout << "SOM TRANING NUMBERS\n";
	vector<SomArray<float> *> itrain;
	vector<int> ltrain;
	vector<SomArray<float> *> itest;
	vector<int> ltest;

	readImageData("..\\Train\\train-images.idx3-ubyte", itrain);	//train set of images
	readImageData("..\\Train\\t10k-images.idx3-ubyte", itest);      //train set of labels


	readLabelSomData("..\\Train\\t10k-labels.idx1-ubyte", ltest);   //test set of images
	readLabelSomData("..\\Train\\train-labels.idx1-ubyte", ltrain); //test set of labels

	cout << "DO U WANT TO TRAIN SOM OR WANT TO LOAD IT\n0-train, 1-load\n"; 
	int param = 0;
	cin >> param;
	SomNN *pMap;
	if (param != 1)
	{
		cout << "TRAIN BEGIN\n";
		SomArray<int> s(2);//size of som map X.Y
		s[0] = 50;
		s[1] = 50;

		pMap = new SomNN(2, s, 28 * 28);//dimension, sizeof of each dim, size of weights vector
		SomNode::METRIC = 1;//set metric
		float R = 25.0f; //Default r of traning
		float rule = 0.9f;//defult coef of traning
		float N = SOM_ITTERATION;//number of itterations

		for (int i = 0; i < N; i++)//do train
		{
			float r0 = R * exp(-10.0f * (i * i) / (N * N));
			float nrule = rule * exp(-10.0f * (i * i) / (N * N));
			pMap->train(itrain, r0, nrule);
			cout << "Itteration: " << i << endl;	
		}

		cout << "TRAIN ENDED\nCLUSTERING BEGIN\n";
		pMap->clustering(itrain, ltrain, 10);
		pMap->save("..\\Train\\1.som");
		cout << "CLUSTERING ENDED\nTESTING BEGIN\n";
	}
	else
	{
		pMap = new SomNN();
		pMap->load("..\\Train\\1.som");
	}

	//UNCOMMNET to see distance map and class map
	/*	SomArray<float> map;
	pMap->distanceMap(map);
	showMap(map);
	pMap->classMap(map);
	showMap(map);	*/
	int errr = 0;
	for (int i = 0; i < itest.size();i++)
	{
		int c = pMap->classify((*itest[i]));
		if ( c != ltest[i])
			errr++;
	}
	float accuracy =  (float)errr / (float)itest.size();
	cout << "ERROR IS " << accuracy * 100.0f << "%" << endl;

	//UNCOMENT if you want to save som nodes into bmp images
	/*
	for (int i = 0; i < s[0] * s[1]; i++)
	{
		CImg<unsigned char> im(28 ,28 ,1, 3, 0);
		for (int j = 0; j < 28 * 28; j++)
		{
			int x = j / 28;
			int y = j % 28;
			unsigned char color[3];
			color[0] = int(pMap->mNodes[i]->mWeights[j] * 255);
			color[1] = int(pMap->mNodes[i]->mWeights[j] * 255);
			color[2] = int(pMap->mNodes[i]->mWeights[j] * 255);
			im.draw_point(x ,y ,color);
		}
		im.normalize(0, 255);
		std::string fn = "..\\Train\\b\\"+std::to_string(int(pMap->mNodes[i]->mCoords[0]))+"_"+std::to_string(int(pMap->mNodes[i]->mCoords[1]))+
			"_"+std::to_string(pMap->mNodes[i]->mClass)+".bmp";
		im.save(fn.c_str());
	}*/

	system("pause");
}


/****************************************************
/*                   BP
/****************************************************/

void doBP()
{
	cout << "BP TRANING NUMBERS\n";
	vector<SomArray<float> *> itrain;
	vector<SomArray<float> *> ltrain;
	vector<SomArray<float> *> itest;
	vector<SomArray<float> *> ltest;

	readImageData("..\\Train\\train-images.idx3-ubyte", itrain);	//train set of images
	readImageData("..\\Train\\t10k-images.idx3-ubyte", itest);		//train set of labels

	readLabelData("..\\Train\\t10k-labels.idx1-ubyte", ltest);		//test set of images
	readLabelData("..\\Train\\train-labels.idx1-ubyte", ltrain);	//test set of labels
	
	srand((unsigned int)time(0));
	BPNN *pBP;

	cout << "DO U WANT TO TRAIN BP OR WANT TO LOAD IT\n0-train, 1-load\n"; 
	int param = 0;
	cin >> param;
	if (param != 1)
	{
		SomArray<int> layers(3);//layers
		layers[0] = 28 * 28;//input layer
		layers[1] = 300;//mid layer
		layers[2] = 10;//output layer

		pBP = new BPNN(3, layers);//layers count, layers
		pBP->mAlpha = 0.1;
		pBP->mRule = 0.3;

		cout << "TRAIN BEGIN\n";
		for (int j = 0; j < BP_ITTERATIONS; j++)
		{
			for (int i = 0; i < itrain.size(); i++)
				pBP->train(*(itrain[i]), *(ltrain[i]),0.001f);

				cout << "Itteration: "<< j << endl;
		}
		pBP->save("..\\Train\\1.bp");
		cout << "TRAIN ENDED\n";
	}
	else
	{
		pBP = new BPNN();
		pBP->load("..\\Train\\1.bp");
	}
	cout << "TEST BEGIN\n";
	//test
	int errr = 0;
	for (int i = 0; i < itest.size(); i++)
	{
		SomArray<float> out(10);
		out = pBP->classify(*itest[i]);
		out.size = 10;
		if (getClass(out) != getClass(*itest[i]))
			errr++;
	}
	float accuracy =   (float)errr / (float)itest.size();
	cout << "ERROR IS " << accuracy * 100.0f << "%" << endl;

	system("pause");
}

/****************************************************
/*				  MAIN
/****************************************************/

int _tmain(int argc, _TCHAR* argv[])
{
	while(true)
	{
		cout << "Input command\n";
		cout << "0 - exit\n";
		cout << "1 - som numbers\n";
		cout << "2 - bp numbera\n";
		cout << "3 - sex recognation\n";

		int command = 0;
		cin >> command;

		if (command == 0)
			return 0;

		if (command == 1)//som with numbers
			doSOM();
		if (command == 2)//bp with numbers
			doBP();
		if (command == 3)//sex rec
		{
			cout << "Input 4 numbers(0, 1) for 1 - ConvertDataForSom, 2 - doSexSOM, 3 - doSexSomCluster, 4 - doBpSex\n";
			cout << "Examp.: 0 0 0 1 means that only doBpSex will done\n";
			int a,b,c,d;
			cin >> a >> b >> c >> d;
			if (a == 1)
				ConvertDataForSom();
			if (b == 1)
				doSexSOM();
			if (c == 1)
				doSexSomCluster();
			if (d == 1)
				doBpSex();
		}
	}


	doSexSomCluster();
	doBpSex();
}

