#pragma once

#include <math.h>   


#define SOMNORM_MINMAX 1
#define SOMNORM_NONE 0
#define SOMNORM_IMAGE 2

#define cimg_use_jpeg 1
#include "..\..\CImg\CImg.h"
using namespace cimg_library;

template <class T>
struct SomArray
{
	SomArray()
	{
		size = 0;
		data = NULL;
	}

	SomArray(int _size)
	{
		size = _size;
		data = NULL;
		if (size > 0)
			data = new T[_size];
	}
	SomArray(const SomArray<T> &v)
	{

		initalize(v.size);
		for (int i = 0; i < size; i++)
			data[i] = v.data[i];
	}

	void initalize(int _size)
	{
		//if (data)
		//	delete[] data;
		data = NULL;
		size = _size;
		if (size > 0)
			data = new T[_size];
	}

	~SomArray()
	{
		if (data)
			delete[] data;
	}

	inline T &operator[](int index)
	{
		return data[index];
	}

	inline T operator[](int index) const
	{
		return data[index];
	}


	SomArray<T> &operator=(const SomArray<T> &other)
	{
		initalize(other.size);
		for (int i = 0; i < size; i++)
			data[i] = other.data[i];

		return *this;
	}

	T *data;
	int size;
};
void showVector(const SomArray<float> &vec, int w, int h);


