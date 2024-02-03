module;

#include <cassert>
#include <algorithm>
#include <string>

export module Exp;
import LayersCore;
import ProjectModule;
import RasterBarHolderRLayer;
import VectorLayers;
import BarTypes;
import BarcodeModule;
import RasterLayers;
import Classifiers;
import GeoprocessorModule;
import EnergyModule;
import IItemModule;
import BackBind;
import MatrModule;


class TreeWalk
{
public:
	TreeVectorLayer* layer;
	VectorLayer* layerRect;
	RasterLayer* rasterSpot;
	IItemFilter* filter;

	VecTree addTree(bc::barline* line)
	{
		VecTree guiTree;

		BettylineClass cls(line);
		if (filter != nullptr && !filter->pass(&cls))
			return guiTree;

		BackColor pointCol = BackColor::random();
		Barscalar barclo(pointCol.r, pointCol.g, pointCol.b);
		BackImage& imgout = rasterSpot->mat;
		for (auto& p : line->matr)
		{
			imgout.set(p.x, p.y, barclo);
		}

		const int childrenSize = line->getChildrenCount();
		for (int i = 0; i < childrenSize; i++)
		{
			VecTree t = addTree(line->getChild(i));
			const bool passed = t.inited();
			if (passed)
			{
				guiTree.children.push_back(t);
				//printf("%d\n", guiTree.children.size());
			}
		}

		/*if (guiTree.children.size() < 2)
		{
			return VecTree();
		}*/


		// Barscalar pointCol = RasterLineLayer::colors[rand() % RasterLineLayer::colors.size()];
		//line->getChildsMatr(line->matr, true);
		//line->getChildsMatr(line->matr, true);

		if (line->matr.size() == 0)
			return guiTree;

		std::vector<uint> out;
		auto rect = getCountourOder(line->matr, out, true);
		if (out.size() == 0)
		{
			//return guiTree;
			//for (const auto& pm : line->matr)
			//{
			//	out.push_back(bc::barvalue::getStatInd(pm.getX(), pm.getY()));
			//}
		}


		DrawPrimitive* p = layer->addPrimitive(pointCol);
		guiTree.primId = p->id;
		guiTree.size = line->matr.size();
		for (const auto& pm : out)
		{
			auto op = bc::barvalue::getStatPoint(pm);
			//BackPoint iglob(static_cast<float>(op.x) + 0.5f, static_cast<float>(op.y) + 0.5f);
			BackPoint iglob(static_cast<float>(op.x), static_cast<float>(op.y));

			iglob = layer->cs.toGlobal((float)iglob.x, (float)iglob.y); // To real
			p->addPoint(iglob);
		}

		p = layerRect->addPrimitive(pointCol);
		p->addPoint(rect.topLeft);
		p->addPoint(rect.topRight());
		p->addPoint(rect.getBottomRight());
		p->addPoint(rect.bottomLeft());


		if (guiTree.children.size() < 2)
		{
			//guiTree.size = -1;
		}

		return guiTree;
	}

};


export RetLayers exeGUI(InOutLayer& iol, const BarcodeProperies& propertices, IItemFilter* filter)
{
	Project* proj = Project::getProject();

	//if (u_displayFactor < 1.0)
	//	throw std::exception();
	IRasterLayer* input = proj->getInRaster(iol);

	RetLayers ret;
	TreeVectorLayer* layer = proj->addOrUpdateOut<TreeVectorLayer>(iol, input->cs.getProjId());
	layer->color = BackColor::random();
	layer->vecType = VectorLayer::VecType::polygons;
	layer->initCSFrom(input->cs);
	layer->name = "Source bounds";

	auto* layerRect = proj->addLayerData<VectorLayer>(input->cs.getProjId());
	layerRect->color = BackColor::random();
	layerRect->vecType = VectorLayer::VecType::polygons;
	layerRect->initCSFrom(input->cs);
	layerRect->name = "Rect bounds";

	RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>(input->cs.getProjId());
	rasterSpot->initCSFrom(input->cs);
	rasterSpot->init(input);

	//ret.push_back(rasterSpot);
	ret.push_back(layer);
	//ret.push_back(layerRect);
	// layer->init(input);

	BackImage src = *(input->getCachedImage());

	bc::barstruct constr = propertices.barstruct;
	constr.createBinaryMasks = true;
	constr.createGraph = true;
	constr.attachMode = bc::AttachMode::morePointsEatLow;
	constr.returnType = bc::ReturnType::barcode2d;
	// constr.maxRadius = 10;
	//constr.maxLen.set(15);

	bc::BarcodeCreator bc;
	std::unique_ptr<bc::Baritem> item(bc.createBarcode(&src, constr));

	//std::unique_ptr<bc::Baritem> item(bc::Eater::createBarcode(&src));

	rasterSpot->mat = src;
	if (filter)
		filter->imgLen = src.length();

	RasterLineLayer(); // init colors

	auto* root = item->getRootNode();

	TreeWalk helper;
	helper.layer = layer;
	helper.layerRect = layerRect;
	helper.rasterSpot = rasterSpot;
	helper.filter = filter;
	assert(root->childrenId.size() == 1);
	layer->tree = helper.addTree(root->getChild(0));

	return ret;
}


