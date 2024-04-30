#include "pch.h"
import ConvertItem;


std::vector<float> buildLandscape(std::initializer_list<std::pair<float, float>> list)
{
	int startX = 0;
	float startY = 0;
	std::vector<float> output;
	for (auto& land : list)
	{
		float iter = (land.second - startY) / (land.first * 100 - startX);
		for (size_t i = startX; i < land.first * 100; i++)
		{
			output.push_back(startY);
			startY += iter;
		}
		startX = land.second;
	}

	while (startX < 255)
	{
		output.push_back(0.f);
	}

	return output;
}


struct TestOut
{
	std::vector<std::vector<landres>> expected;

	void compare(ConvertCollection& result)
	{
		for (size_t i = 0; i < result.getItemsCount(); i++)
		{
			auto* col = result.getRItem(i);
			auto& path = col->pathset[0];
			EXPECT_EQ(col->pathset.size(), 1);
			for (size_t j = 0; j <  path.size(); j++)
			{
				EXPECT_EQ(path[j].x, expected[i][j].x);
				EXPECT_EQ(path[j].y, expected[i][j].y);
			}
		}
	}
	void buildexpectedource(std::initializer_list<std::pair<float, float>> list)
	{
		expected.push_back({});
		for (auto& par : list)
		{
			expected.back().push_back({par.first, par.second});
		}
	}
};




TEST(Landscape, TestBubenick)
{
	ConvertCollection col;
	col.convertLand.addExpr(3, 9);
	col.convertLand.addExpr(4, 6);
	col.convertLand.addExpr(5, 11);
	col.perform();

	TestOut bubenic;
	bubenic.buildexpectedource({{3,0},{6,3}, {7,2}, {8,3}, 	{11,0}});
	bubenic.buildexpectedource({{4,0},{5,1}, {5.5,0.5}, {7,2},{9,0}});
	bubenic.buildexpectedource({{5,0},{5.5,0.5}, {6,0}});
	bubenic.compare(col);
}

TEST(Landscape, TestMdpikName)
{
	ConvertCollection mdpiTests;
	mdpiTests.convertLand.addExpr(0.5, 2.5);
	mdpiTests.convertLand.addExpr(2, 6);
	mdpiTests.convertLand.addExpr(3, 5);
	mdpiTests.convertLand.addExpr(5, 7);

	mdpiTests.perform();

	TestOut mdpiExpected;
	mdpiExpected.buildexpectedource({{0.5,0}, {1.5,1},{2.25,0.25}, {4,2}, {5.5,0.5},{6,1},{7,0}});
	mdpiExpected.buildexpectedource({{2,0},{2.25,0.25},{2.5,0}});
	mdpiExpected.buildexpectedource({{3,0}, {4,1},{5,0}});
	mdpiExpected.buildexpectedource({{5,0}, {5.5, 0.5},{6,0}});
	mdpiExpected.compare(mdpiTests);



	// buildexpectedource();
	// buildexpectedource();


	// std::vector < std::pair<float, float>> expected = {
	// 	{3,11},
	// 	{4,9},
	// 	{5,6}
	// };
	// for (size_t i = 0; i < col.getItemsCount(); i++)
	// {
	// 	EXPECT_EQ(col.getRItem(i)->start().getAvgFloat(), expected[i].first);
	// 	EXPECT_EQ(col.getRItem(i)->end().getAvgFloat(), expected[i].second);
	// }
}

TEST(Landscape, SameStart)
{
	ConvertCollection col;
	col.convertLand.addExpr(60, 62);
	col.convertLand.addExpr(60, 84);
	col.convertLand.addExpr(60, 186);
	col.perform();

	TestOut expected;
	expected.buildexpectedource({{60,0},{61,1}, {62,0}});
	expected.buildexpectedource({{60,0},{72,12}, {84,0}});
	expected.buildexpectedource({{60,0},{123,63}, {196,0}});
	expected.compare(col);
}