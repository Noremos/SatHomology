module;

#include "../Bind/Common.h"
#include "../side/sol3/sol.hpp"

export module LuaStates;

import BarcodeModule;
import VectorLayers;
import RasterLayers;
import IOCore;
import ClassifierInterface;

export using CreateVectorLayerFunc = std::function<VectorLayer* ()>;
export using FindVectorLayerByNameFunc = std::function<VectorLayer* (const char* name)>;
export using FindVectorLayerByIdFunc = std::function<VectorLayer* (int id)>;
export using DropLayer = std::function<void(int id)>;

export using FindRasterLayerByNameFunc = std::function<RasterLayer* (const char* name)>;
export using FindRasterLayerByIdFunc = std::function<RasterLayer* (int id)>;

export class LuaState
{
protected:
	sol::state state;

public:
	LuaState()
	{
		state.open_libraries(sol::lib::base);
	}


	void bindBarscalar()
	{
		// Barscalar
		state.new_enum("BarType",
			"BYTE8_1", BarType::BYTE8_1,
			"BYTE8_3", BarType::BYTE8_3,
			"FLOAT32_1", BarType::FLOAT32_1
		);

		state.new_usertype<Barscalar>("Barscalar",
			"type", &Barscalar::type,
			"getAvgUchar", &Barscalar::getAvgUchar,
			"getByte8", &Barscalar::getByte8,
			"getFloat", &Barscalar::getFloat,
			"getRGB", &Barscalar::getRGB
		);
		state.new_usertype<bc::barvalue>("Matrvalue",
			"getX", &bc::barvalue::getX,
			"getY", &bc::barvalue::getY,
			"value", &bc::barvalue::value
		);
	}
};


const char* templ = "function filterCheck(Item)\n\treturn true\nend\n";
export class LuaFilter : public LuaState
{
public:

	void bindItem()
	{
		bindBarscalar();

		state.new_usertype<IClassItem>("ClassItem",
			"getId", &IClassItem::getId,
			"getParentId", &IClassItem::getParentId,
			"getMatrValue", &IClassItem::getMatrValue,
			"getMatrixSize", &IClassItem::getMatrixSize,
			"getDepth", &IClassItem::getDeath,
			"start", &IClassItem::start,
			"end", &IClassItem::end
		);

	}

	void setScript(const char* script)
	{
		state.script(script);
	}

	bool checkFilter(const IClassItem* item) const
	{
		bool res = state["filterCheck"](item);
		return res;
	}
};

export struct ScriptFilterInfo : public IItemFilter
{
private:
	LuaFilter lua;

public:
	ScriptFilterInfo()
	{
		lua.bindItem();
	}

	static BackString getScriptTemplate()
	{
		return templ;
	}

	void setScript(const char* script)
	{
		lua.setScript(script);
	}

	bool pass(const IClassItem* item) const
	{
		return lua.checkFilter(item);
	}
};


export class LuaLayers : public LuaState
{
	void bindVector(CreateVectorLayerFunc& func0, FindVectorLayerByNameFunc& func1, FindVectorLayerByIdFunc& func2, DropLayer& func3)
	{
		state.set_function("createVector", func0);
		state.set_function("findVectorById", func1);
		state.set_function("findVectorByName", func2);
		state.set_function("dropLayer", func3);
	}

	void bindRaster(FindRasterLayerByNameFunc& func0, FindRasterLayerByIdFunc& func1)
	{
		state.set_function("findRaserById", func0);
		state.set_function("findRasterByName", func1);
	}


