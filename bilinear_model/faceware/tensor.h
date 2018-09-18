#pragma once
#include <vector>
#include <iostream>
#include <initializer_list>
#include <assert.h>
#include <memory>

#include <snow.h>

/* 3 dim Tensor3 */
class Tensor3
{
public:
	Tensor3() : mDataPtr(NULL), mSize(0), mIsSub(false), mShape(3, 0) {}
	Tensor3(const Tensor3 &b)
		: mDataPtr(b.mDataPtr)
		, mSize(b.mSize)
		, mIsSub(b.mIsSub)
		, mShape(b.mShape)
	{
		if (!mIsSub)
		{
            mDataPtr = snow::alignedMalloc<double>(mSize);
			memcpy(mDataPtr, b.mDataPtr, sizeof(double) * mSize);
		}
	}
	Tensor3(const std::vector<int> &shape) : Tensor3()
	{
		resize(shape);
	}
	Tensor3(const Tensor3 &father, int start, int len=-1)
		: mIsSub(true)
	{
		int c = 1;
		for (int i = 1; i < father.mShape.size(); ++i) c *= father.mShape[i];
		mDataPtr = father.mDataPtr + start * c;
		mShape.resize(father.mShape.size());
		for (int i = 1; i < mShape.size(); ++i) mShape[i] = father.mShape[i];
		if (len < 0)
		{
			len = father.mShape[0] - start;
		}
		mShape[0] = len;
		mSize = len * c;
	}
	~Tensor3() {
        if (!mIsSub) { if (!is_null()) snow::alignedFree(mDataPtr); mDataPtr = NULL; }
    }

	bool is_null() const { return mDataPtr == NULL; }

	void resize(const std::vector<int> & shape)
	{
		assert(shape.size() <= 3);
		if (mIsSub)
		{
			printf("[Tensor3 error]: You cannot resize sub-Tensor3.");
			exit(1);
			return;
		}
		mShape.resize(3);
		int size = 1;
		for (size_t i = 0; i < mShape.size(); ++i)
		{
			mShape[i] = (i < shape.size()) ? (shape[i]) : 1;
			size *= mShape[i];
		}
		if (size != mSize)
		{
            printf("resize\n");
            if (mDataPtr != nullptr) snow::alignedFree(mDataPtr);
            mDataPtr = snow::alignedMalloc<double>(size);
            printf("resize done\n");
			mSize = size;
		}
	}

	template <typename T>
	void unfoldData(T *data, int unfold_mode, bool is_data_colmajor=true)
	{
		assert(unfold_mode >= 0 && unfold_mode < 3);
		if (mIsSub)
		{
			printf("[Tensor3 error]: You cannot set data of sub-Tensor3.");
			exit(1);
			return;
		}

		std::vector<int> indices(mShape.size());
		std::vector<int> div(mShape.size());  // idx -> i, j, k...
		std::vector<int> mul(mShape.size());  // idx <- i, j, k...
		int D = indices.size();
		{
			div[mShape.size() - 1] = 1;
			for (int i = mShape.size() - 2; i >= 0; --i) { div[i] = div[i + 1] * mShape[i + 1]; }
		}
		if (is_data_colmajor)
		{
			for (int i = 0, d=unfold_mode, v=1; i < D; ++i)
			{
				mul[d] = v;
				v *= mShape[d];
				d = (d + 1) % D;
			}
		}
		else
		{
			printf("not support so far.\n");
			exit(1);
			return;
		}

		for (int i = 0; i < mSize; ++i)
		{
			int ii = i;
			int j = 0;
			for (int d = 0; d < D; ++d)
			{
				indices[d] = ii / div[d];
				ii %= div[d];
				j += indices[d] * mul[d];
			}
			mDataPtr[i] = data[j];
		}
	}

	const std::vector<int> &shape() const { return mShape; }
    const int shape(int i) const { return mShape[i]; }

	template <int Mode>
	void mulVec(const double *vec, Tensor3 &result) const;
	void mulVec(const double *vec1, const double *vec2, Tensor3 &result) const;
	template <int Mode>
	void mulMat(const double *mat, int rows, int cols, Tensor3 &result) const;
	void mul(double value, Tensor3 &result) const;

	template <int Mode>
	void unfold(Tensor3 &result);

	double *data() { return mDataPtr; }
	const double *data() const { return mDataPtr; }

	void setZero() { memset(mDataPtr, 0, sizeof(double) * mSize); }

private:
	double *			mDataPtr;
	int					mSize;
	bool				mIsSub;
	std::vector<int>	mShape;
};

inline std::ostream &operator<<(std::ostream &out, const std::vector<int> &v)
{
	out << "[ ";
	for (int i = 0; i < v.size(); ++i)
	{
		out << v[i];
		if (i == v.size() - 1) out << " ]"; else out << " , ";
	}
	return out;
}

template <>
inline void Tensor3::mulVec<1>(const double *vec, Tensor3 &result) const
{
    {
        // snow::StopWatch watch("alloc");
        if (!(result.mShape[0] == mShape[0] && result.mShape[1] == 1 && result.mShape[2] == mShape[2]))
            result.resize({ mShape[0], 1, mShape[2] });
        result.setZero();
    }
    const int shape12 = mShape[1] * mShape[2];
	for (int i = 0, c = 0, c0 = 0; i < mShape[0]; ++i, c += mShape[2], c0 += shape12) {
		for (int j = 0, c1= 0; j < mShape[1]; ++j, c1 += mShape[2]) {
            snow::addwb(&result.mDataPtr[c], &mDataPtr[c0+c1], vec[j], mShape[2]);
		}
	}
}