void ResizeAspect(BackSize& size, const BackSize maxSize)
{
	// Calculate the aspect ratio of the image
	double aspect_ratio = static_cast<float>(size.wid) / size.hei;
	double max_aspect_width = static_cast<float>(maxSize.hei) * aspect_ratio;
	double max_aspect_height = static_cast<float>(maxSize.wid) / aspect_ratio;
	size.wid = (int)std::min((double)maxSize.wid, max_aspect_width);
	size.hei = (int)std::min((double)maxSize.hei, max_aspect_height);
}


template<class T>
constexpr T mmmin(T a, T b)
{
	return a > b ? b : a;
}

template<class T>
constexpr T mmmax(const T a, const T b)
{
	return a > b ? a : b;
}

void getMod(BackPixelPoint& start, BackPixelPoint& end, BackPixelPoint p, BackSize size, float aspectX, float aspectY)
{
	const float xa = static_cast<float>(p.x) * aspectX;
	start.x = (int)round(xa);
	end.x = (int)round(mmmin<float>(xa + aspectX + 1, size.wid) - xa);

	const float ya = static_cast<float>(p.y) * aspectY;
	start.y = (int)round(ya);
	end.y = (int)mmmin<int>(round(ya + aspectY) + 1, size.hei);
}

export RetLayers exeQuadro(IRasterLayer* input, bc::ProcType type)
{
	Project* proj = Project::getProject();

	RetLayers ret;
	const BackImage& src = *(input->getCachedImage());

	bc::barstruct constr;
	constr.createBinaryMasks = true;
	constr.createGraph = false;
	constr.returnType = bc::ReturnType::barcode2d;


	bc::BarcodeCreator bcc;

	const BackSize srcsize(src.width(), src.height());
	//BackSize b = srcsize;
	int maskMin = 0;

	BackSize b(4, 4);
	BackSize imgSize(srcsize.wid, srcsize.hei);
	ResizeAspect(imgSize, b);

	BackImage mask(imgSize.wid, imgSize.hei, 4);
	mask.reintAsInt();
	mask.fill(0);


	constr.addStructure(type, bc::ColorType::native, bc::ComponentType::Component);
	constr.maskId = 0;
	constr.mask = &mask;

	bc::BarConstructor constHolder;
	constHolder.structs.push_back(constr);

	while (true)
	{
		BackImage imgin = src;
		imgin.resize(imgSize.wid, imgSize.hei);


		RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>(input->cs.getProjId());
		rasterSpot->initCSFrom(input->cs);
		rasterSpot->init(srcsize.wid, srcsize.hei, 3);
		ret.push_back(rasterSpot);
		BackImage& out = rasterSpot->mat;
		const float aspectX = static_cast<float>(srcsize.wid) / imgSize.wid;
		const float aspectY = static_cast<float>(srcsize.hei) / imgSize.hei;


		//constr.
		//if (constr.structure.size() == 0)
		//	break;
		assert(imgin.length() == mask.length());
		std::unique_ptr<bc::Barcontainer> containner(bcc.createBarcode(&imgin, constHolder));

		constHolder.structs.clear();
		maskMin = 0;

		b.wid *= 4;
		b.hei *= 4;
		imgSize = BackSize(srcsize.wid, srcsize.hei);

		if (b.wid > src.width() || b.hei > src.height())
		{
			b.wid = src.width();
			b.hei = src.height();
		}
		else
		{
			ResizeAspect(imgSize, b);
		}

		mask.resize(imgSize.wid, imgSize.hei);
		mask.fill(0);
		const float maskAspectX = static_cast<float>(mask.width()) / imgin.width();
		const float maskAspectY = static_cast<float>(mask.height()) / imgin.height();


		for (size_t ci = 0; ci < containner->count(); ci++)
		{
			bc::Baritem* item = containner->getItem(0);

			for (size_t i = 0; i < item->barlines.size(); ++i)
			{
				const auto& matr = item->barlines[i]->matr;

				//if (matr.size() < 5)
				//	continue;

				bc::barstruct bst = constr;
				bst.maskId = i;
				constHolder.structs.push_back(bst);

				// if (item->barlines[i]->len() < 10)
				const auto randCol = BackColor::random();
				const Barscalar rcol(randCol.r, randCol.g, randCol.b);
				const Barscalar rid(i, BarType::INT32_1);

				// BackSize maskSize = b;
				for (const auto& pm : matr)
				{
					BackPixelPoint pix(pm.getX(), pm.getY());
					BackPixelPoint start, end;
					getMod(start, end, pix, srcsize, aspectX, aspectY);
					for (size_t l = start.y; l < end.y; ++l)
					{
						// end.x is a length
						out.setRow(start.x, l, end.x, rcol);
					}

					getMod(start, end, pix, imgSize, maskAspectX, maskAspectY);

					for (size_t l = start.y; l < end.y; ++l)
					{
						mask.setRow(start.x, l, end.x, rid);
					}
				}
			}
		}


		//if (b.wid < 8 && b.hei < 6)
		//	break;
		if (b.wid == src.width() && b.hei == src.height())
			break;

		////b.wid /= 2;
		////b.hei /= 2;
		//b.wid *= 2;
		//b.hei *= 2;
		//if (b.wid > src.width() || b.hei > src.height())
		//{
		//	b.wid = src.width();
		//	b.hei = src.height();
		//}
	}

	//std::reverse(ret.begin(), ret.end());
	return ret;
}


