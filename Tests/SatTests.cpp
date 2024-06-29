#include "pch.h"
#include "../backend/Clusterizers/LandscapeItem.h"
// import ConvertItem;
// import BackBind;


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
	std::vector<std::vector<LandPoint>> expected;

	void compare(LandscapeClass& result)
	{
		EXPECT_EQ(result.landscape.size(), expected.size());

		int minl = std::min(result.landscape.size(), expected.size());
		for (size_t i = 0; i < minl; i++)
		{
			EXPECT_EQ(result.landscape[i].size(), expected[i].size());

			int minn = std::min(result.landscape[i].size(), expected[i].size());
			for (size_t j = 0; j <  minn; j++)
			{
				EXPECT_EQ(result.landscape[i].points[j].x, expected[i][j].x);
				EXPECT_EQ(result.landscape[i].points[j].y, expected[i][j].y);
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




TEST(InputLandData, TestBubenick)
{
	LandscapeCollection col;
	col.setTestEnv();

	col.convertLand.addExprRaw(3, 9);
	col.convertLand.addExprRaw(4, 6);
	col.convertLand.addExprRaw(5, 11);
	col.performOnPerform();
	LandscapeClass out;
	col.convertToLandscape(col.convertLand, &out);

	TestOut bubenic;
	bubenic.buildexpectedource({{3,0},{6,3}, {7,2}, {8,3}, 	{11,0}});
	bubenic.buildexpectedource({{4,0},{5,1}, {5.5,0.5}, {7,2},{9,0}});
	bubenic.buildexpectedource({{5,0},{5.5,0.5}, {6,0}});
	bubenic.compare(out);
}

TEST(InputLandData, TestMdpikName)
{
	// https://www.mdpi.com/2073-431X/12/6/110

	LandscapeCollection mdpiTests;
	mdpiTests.setTestEnv();
	mdpiTests.convertLand.addExprRaw(0.5, 2.5);
	mdpiTests.convertLand.addExprRaw(2, 6);
	mdpiTests.convertLand.addExprRaw(3, 5);
	mdpiTests.convertLand.addExprRaw(5, 7);

	mdpiTests.performOnPerform();
	LandscapeClass out;
	mdpiTests.convertToLandscape(mdpiTests.convertLand, &out);

	TestOut mdpiExpected;
	mdpiExpected.buildexpectedource({{0.5,0}, {1.5,1},{2.25,0.25}, {4,2}, {5.5,0.5},{6,1},{7,0}});
	mdpiExpected.buildexpectedource({{2,  0}, {2.25,0.25},{2.5,0},
		{3, 0},  {4,1},  {5,0},
		{5.5, 0.5}, {6,0}});
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

TEST(InputLandData, SameStart)
{
	LandscapeCollection col;
	col.setTestEnv();
	col.convertLand.addExprRaw(60, 62);
	col.convertLand.addExprRaw(60, 84);
	col.convertLand.addExprRaw(60, 186);
	col.performOnPerform();
	LandscapeClass out;
	col.convertToLandscape(col.convertLand, &out);

	TestOut expected;
	expected.buildexpectedource({{60,0},{123,63}, {186,0}});
	expected.buildexpectedource({{60,0},{72,12}, {84,0}});
	expected.buildexpectedource({{60,0},{61,1}, {62,0}});
	expected.compare(out);
}

TEST(InputLandData, PaperTest)
{
	// https://www.sciencedirect.com/science/article/pii/S0747717116300104
	LandscapeCollection col;
	col.setTestEnv();

	col.convertLand.addExprRaw(1, 5);
	col.convertLand.addExprRaw(2, 8);
	col.convertLand.addExprRaw(3, 4);
	col.convertLand.addExprRaw(5, 9);
	col.convertLand.addExprRaw(6, 7);

	col.performOnPerform();
	LandscapeClass out;
	col.convertToLandscape(col.convertLand, &out);

	TestOut expected;
	expected.buildexpectedource({{1,0}, {3,   2},   {3.5,1.5}, {5,3}, {6.5,1.5}, {7,2}, {9,0}});
	expected.buildexpectedource({{2,0}, {3.5, 1.5}, {5,0},    {6.5, 1.5}, {8,0}});
	expected.buildexpectedource({{3,0}, {3.5, 0.5}, {4,0}, {6,0}, {6.5, 0.5}, {7,0}});
	expected.compare(out);
}


TEST(LandScape, signature)
{
	// https://www.sciencedirect.com/science/article/pii/S0747717116300104
	LandscapeCollection col;
	col.setTestEnv();

	col.convertLand.addExprRaw(1, 5);
	col.convertLand.addExprRaw(2, 8);
	col.convertLand.addExprRaw(3, 4);
	col.convertLand.addExprRaw(5, 9);
	col.convertLand.addExprRaw(6, 7);
	col.maxEnd = 9;
	col.performOnPerform();
	col.perform();

	std::vector<std::vector<float>> expectedSignature = {
		{0.0f, 0.0f, 1.0f, 2.0f, 2.25f, 3.0f, 2.25f, 2.0f, 1.0f},
		{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // small
		{0.0f, 0.0f, 0.0f, 0.75f, 0.75f, 0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.75f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, // small
	};

	// ASSERT_EQ(col.getItemsCount(), expectedSignature.size());
	for (size_t i = 0; i < col.getItemsCount(); i++)
	{
		LandscapeClass* item = col.getRItem(i);

		std::vector<float> signature;
		item->getSignatureAsVector(signature, 1);
		assert(item->landscape.size() == 1);
		signature.resize(9);

		ASSERT_EQ(signature, expectedSignature[i]);
	}
}

TEST(LandScape, dwq1)
{
	LandscapeCollection col;
	col.setTestEnv();

	col.convertLand.addExprRaw(4, 5);
	// col.convertLand.addExprRaw(144.667, 175.667);
	// col.convertLand.addExprRaw(143.667, 169.333);
	// col.convertLand.addExprRaw(145, 175);
	// col.convertLand.addExprRaw(145.333, 175.667);
	// col.convertLand.addExprRaw(145.667, 180.333);
	// col.convertLand.addExprRaw(144, 179.333);
	col.convertLand.addExprRaw(6, 10);
	col.convertLand.addExprRaw(7, 11);
	col.convertLand.addExprRaw(8, 11);

	LandscapeClass out;
	col.convertToLandscape(col.convertLand, &out);
}


TEST(LandScape, closee)
{
	LandscapeCollection col;
	col.setTestEnv();

	col.convertLand.addExprRaw(144.333, 169);
	// col.convertLand.addExprRaw(144.667, 175.667);
	// col.convertLand.addExprRaw(143.667, 169.333);
	// col.convertLand.addExprRaw(145, 175);
	// col.convertLand.addExprRaw(145.333, 175.667);
	// col.convertLand.addExprRaw(145.667, 180.333);
	// col.convertLand.addExprRaw(144, 179.333);
	col.convertLand.addExprRaw(146, 180);
	col.convertLand.addExprRaw(146.333, 180.333);
	col.convertLand.addExprRaw(146.667, 180.333);

	LandscapeClass out;
	col.convertToLandscape(col.convertLand, &out);
}