template <>
inline void Tensor3::mulVec<2>(const double *vec, Tensor3 &result) const
{
    {
        // snow::StopWatch watch("alloc");
        if (!(result.mShape[0] == mShape[0] && result.mShape[1] == mShape[1] && result.mShape[2] == 1))
            result.resize({ mShape[0], mShape[1], 1 });
        result.setZero();
    }
    const int shape12 = mShape[1] * mShape[2];
    for (int i = 0, c = 0, c0 = 0; i < mShape[0]; ++i, c += mShape[1], c0 += shape12) {
    	for (int j = 0, c1 = 0; j < mShape[1]; ++j, c1 += mShape[2]) {
            result.mDataPtr[c + j] += snow::dot(&mDataPtr[c0 + c1], vec, mShape[2]);
		}
	}
}

inline void Tensor3::mulVec(const double *vec1, const double *vec2, Tensor3 &result) const
{
    Tensor3 tmp;
    if (mShape[2] > mShape[1]) {
        mulVec<2>(vec2, tmp);
        tmp.mulVec<1>(vec1, result);
    }
    else {
        mulVec<1>(vec1, tmp);
        tmp.mulVec<2>(vec2, result);
    }
}

template <>
inline void Tensor3::mulMat<1>(const double *mat, int rows, int cols, Tensor3 &result) const
{
	assert(rows == mShape[1]);
	if (!(result.mShape[0] == mShape[0] && result.mShape[1] == cols && result.mShape[2] == mShape[2]))
		result.resize({ mShape[0], cols, mShape[2] });
	result.setZero();
#pragma omp parallel for
	for (int i = 0; i < mShape[0]; ++i)
	{
		int c0 = i * mShape[1] * mShape[2];
		int r0 = i * cols * mShape[2];
		for (int j = 0; j < mShape[1]; ++j)
		{
			int c1 = j * mShape[2];
			int idx = j * cols;
			for (int m = 0; m < cols; ++m)
			{
				int r1 = m * mShape[2];
				for (int k = 0; k < mShape[2]; ++k)
				{
					result.mDataPtr[r0 + r1 + k] += mDataPtr[c0 + c1 + k] * mat[idx + m];
				}
			}
		}
	}
}

template <>
inline void Tensor3::mulMat<2>(const double *mat, int rows, int cols, Tensor3 &result) const
{
	assert(rows == mShape[2]);
	if (!(result.mShape[0] == mShape[0] && result.mShape[1] == mShape[1] && result.mShape[2] == cols))
		result.resize({ mShape[0], mShape[1], cols });
	result.setZero();
#pragma omp parallel for
	for (int i = 0; i < mShape[0]; ++i)
	{
		int c0 = i * mShape[1] * mShape[2];
		int r0 = i * mShape[1] * cols;
		for (int j = 0; j < mShape[1]; ++j)
		{
			int c1 = j * mShape[2];
			int r1 = j * cols;
			for (int k = 0; k < mShape[2]; ++k)
			{
				int idx = k * cols;
				for (int m = 0; m < cols; ++m)
				{
					result.mDataPtr[r0 + r1 + m] += mDataPtr[c0 + c1 + k] * mat[idx + m];
				}
			}
		}
	}
}

template <>
inline void Tensor3::unfold<1>(Tensor3 &result)
{
	if (!(result.mShape[0] == mShape[1] && result.mShape[1] == mShape[2] * mShape[0] && result.mShape[2] == 1))
		result.resize({ mShape[1], mShape[2] * mShape[0], 1 });
	result.setZero();
#pragma omp parallel for
	for (int i = 0; i < mShape[0]; ++i)
	{
		int c0 = i * mShape[1] * mShape[2];
		for (int j = 0; j < mShape[1]; ++j)
		{
			int c1 = j * mShape[2];
			int r0 = j * mShape[2] * mShape[0];
			for (int k = 0; k < mShape[2]; ++k)
			{
				result.mDataPtr[r0 + k * mShape[0] + i] = mDataPtr[c0 + c1 + k];
			}
		}
	}
}

template <>
inline void Tensor3::unfold<2>(Tensor3 &result)
{
	if (!(result.mShape[0] == mShape[2] && result.mShape[1] == mShape[0] * mShape[1] && result.mShape[2] == 1))
		result.resize({ mShape[2], mShape[0] * mShape[1], 1 });
	result.setZero();
	int shape01 = mShape[0] * mShape[1];
#pragma omp parallel for
	for (int i = 0; i < mShape[0]; ++i)
	{
		int c0 = i * mShape[1] * mShape[2];
		int r1 = i * mShape[1];
		for (int j = 0; j < mShape[1]; ++j)
		{
			int c1 = j * mShape[2];
			for (int k = 0; k < mShape[2]; ++k)
			{
				result.mDataPtr[k * shape01 + r1 + j] = mDataPtr[c0 + c1 + k];
			}
		}
	}
}

inline void Tensor3::mul(double scale, Tensor3 &result) const
{
	if (!(result.mShape == mShape))
		result.resize(mShape);
#pragma omp parallel for
	for (int i = 0; i < mShape[0]; ++i)
	{
		int c0 = i * mShape[1] * mShape[2];
		for (int j = 0; j < mShape[1]; ++j)
		{
			int c1 = j * mShape[2];
			for (int k = 0; k < mShape[2]; ++k)
			{
				result.mDataPtr[c0 + c1 + k] = mDataPtr[c0 + c1 + k] * scale;
			}
		}
	}
}