// Linear interpolation function
Barscalar lerp(double t)
{
	if (t < 0)
		t = 0;

	assert(t <= 1.0 && t >= 0);
	//t = log(1 + t);
	const BackColor start(0, 255, 255);
	const BackColor end(255, 0, 0);
	uint8_t r = static_cast<uint8_t>(start.r + t * (end.r - start.r));
	uint8_t g = static_cast<uint8_t>(start.g + t * (end.g - start.g));
	uint8_t b = static_cast<uint8_t>(start.b + t * (end.b - start.b));
	return Barscalar(r, g, b);
}

export RetLayers exeEnergy(InOutLayer& iol, bc::ProcType type, float energyStart, bool useCells = false)
{
	Project* proj = Project::getProject();

	IRasterLayer* input = proj->getInRaster(iol);

	RetLayers ret;
	const BackImage& srcl = *(input->getCachedImage());
	const BackSize srcsize(srcl.width(), srcl.height());
	float aspect = 1.f;

	BackImage src(srcsize.wid, srcsize.hei, input->getRect(0, 0, 1, 1).get(0, 0).type);
	if (input->realWidth() != srcsize.wid)
	{
		aspect = static_cast<float>(input->realWidth()) / srcsize.wid;
		for (int h = 0; h < srcsize.hei; ++h)
		{
			for (int i = 0; i < srcsize.wid; ++i)
			{
				src.set(i, h, input->getRect(i * aspect, h * aspect, 1, 1).get(0, 0));
			}
		}
	}
	else
		src = srcl;

	bc::barstruct constr;
	constr.createBinaryMasks = true;
	constr.createGraph = false;
	constr.attachMode = bc::AttachMode::morePointsEatLow;
	//constr.attachMode = bc::AttachMode::closer;
	constr.returnType = bc::ReturnType::barcode2d;
	constr.energyStart = energyStart;

	bc::BarcodeCreator bcc;

	//BackSize b = srcsize;
	int maskMin = 0;

	constr.addStructure(type, bc::ColorType::native, bc::ComponentType::Component);

	RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>(input->cs.getProjId());
	rasterSpot->initCSFrom(input->cs);
	rasterSpot->aspect = aspect;
	rasterSpot->init(srcsize.wid, srcsize.hei, 3);
	ret.push_back(rasterSpot);

	//bool useEmbeded = false;

	BackImage& out = rasterSpot->mat;
	if (useCells)
	{
		/*	bcc.maxe = energyStart;
			std::unique_ptr<bc::Barcontainer> containner(bcc.createBarcode(&src, constr));

			for (size_t i = 0; i < src.length(); i++)
			{
				out.setLiner(i, lerp(static_cast<float>(bcc.energy[i]) / static_cast<float>(bcc.maxe)));
			}*/

		CellBarcode bce;
		std::unique_ptr<bc::Baritem> containner(bce.run(&src, constr, energyStart));

		bc::Baritem* item = containner.get();
		for (size_t i = 0; i < item->barlines.size(); ++i)
		{
			const auto& matr = item->barlines[i]->matr;

			// BackSize maskSize = b;
			for (const auto& pm : matr)
			{
				out.set(pm.getX(), pm.getY(), lerp(pm.value.getAvgFloat()));
				//out.set(pm.getX(), pm.getY(), pm.value.getAvgUchar());
			}
		}
	}
	else
	{
		EnetrgyBarcode eb;
		float* outenergy = eb.run(&src, constr, energyStart);
		for (size_t i = 0; i < src.length(); i++)
		{
			out.setLiner(i, lerp(outenergy[i] / static_cast<float>(energyStart)));
		}
		delete[] outenergy;

	}
	return ret;
}