	void bindLayers()
	{
		state.new_enum("VecType",
			"points", VectorLayer::VecType::points,
			"polygons", VectorLayer::VecType::polygons
		);

		state.new_usertype<BackPoint>("BackPoint",
			"x", &BackPoint::x,
			"y", &BackPoint::y
		);

		state.new_usertype<BackPixelPoint>("BackPixelPoint",
			"x", &BackPixelPoint::x,
			"y", &BackPixelPoint::y
		);

		state.new_usertype<BackColor>("BackColor",
			"r", &BackColor::r,
			"g", &BackColor::g,
			"b", &BackColor::b
		);

		state.new_usertype<DrawPrimitive>("DrawPrimitive",
			"addPoint", &DrawPrimitive::addPoint,
			"clear", &DrawPrimitive::clear
		);


		state.new_usertype<CSBinding>("CSBinding",
			"toGlobal", &CSBinding::toGlobal,
			"toLocal", &CSBinding::toLocal,
			"init", &CSBinding::initProj,
			"getProjId", &CSBinding::getProjId,
			"getScale", &CSBinding::getScale,
			"getScaledEnd", &CSBinding::getScaledEnd
		);

		state.new_usertype<VectorLayer>("VectorLayer",
			"name", &VectorLayer::name,
			"cs", &VectorLayer::cs,
			"vecType", &VectorLayer::vecType,
			"init", &VectorLayer::init,
			"getSysId", &VectorLayer::getSysId,
			"color", &VectorLayer::color,
			"addPrimitive", &VectorLayer::addPrimitive,
			"clear", &VectorLayer::clear
		);

		state.new_usertype<MatrImg>("BackImage",
			"resize", &MatrImg::resize,
			"getRect", &MatrImg::getRect,
			"set", &MatrImg::set,
			"get", &MatrImg::get,
			"channels", &MatrImg::channels,
			"min", &MatrImg::min,
			"max", &MatrImg::max,
			"wid", &MatrImg::wid,
			"hei", &MatrImg::hei,
			"fill", &MatrImg::fill
		);

		state.new_usertype<RasterLayer>("RasterLayer",
			"name", &RasterLayer::name,
			"cs", &RasterLayer::cs,
			"mat", &RasterLayer::mat,
			"displayWidth", &RasterLayer::displayWidth,
			"displayHeight", &RasterLayer::displayHeight,
			"getRect", &RasterLayer::getRect
		);
	}

