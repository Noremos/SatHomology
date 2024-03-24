module;
#include "../../side/kde.h"
#include "Barcode/PrjBarlib/include/barstrucs.h"

export module TreeSignClass;

import ClusterInterface;
import Platform;

import CachedBarcode;
import MLSettings;
import BackBind;

using StgnType = float;

double linearInterpolation(double x1, double y1, double x2, double y2, double x) {
	return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}



std::vector<StgnType> resizeVectorWithApproximation(const std::vector<StgnType>& oldVector, int newSize) {
	std::vector<StgnType> newVector(newSize);

	for (int i = 0; i < newSize; ++i) {
		double indexInOldVector = static_cast<double>(i) / (newSize - 1) * (oldVector.size() - 1);
		int lowerIndex = static_cast<int>(indexInOldVector);
		int upperIndex = lowerIndex + 1;

		if (upperIndex >= oldVector.size()) {
			newVector[i] = oldVector[lowerIndex];
		}
		else {
			newVector[i] = static_cast<int>(linearInterpolation(
				lowerIndex, oldVector[lowerIndex],
				upperIndex, oldVector[upperIndex],
				indexInOldVector
			));
		}
	}

	return newVector;
}


export class TreeClass : public ICluster
{
public:
	std::vector<float> signature;
	bc::barvector matrix;
	bool addMiddleDepth;

	// IClassItem* parent;
	//TreeClass(CachedBarline* line = nullptr) //: id(id)
	//{
	//	if (line)
	//	{
	//		walk(line, 0);
	//	}
	//}

	const bc::barvector& getMatrix() const override
	{
		return matrix;
	}

	TreeClass()
	{ }

	explicit TreeClass(MLSettings& options, const CachedBarline& line) //: id(id)
	{
		addMiddleDepth = *options.getBool("addMiddleDepth");
		walk(&line, 0);
		matrix = line.getMatrix();
		std::string_view enu = options.getEnum("compress");
		if (enu == "size")
		{
			signature = { (float)signature.size() };
		}
		else if (enu == "kde")
		{
			short first = signature.front();
			if (std::all_of(signature.begin(), signature.end(), [first](float x) { return x == first; }))
			{
				signature = { signature[0] };
				return;
			}

			kdepp::Kde1d<float> kernel(signature);
			auto result = kernel.eval(*options.getDouble("kdeEps"));
			signature = { result };
		}
		else if (enu == "linearInterpolation")
		{
			const int n = *options.getInt("linerInterSize");
			signature = resizeVectorWithApproximation(signature, n);
		}
		else if (enu == "zeros")
		{
			const int n = *options.getInt("linerInterSize");
			if (signature.size() > n)
			{
				signature = resizeVectorWithApproximation(signature, n);
			}
			else
			{
				for (int i = signature.size(); i < n; ++i)
				{
					signature.push_back(0);
				}
			}
		}
	}

	explicit TreeClass(const TreeClass& other) //: id(id)
	{
		signature = other.signature;
		matrix = other.matrix;
	}

	explicit TreeClass(TreeClass&& other) //: id(id)
	{
		signature = std::move(other.signature);
		matrix = std::move(other.matrix);
	}

	void getSignature(BackString& line) const override
	{
		for (size_t i = 0; i < signature.size(); i++)
		{
			line += signature[i];
			line += " ";
		}
	}

	//virtual void saveLoadState(StateBinFile::BinState* state) override
	//{
	//	// Throw
	//	state->beginItem();
	//	size_t arraySize = 0;
	//	state->beginArray(signature, arraySize);
	//	for (size_t i = 0; i < arraySize; i++)
	//	{
	//		signature[i] = state->pShort(signature[i]);
	//	}
	//}

private:
	void walk(const CachedBarline* line, int depth)
	{
		if (line->getChildrenCount() == 0)
		{
			signature.push_back(depth);
		}
		else
		{
			if (addMiddleDepth)
				signature.push_back(depth);

			//int half = line->getChildrenCount() / 2;
			for (size_t i = 0, total = line->getChildrenCount(); i < total; i++)
			{
				//if (i == half)
				//	signature.push_back(depth);

				walk(line->getChild(i), depth + 1);
			}
		}
	}
};
//

export class TreeSignatureCollection : public IClusterItemValuesHolder<TreeClass>
{
	using Base = IClusterItemValuesHolder<TreeClass>;
protected:

public:
	TreeSignatureCollection() : Base(false)
	{
		Base::settings =
		{
			{"addMiddleDepth", true},
			//{"minSignatureSize", 2},
			{"compress", {"zeros", "size", "linearInterpolation", "kde", "landscapes", "asIs"}},
			{"linerInterSize", 100},
			{"kdeEps", 0.5}
		};
	}

	virtual void addItem(const CachedBarline& item)
	{
		TreeClass clusterItem(Base::settings, item);
		//if (clusterItem.signature.size() > *Base::settings.getInt("minSignatureSize"))
		{
			Base::items.push_back(std::move(clusterItem));
		}
	}
};
