#include "pch.h"
#include "../backend/Clusterizers/ConverctItem.h"
// import ConvertItem;
import BackBind;


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

	void compare(ConvertClass& result)
	{
		EXPECT_EQ(result.pathset.size(), expected.size());

		for (size_t i = 0; i < result.pathset.size(); i++)
		{
			EXPECT_EQ(result.pathset[i].size(), expected[i].size());

			for (size_t j = 0; j <  result.pathset[i].size(); j++)
			{
				EXPECT_EQ(result.pathset[i][j].x, expected[i][j].x);
				EXPECT_EQ(result.pathset[i][j].y, expected[i][j].y);
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
	col.performOnPerform();
	ConvertClass out;
	col.convert(col.convertLand, &out);

	TestOut bubenic;
	bubenic.buildexpectedource({{3,0},{6,3}, {7,2}, {8,3}, 	{11,0}});
	bubenic.buildexpectedource({{4,0},{5,1}, {5.5,0.5}, {7,2},{9,0}});
	bubenic.buildexpectedource({{5,0},{5.5,0.5}, {6,0}});
	bubenic.compare(out);
}

TEST(Landscape, TestMdpikName)
{
	// https://www.mdpi.com/2073-431X/12/6/110

	ConvertCollection mdpiTests;
	mdpiTests.convertLand.addExpr(0.5, 2.5);
	mdpiTests.convertLand.addExpr(2, 6);
	mdpiTests.convertLand.addExpr(3, 5);
	mdpiTests.convertLand.addExpr(5, 7);

	mdpiTests.performOnPerform();
	ConvertClass out;
	mdpiTests.convert(mdpiTests.convertLand, &out);

	TestOut mdpiExpected;
	mdpiExpected.buildexpectedource({{0.5,0}, {1.5,1},{2.25,0.25}, {4,2}, {5.5,0.5},{6,1},{7,0}});
	mdpiExpected.buildexpectedource({{2,  0}, {2.25,0.25},{2.5,0}});
	mdpiExpected.buildexpectedource({{3,  0}, {4,1},      {5,0}});
	mdpiExpected.buildexpectedource({{5,  0}, {5.5, 0.5}, {6,0}});
	mdpiExpected.compare(out);

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
	col.performOnPerform();
	ConvertClass out;
	col.convert(col.convertLand, &out);

	TestOut expected;
	expected.buildexpectedource({{60,0},{61,1}, {62,0}});
	expected.buildexpectedource({{60,0},{72,12}, {84,0}});
	expected.buildexpectedource({{60,0},{123,63}, {186,0}});
	expected.compare(out);
}

TEST(Landscape, PaperTest)
{
	// https://www.sciencedirect.com/science/article/pii/S0747717116300104
	ConvertCollection col;
	col.convertLand.addExpr(1, 5);
	col.convertLand.addExpr(2, 8);
	col.convertLand.addExpr(3, 4);
	col.convertLand.addExpr(5, 9);
	col.convertLand.addExpr(6, 7);

	col.performOnPerform();
	ConvertClass out;
	col.convert(col.convertLand, &out);

	TestOut expected;
	expected.buildexpectedource({{1,0}, {3,   2},   {3.5,1.5}, {5,3}, {6.5,1.5}, {7,2}, {9,0}});
	expected.buildexpectedource({{2,0}, {3.5, 1.5}, {5,0}});
	expected.buildexpectedource({{3,0}, {3.5, 0.5}, {4,0}});
	expected.buildexpectedource({{5,0}, {6.5, 1.5}, {8,0}});
	expected.buildexpectedource({{6,0}, {6.5, 0.5}, {7,0}});
	expected.compare(out);
}


TEST(LandScape, signature)
{
	// https://www.sciencedirect.com/science/article/pii/S0747717116300104
	ConvertCollection col;
	col.convertLand.addExpr(1, 5);
	col.convertLand.addExpr(2, 8);
	col.convertLand.addExpr(3, 4);
	col.convertLand.addExpr(5, 9);
	col.convertLand.addExpr(6, 7);
	col.maxEnd = 9;

	col.performOnPerform();
	col.perform();

	std::vector<std::string> expectedSignature = {
		{"0.000000 0.000000 1.000000 2.000000 2.250000 3.000000 2.250000 2.000000 1.000000 "},
		{"0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000"},  // small
		{"0.000000 0.000000 0.000000 0.750000 0.750000 0.000000 0.000000 0.000000 0.000000 "},
		{"0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.750000 0.750000 0.000000 "},
		{"0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000"}, // small
	};


	ASSERT_EQ(col.getItemsCount(), expectedSignature.size());
	for (size_t i = 0; i < col.getItemsCount(); i++)
	{
		ConvertClass* item = col.getRItem(i);
		item->AddE = 1;

		std::string signature;
		item->getSignature(signature);

		// ASSERT_EQ(signature, expectedSignature[i]);
	}
}
