#pragma once
#include <map>
#include <cmath>
#include <vector>

class BilateralFilter1D {
private:
	double mFactor, mDistSigma, mSimSigma;
	int    mRadius;
	std::map<int, double> mDistTable;

	void __buildDistanceWeightTable() {
		mDistTable.clear();
		for (int i = -mRadius; i <= mRadius; ++i) {
			double delta = (double)i / mDistSigma;
			mDistTable.insert({ i, std::exp(delta * delta * mFactor) });
		}
	}

	void __buildSimilarityWeightTable() {}

public:
	BilateralFilter1D(double distanceSigma      = 1,
		              double similaritySigma    = 1,
                      int    radius             = 5,
                      double factor             = -0.5)
		: mFactor(factor), mDistSigma(distanceSigma)
		, mSimSigma(similaritySigma), mRadius(radius) {
		this->__buildDistanceWeightTable();
		this->__buildSimilarityWeightTable();
	}

	double distanceWeight(int distance) {
		auto it = mDistTable.find(distance);	
		return (it != mDistTable.end()) ? mDistTable.find(distance)->second : 0.0;
	}

	double similarityWeight(double delta) {
		delta = std::abs(delta) / mSimSigma;
		return std::exp(delta * delta * mFactor);
	}

	template <typename T>
	std::vector<T> filter(const std::vector<T> &signal) {
		int r = mRadius;
		std::vector<T> newSignal;
		for (int i = 0; i < signal.size(); ++i) {
			T mean = 0.0;
			double weight = 0.0;
			for (int di = -r; di <= r; ++di) {
				int ni = di + i;
				if (ni < 0 || ni >= signal.size()) continue;
				double dw = this->distanceWeight(di);
				double sw = this->similarityWeight(signal[i] - signal[ni]);
				weight += dw * sw;
				mean += dw * sw * signal[ni];
			}
			mean /= weight;
			newSignal.push_back(mean);
		}
		return newSignal;
	}
};