export void exe3d(IRasterLayer* input, bc::ProcType type)
{
	const BackImage& src = *(input->getCachedImage());

	bc::barstruct constr;
	constr.createBinaryMasks = true;
	constr.createGraph = false;
	constr.returnType = bc::ReturnType::barcode3d;

	bc::BarcodeCreator bcc;

	constr.addStructure(type, bc::ColorType::native, bc::ComponentType::Component);

	std::unique_ptr<bc::Baritem> item(bcc.createBarcode(&src, constr));

	std::string globout = "";
	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		auto& counter = *item->barlines[i]->bar3d;
		std::string outstr = "";
		for (const auto& pm : counter)
		{
			outstr += std::to_string(pm.cx);
			outstr += " ";
			outstr += std::to_string(pm.cy);
			outstr += " ";
			outstr += std::to_string(pm.rat);
			outstr += "|";
		}
		globout += outstr + "\r\n";
	}
	WriteFile("D:\\12.txt", globout);
}



export RetLayers exeFilter(InOutLayer& iol, bc::ProcType type, int algNum)
{
	Project* proj = Project::getProject();
	//if (u_displayFactor < 1.0)
	//	throw std::exception();
	IRasterLayer* input = proj->getInRaster(iol);

	switch (algNum)
	{
	case 0:
		break;
	case 1:
		return exeQuadro(input, type);
		//case 2:
			//return exeEnergy(input, type, 100);
	case 3:
		exe3d(input, type);
		return {};
	}

	RetLayers ret;
	RasterLayer* layer = proj->addOrUpdateOut<RasterLayer>(iol, input->cs.getProjId());
	layer->initCSFrom(input->cs);

	ret.push_back(layer);
	layer->init(input);



	const BackImage src = *(input->getCachedImage());

	uint hist[256];//256
	uint offs[256];//256
	std::fill_n(hist, 256, 0);
	std::fill_n(offs, 256, 0);
	for (size_t i = 0; i < src.length(); i++)
	{
		auto p = (int)src.getLiner(i);
		++hist[p];//����� vector, �� ��
	}

	for (size_t i = 1; i < 256; ++i)
	{
		hist[i] += hist[i - 1];
		offs[i] = hist[i - 1];
	}

	std::unique_ptr<uint> ods;
	uint* data = new uint[src.length()];//256
	ods.reset(data);

	for (size_t i = 0; i < src.length(); i++)
	{
		uchar p = src.getLiner(i).getAvgUchar();
		data[offs[p]++] = i;
	}

	//std::reverse(data, data + src.length());

	std::vector<char> setted;
	setted.resize(src.length());
	std::fill(setted.begin(), setted.end(), 0);

	BackImage& imgout = layer->mat = src;

	//.width(), src.hei(), src.channels());
	//imgout.fill(0);

	const char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };
	for (size_t i = 0; i < src.length(); i++)
	{
		auto dat = data[src.length() - i - 1];
		auto p = bc::barvalue::getStatPoint(dat, src.width());
		if (setted[i] == 10)
		{
			continue;
		}

		Barscalar val = imgout.getLiner(dat);

		for (int u = 0; u < 8; ++u)
		{
			bc::point IcurPoint(p + poss[u]);

			if (IcurPoint.x < 0 || IcurPoint.x >= src.width() || IcurPoint.y < 0 || IcurPoint.y >= src.height())
				continue;

			auto re = IcurPoint.getLiner(src.wid());

			//if (setted[re] == 10)
			//{
			//	continue;
			//}

			Barscalar valNext = imgout.get(IcurPoint.x, IcurPoint.y);
			if (valNext.absDiff(val) < 15)
				imgout.set(IcurPoint.x, IcurPoint.y, val);
			else
			{
				setted[re] = 10;
				//imgout.set(IcurPoint.x, IcurPoint.y, 0);
			}
		}
	}

	return ret;
}
