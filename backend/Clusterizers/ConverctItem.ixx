//module;
//#include "../../side/kde.h"
//
//export module ConvertItem;
//
//import ClusterInterface;
//import Platform;
//
//import CachedBarcode;
//import MLSettings;
//
//
//export class ConvertClass : public ICluster
//{
//public:
//	std::vector<float> signature;
//	bc::barvector matrix;
//	bool addMiddleDepth;
//
//	const bc::barvector& getMatrix() const
//	{
//		return matrix;
//	}
//
//	ConvertClass()
//	{ }
//
//	explicit ConvertClass(MLSettings& options, const CachedBarline& root, bc::) //: id(id)
//	{
//		addMiddleDepth = *options.getBool("addMiddleDepth");
//		walk(&line, 0);
//		matrix = line.getMatrix();
//		std::string_view enu = options.getEnum("compress");
//		if (enu == "size")
//		{
//			signature = { (float)signature.size() };
//		}
//		else if (enu == "kde")
//		{
//			short first = signature.front();
//			if (std::all_of(signature.begin(), signature.end(), [first](float x) { return x == first; }))
//			{
//				signature = { signature[0] };
//				return;
//			}
//
//			kdepp::Kde1d<float> kernel(signature);
//			auto result = kernel.eval(*options.getDouble("kdeEps"));
//			signature = { result };
//		}
//		else if (enu == "linearInterpolation")
//		{
//			const int n = *options.getInt("linerInterSize");
//			signature = resizeVectorWithApproximation(signature, n);
//		}
//		else if (enu == "zeros")
//		{
//			const int n = *options.getInt("linerInterSize");
//			if (signature.size() > n)
//			{
//				signature = resizeVectorWithApproximation(signature, n);
//			}
//			else
//			{
//				for (int i = signature.size(); i < n; ++i)
//				{
//					signature.push_back(0);
//				}
//			}
//		}
//	}
//
//	explicit TreeClass(const TreeClass& other) //: id(id)
//	{
//		signature = other.signature;
//		matrix = other.matrix;
//	}
//
//	explicit TreeClass(TreeClass&& other) //: id(id)
//	{
//		signature = std::move(other.signature);
//		matrix = std::move(other.matrix);
//	}
//	//virtual void saveLoadState(StateBinFile::BinState* state) override
//	//{
//	//	// Throw
//	//	state->beginItem();
//	//	size_t arraySize = 0;
//	//	state->beginArray(signature, arraySize);
//	//	for (size_t i = 0; i < arraySize; i++)
//	//	{
//	//		signature[i] = state->pShort(signature[i]);
//	//	}
//	//}
//
//private:
//	void walk(const CachedBarline* line, int depth)
//	{
//		if (line->getChildrenCount() == 0)
//		{
//			signature.push_back(depth);
//		}
//		else
//		{
//			if (addMiddleDepth)
//				signature.push_back(depth);
//
//			//int half = line->getChildrenCount() / 2;
//			for (size_t i = 0, total = line->getChildrenCount(); i < total; i++)
//			{
//				//if (i == half)
//				//	signature.push_back(depth);
//
//				walk(line->getChild(i), depth + 1);
//			}
//		}
//	}
//};
////
//struct landscape
//{
//	int start;
//	int end;
//};
//
//struct lanred
//{
//	std::vector< landscape*> lands;
//};
//
//export class ConvertCollection : public IClusterItemValuesHolder<ConvertClass>
//{
//	using Base = IClusterItemValuesHolder<ConvertClass>;
//protected:
//
//public:
//	ConvertCollection() : Base(false)
//	{
//		Base::settings =
//		{
//			{"addMiddleDepth", true},
//			//{"minSignatureSize", 2},
//			{"compress", {"zeros", "size", "linearInterpolation", "kde", "landscapes", "asIs"}},
//			{"linerInterSize", 100},
//			{"kdeEps", 0.5}
//		};
//	}
//	lanred land[256];
//
//	virtual void addItem(const CachedBarline& line)
//	{
//		//int st = land[line.start().getAvgUchar()];
//		//int ed = land[line.end().getAvgUchar()];
//		//for (int i = st; i < ed; i++)
//		//{
//		//	if (land[i].lands.size())
//		//	{
//
//		//	}
//		//}
//
//	}
//};