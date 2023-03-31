//  module;

// #include "../side/sol3/sol.hpp"

// export module Lua;

// import BarcodeModule;
// import RasterLayers;

// export
// {
// 	using LuaState = sol::state;

// }

// export void bindBarcode(sol::state& lua)
// {
// 	lua.open_libraries(sol::lib::base);

// 	// Bind enum to Lua

// 	lua.new_enum("AttachMode",
// 		"firstEatSecond", bc::AttachMode::firstEatSecond,
// 		"secondEatFirst", bc::AttachMode::secondEatFirst,
// 		"createNew", bc::AttachMode::createNew,
// 		"dontTouch", bc::AttachMode::dontTouch)
// 		;

// 	lua.new_enum("CompireStrategy",
// 		"CommonToLen", bc::CompireStrategy::CommonToLen,
// 		"CommonToSum", bc::CompireStrategy::CommonToSum)
// 		;

// 	lua.new_enum("ComponentType",
// 		"Component", bc::ComponentType::Component,
// 		"Hole", bc::ComponentType::Hole)
// 		;

// 	lua.new_enum("ProcType",
// 		"f0t255", bc::ProcType::f0t255,
// 		"f255t0", bc::ProcType::f255t0,
// 		"Radius", bc::ProcType::Radius)
// 		;

// 	lua.new_enum("ColorType",
// 		"gray", bc::ColorType::gray,
// 		"native", bc::ColorType::native,
// 		"rgb", bc::ColorType::rgb)
// 		;

// 	lua.new_enum("ReturnType",
// 		"barcode2d", bc::ReturnType::barcode2d,
// 		"barcode3d", bc::ReturnType::barcode3d)
// 		;

// 	lua.new_usertype<bc::point>("Point",
// 		sol::constructors<bc::point(int, int)>(),
// 		"x",&bc::point::x,
// 		"y", &bc::point::y)
// 		;

// 	//#define TN(NAME) (std::string(NAME)+"8u").c_str()
// 	//#include "pytemplcalsses.h"
// 	///////////////////////////////////////////// TYPE /////////////////////

// 	lua.new_usertype<bc::barvalue>("Matrvalue",
// 		"getX", & bc::barvalue::getX,
// 		"getY",&bc::barvalue::getY,
// 		"value", &bc::barvalue::value)
// 		;


// 	lua.new_usertype<bc::BarRect>("BarRect",
// 		"x", &bc::BarRect::x,
// 		"y", &bc::BarRect::y,
// 		"width", &bc::BarRect::width,
// 		"height", &bc::BarRect::height,
// 		"coof", &bc::BarRect::coof,
// 		"right", & bc::BarRect::right,
// 		"area", & bc::BarRect::area,
// 		"isItemInside", &bc::BarRect::isItemInside)
// 		;

// 	lua.new_usertype<bc::barline>("Barline",
// 		"start", &bc::barline::start,
// 		"len", &bc::barline::len,
// 		"end", &bc::barline::end,
// 		"getPointsSize", &bc::barline::getPointsSize,
// 		"getMatrvalue", &bc::barline::getPoint,
// 		"getRect", &bc::barline::getBarRect,
// 		"parent", &bc::barline::parent,
// 		"getChildrenSize", &bc::barline::getChildrenCount,
// 		"getChild", & bc::barline::getChild);
// 		//"compire3dbars", &bc::barline::compire3dbars,

// 		//"get3dList", &bc::barline::getBarcode3d,
// 		//"get3dSize", &bc::barline::getBarcode3dSize,
// 		//"get3dValue", &bc::barline::getBarcode3dValue)
// 		;

// 	lua.new_usertype<bc::Baritem>("Baritem",
// 		"sum", &bc::Baritem::sum,
// 		"relen", &bc::Baritem::relen,
// 		"clone", &bc::Baritem::clone,
// 		"maxLen", &bc::Baritem::maxLen,
// 		"removePorog", &bc::Baritem::removePorog,
// 		"preprocessBar", &bc::Baritem::preprocessBar,
// 		"cmp", &bc::Baritem::compireFull,
// 		"cmpOccurrence", &bc::Baritem::compareOccurrence,
// 		"compireBestRes", &bc::Baritem::compireBestRes,
// 		//"getBarcode", &bc::Baritem::getBarcode,
// 		"SortByLineLen", &bc::Baritem::sortByLen,
// 		"SortByPointsCount", &bc::Baritem::sortBySize,
// 		//"calcHistByBarlen", &bc::Baritem::calcHistByBarlen,
// 		"getRootNode", &bc::Baritem::getRootNode)
// 		;

// 	lua.new_usertype<bc::Barcontainer>("Barcontainer",
// 		"sum", &bc::Barcontainer::sum,
// 		"relen", &bc::Barcontainer::relen,
// 		"clone", &bc::Barcontainer::clone,
// 		"maxLen", &bc::Barcontainer::maxLen,
// 		"count", &bc::Barcontainer::count,
// 		"removePorog", &bc::Barcontainer::removePorog,
// 		"preprocessBar", &bc::Barcontainer::preprocessBar,
// 		//"compireCTML", &bc::Barcontainer::compireCTML, args("bc",,
// 		//"compireCTS", &bc::Barcontainer::compireCTS, args("bc",,
// 		//"compireCTML", static_cast<float (bc::Barcontainer::*,(const bc::Barbase*, const> (&bc::Barcontainer::compireCTML,, args("bc",,
// 		//"compireCTS", static_cast<float (bc::Barcontainer::*,(bc::Barbase const*, const>(&bc::Barcontainer::compireCTS,, args("bc",,
// 		"addItem", &bc::Barcontainer::addItem,
// 		"getItem", &bc::Barcontainer::getItem)
// 		;

// 	lua.new_usertype<bc::BarConstructor>("BarConstructor",
// 		"addStructure", &bc::BarConstructor::addStructure,
// 		"setPorogStep", &bc::BarConstructor::setStep,
// 		"setMaxLen", &bc::BarConstructor::setMaxLen,
// 		"returnType", &bc::BarConstructor::returnType,
// 		"createBinaryMasks", &bc::BarConstructor::createBinaryMasks,
// 		"createGraph", &bc::BarConstructor::createGraph,
// 		"attachMode", &bc::BarConstructor::attachMode,
// 		"killOnMaxLen", &bc::BarConstructor::killOnMaxLen)
// 		;

// 	lua.new_usertype<bc::BarcodeCreator>("BarcodeCreator",
// 		"createBarcode", &bc::BarcodeCreator::createBarcode)
// 		;
// }


// //void r()
// //{
// //	lua.set_function("add_layer", &add);
// //	lua.set_function("find_layer", &add);
// //	lua.set_function("search_layer", &add);
// //}
