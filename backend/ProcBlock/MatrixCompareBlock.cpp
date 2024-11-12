#include "DiffuseCommon.h"
#include "Common.h"
#include "../Interfaces/IBlock.h"

#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "../MatrImg.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"

// import RasterLayers;
// import IBlock;
// import LayersCore;
//import IAlgorithm
#include "../project.h"
#include "../Algs/Common.h"

//import AlgUtils;
//import BackBind;
// import MatrModule;

#include "../../side/Barcode/PrjBarlib/modules/StateBinFile.h"

class MatrixCompareBlock : public IBlock
{

public:
	MatrixCompareBlock()
	{
		IBlock::settings = {
			// {"Aje", adj},
		};
	}

	virtual const BackString name() const override
	{
		return "MatrixCompareBlock";
	}

	virtual RetLayers execute(InOutLayer iol) override
	{
		execute();

		Project* proj = Project::getProject();
		RasterLayer* layer = proj->addLayerData<RasterLayer>();
		layer->tileOffset = 0;
		layer->mat = resultDiff;

// 		BackImage src;
		return {layer};
	}

	void clear() override
	{
		imgs.clear();
	}

	void addInput(InOutLayer iol) override
	{
		if (imgs.size() < 2)
			addToTrain(iol);
	}

	void addToTrain(InOutLayer iol)
	{
		BackImage& src = imgs.emplace_back();
		getSrcFromInput(iol, src);
	}

	BackImage matr2map(bc::barline* lhs)
	{
		// auto rect = lhs->getBarRect();
		const bc::barvector& lmatr = lhs->getMatrix();

		BackImage img(resultDiff.width(), resultDiff.height(), 1, BarType::FLOAT32_1);
		img.fill(0.f);

		for (size_t i = 0; i < lmatr.size(); i++)
		{
			auto pint = lmatr[i];
			img.set(pint.getX(), pint.getY(), pint.value.getAvgFloat() / 255.0f);
		}
		return img;
	}

	float compare(bc::barline* lhs, bc::barline* rhs)
	{
		BackImage lmap = matr2map(lhs);
		BackImage rmap = matr2map(rhs);

		// Resize by max
		auto rect0 = lhs->getBarRect();
		auto rect1 = rhs->getBarRect();

		int mwid = std::max(rect0.width, rect1.width);
		int mhei = std::max(rect0.height, rect1.height);
		assert(mwid > 0);
		assert(mhei > 0);

		// lmap.resize(mwid, mhei);
		// rmap.resize(mwid, mhei);

		float ratio = static_cast<float>(mwid * mhei) / static_cast<float>(resultDiff.length());
		float diff = 0;
		for (size_t i = 0; i < resultDiff.length(); i++)
		{
			float curDiff = lmap.getLiner(i).absDiff(rmap.getLiner(i)).getAvgFloat() * ratio;
			assert(!isinf(curDiff));
			if (curDiff == 0)
				continue;

			float opp = resultDiff.getLiner(i)[3];
			resultDiff.setOpp(i, std::min(255, static_cast<int>(opp + 255 * curDiff)));

			diff += curDiff;
			assert(!isinf(diff));
		}

		// diff *= ratio;

		if (lhs->getChildrenCount() > rhs->getChildrenCount())
			std::swap(lhs, rhs);

		for (size_t i = 0; i < lhs->getChildrenCount(); i++)
		{
			diff += compare(lhs->getChild(i), rhs->getChild(i));
		}

		bc::barline dummy;
		for (size_t i = lhs->getChildrenCount(); i < rhs->getChildrenCount(); i++)
		{
			diff += compare(&dummy, rhs->getChild(i));
		}

		return diff;
	}

	void execute()
	{
		if (imgs.size() < 2)
			return;

		BackSize size(imgs[0].width(), imgs[0].height());

		bc::BarcodeCreator bcc;
		bc::barstruct constr;// = getConstr(setting);
		constr.createBinaryMasks = true;
		constr.createGraph = true;
		resultDiff = BackImage(imgs[0].width(), imgs[0].height(), 4, BarType::BYTE8_4);
		Barscalar redZeroOpt(255, 0,0,0);
		resultDiff.fill(redZeroOpt);

		bc::Baritem* item0 = (bcc.createBarcode(&imgs[0], constr));
		bc::Baritem* item1 = (bcc.createBarcode(&imgs[1], constr));
		bc::barline* root0 = item0->getRootNode();
		bc::barline* root1 = item1->getRootNode();

		// StateBinFile::BinStateWriter writer;
		// writer.open("matrix.bin");
		// auto bartosave = root0->getChild(0);
		// auto count = writer.pInt(bartosave->matr.size());
		// for (int i = 0; i < count; ++i)
		// {
		// 	auto v = bartosave->matr[i];
		// 	writer.pShort(v.getX());
		// 	writer.pShort(v.getY());
		// 	writer.pShort(v.value.getAvgUchar());
		// }
		// writer.close();

		float diff = compare(root0, root1);
		std::cout << "Done. The diff is " << diff << std::endl;
	}

	BackImage resultDiff;
	std::vector<BackImage> imgs;
};

BlockRegister<MatrixCompareBlock> regMatrixCompareBlock;
