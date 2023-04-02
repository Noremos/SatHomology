#include "../Bind/Common.h"
#include "../side/sol3/sol.hpp"

//
import IOCore;
import ClassifierInterface;

class LuaState
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
class LuaFilter : public LuaState
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

struct ScriptFilterInfo : public IItemFilter
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




//void r()
//{
//	lua.set_function("add_layer", &add);
//	lua.set_function("find_layer", &add);
//	lua.set_function("search_layer", &add);
//}
