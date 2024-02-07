#include "pch.h"
import ConvertItem;

TEST(TestCaseName, TestName) {
 

  ConvertCollection col;
  col.nextLine.addExpr(3, 9);
  col.nextLine.addExpr(4, 6);
  col.nextLine.addExpr(5, 11);

  col.perform();

  std::vector < std::pair<float, float>> expected = {
	  {3,11},
	  {4,9},
	  {5,6}
  };
  for (size_t i = 0; i < col.getItemsCount(); i++)
  {
	  EXPECT_EQ(col.getRItem(i)->start().getAvgFloat(), expected[i].first);
	  EXPECT_EQ(col.getRItem(i)->end().getAvgFloat(), expected[i].second);
  }
  
}