	void bindBarcode()
	{
		// Bind enum to Lua

		state.new_enum("AttachMode",
			"firstEatSecond", bc::AttachMode::firstEatSecond,
			"secondEatFirst", bc::AttachMode::secondEatFirst,
			"createNew", bc::AttachMode::createNew,
			"dontTouch", bc::AttachMode::dontTouch)
			;

		state.new_enum("CompireStrategy",
			"CommonToLen", bc::CompireStrategy::CommonToLen,
			"CommonToSum", bc::CompireStrategy::CommonToSum)
			;

		state.new_enum("ComponentType",
			"Component", bc::ComponentType::Component,
			"Hole", bc::ComponentType::Hole)
			;

		state.new_enum("ProcType",
			"f0t255", bc::ProcType::f0t255,
			"f255t0", bc::ProcType::f255t0,
			"Radius", bc::ProcType::Radius)
			;

		state.new_enum("ColorType",
			"gray", bc::ColorType::gray,
			"native", bc::ColorType::native,
			"rgb", bc::ColorType::rgb)
			;

		state.new_enum("ReturnType",
			"barcode2d", bc::ReturnType::barcode2d,
			"barcode3d", bc::ReturnType::barcode3d)
			;

		state.new_usertype<bc::point>("Point",
			sol::constructors<bc::point(int, int)>(),
			"x", &bc::point::x,
			"y", &bc::point::y)
			;

		bindBarscalar();
		// !

		//#define TN(NAME) (std::string(NAME)+"8u").c_str()
		//#include "pytemplcalsses.h"
		///////////////////////////////////////////// TYPE /////////////////////



		state.new_usertype<bc::BarRect>("BarRect",
			"x", &bc::BarRect::x,
			"y", &bc::BarRect::y,
			"width", &bc::BarRect::width,
			"height", &bc::BarRect::height,
			"coof", &bc::BarRect::coof,
			"right", &bc::BarRect::right,
			"area", &bc::BarRect::area,
			"isItemInside", &bc::BarRect::isItemInside)
			;

		state.new_usertype<bc::barline>("Barline",
			"start", &bc::barline::start,
			"len", &bc::barline::len,
			"end", &bc::barline::end,
			"getPointsSize", &bc::barline::getPointsSize,
			"getMatrvalue", &bc::barline::getPoint,
			"getRect", &bc::barline::getBarRect,
			"parent", &bc::barline::parent,
			"getChildrenSize", &bc::barline::getChildrenCount,
			"getChild", &bc::barline::getChild);
		//"compire3dbars", &bc::barline::compire3dbars,

		//"get3dList", &bc::barline::getBarcode3d,
		//"get3dSize", &bc::barline::getBarcode3dSize,
		//"get3dValue", &bc::barline::getBarcode3dValue)
		;

		state.new_usertype<bc::Baritem>("Baritem",
			"sum", &bc::Baritem::sum,
			"relen", &bc::Baritem::relen,
			"clone", &bc::Baritem::clone,
			"maxLen", &bc::Baritem::maxLen,
			"removePorog", &bc::Baritem::removePorog,
			"preprocessBar", &bc::Baritem::preprocessBar,
			"cmp", &bc::Baritem::compireFull,
			"cmpOccurrence", &bc::Baritem::compareOccurrence,
			"compireBestRes", &bc::Baritem::compireBestRes,
			//"getBarcode", &bc::Baritem::getBarcode,
			"SortByLineLen", &bc::Baritem::sortByLen,
			"SortByPointsCount", &bc::Baritem::sortBySize,
			//"calcHistByBarlen", &bc::Baritem::calcHistByBarlen,
			"getRootNode", &bc::Baritem::getRootNode
		);
		//;

		state.new_usertype<bc::Barcontainer>("Barcontainer",
			"sum", &bc::Barcontainer::sum,
			"relen", &bc::Barcontainer::relen,
			"clone", &bc::Barcontainer::clone,
			"maxLen", &bc::Barcontainer::maxLen,
			"count", &bc::Barcontainer::count,
			"removePorog", &bc::Barcontainer::removePorog,
			"preprocessBar", &bc::Barcontainer::preprocessBar,
			//"compireCTML", &bc::Barcontainer::compireCTML, args("bc",,
			//"compireCTS", &bc::Barcontainer::compireCTS, args("bc",,
			//"compireCTML", static_cast<float (bc::Barcontainer::*,(const bc::Barbase*, const> (&bc::Barcontainer::compireCTML,, args("bc",,
			//"compireCTS", static_cast<float (bc::Barcontainer::*,(bc::Barbase const*, const>(&bc::Barcontainer::compireCTS,, args("bc",,
			"addItem", &bc::Barcontainer::addItem,
			"getItem", &bc::Barcontainer::getItem)
			;

		state.new_usertype<bc::BarConstructor>("BarConstructor",
			"addStructure", &bc::BarConstructor::addStructure,
			"setPorogStep", &bc::BarConstructor::setStep,
			"setMaxLen", &bc::BarConstructor::setMaxLen,
			"returnType", &bc::BarConstructor::returnType,
			"createBinaryMasks", &bc::BarConstructor::createBinaryMasks,
			"createGraph", &bc::BarConstructor::createGraph,
			"attachMode", &bc::BarConstructor::attachMode,
			"killOnMaxLen", &bc::BarConstructor::killOnMaxLen)
			;

		state.new_usertype<bc::BarcodeCreator>("BarcodeCreator",
			"createBarcode", &bc::BarcodeCreator::createBarcode)
			;
	}
};


//void r()
//{
//	lua.set_function("add_layer", &add);
//	lua.set_function("find_layer", &add);
//	lua.set_function("search_layer", &add);
//}
