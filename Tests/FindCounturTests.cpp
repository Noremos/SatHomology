#include "gtest/gtest.h"

#include "../backend/GeoProcessor.h"


TEST(CounturTest, FindSimple)
{
	return;

	bc::barvector inPoints;

	inPoints.push_back({0,0, 0.f});
	inPoints.push_back({0,1, 0.f});
	inPoints.push_back({0,2, 0.f});
	mcountor out;
	getCountourOder(inPoints, out, true);

	ASSERT_EQ(out.size(), 4);

	ASSERT_EQ(out[0].x, 0);
	ASSERT_EQ(out[0].y, 0);

	ASSERT_EQ(out[1].x, 1);
	ASSERT_EQ(out[1].y, 0);

	ASSERT_EQ(out[2].x, 1);
	ASSERT_EQ(out[2].y, 3);

	ASSERT_EQ(out[2].x, 0);
	ASSERT_EQ(out[2].y, 3);
}