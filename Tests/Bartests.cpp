#include "gtest/gtest.h"
#include <initializer_list>
#include <ostream>
#include <vector>
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "../../MatrImg.h"

constexpr bool DEBUGY = false;
TEST(BarTest, FindCloser)
{
	bc::barstruct constr;
	constr.createBinaryMasks = true;
	constr.createGraph = false;
	constr.attachMode = bc::AttachMode::morePointsEatLow;
	//constr.attachMode = bc::AttachMode::closer;
	constr.returnType = bc::ReturnType::barcode2d;

	constr.addStructure(bc::ProcType::f0t255, bc::ColorType::native, bc::ComponentType::Component);

	BackImage img = imread("/Users/sam/Edu/datasets/test_dataset/crater/image_27.png");
	bc::BarcodeCreator creator;
	std::unique_ptr<bc::Baritem> citem(creator.createBarcode(&img, constr));
}




// TEST(TrainerTest, FindCloserSimple)
// {
// 	Trainer<8> train;
// 	0 (new): 0.520325 1 1 1 1 1 0 0 -> 0
// 	1 (new): 0.520325 0.520325 0.520325 0 0 0.520325 0.520325 0.520325 -> 0
// 	2 (new): 0 0.520325 0 1 1 1 0.520325 0.520325 -> 0
// 	3 (new): 1 1 1 1 0.520325 0 0.520325 1 -> 0.520325
// 	4 (new): 0.520325 1 1 1 0 0 0 0.520325 -> 0.520325
// 	5 (new): 1 1 0.520325 0.520325 0 0.520325 0.520325 1 -> 0.520325
// 	6 (new): 1 1 0.520325 0 0.520325 1 1 1 -> 0.520325
// 	7 (new): 0.520325 0.520325 0 0 0.520325 1 1 1 -> 0.520325
// 	8 (new): 0.520325 0 0 1 1 1 1 1 -> 0.520325
// 	9 (new): 0 0 0 0 1 0.520325 1 0 -> 1
// 	10 (new): 1 0 0 0 1 0.520325 0.520325 1 -> 1
// 	11 (new): 1 0 0 0 1 0 0.520325 0.520325 -> 1
// 	12 (new): 1 0 0 0 1 1 0 0.520325 -> 1
// 	13 (new): 1 0 0 0 0 0 1 0 -> 1
// 	14 (new): 0 0 1 1 0.520325 0.520325 1 0 -> 1
// 	15 (new): 0 1 1 0 0 0 1 0 -> 1
// 	16 (new): 0 0 1 0.520325 0.520325 0.520325 1 0 -> 1
// 	17 (new): 0 0 1 0 0 0 1 0.520325 -> 1
// 	18 (new): 0 0 1 0.520325 0.520325 1 1 0 -> 1
// 	19 (new): 0.520325 0 1 0 0 0 1 1 -> 1
// 	20 (new): 0 0 1 0.520325 1 0 0 0 -> 1
// 	21 (new): 1 1 0.520325 0.520325 1 0 0 0 -> 1
// 	22 (new): 1 0.520325 0.520325 0.520325 1 0 0 0 -> 1
// 	23 (new): 1 0.520325 0.520325 1 1 0 0 0 -> 1
// 	24 (new): 1 0.520325 1 0 0 0 0 0 -> 1
// 	Generating
// 	0.0480942 0.761018 0.139636 0.768994 0.705262 0.871071 0.405527 0.0999346 id> 2(0->0.697034)
// 	0.690132 0.139636 0.768994 0.571553 0.719276 0.917196 0.871071 0.405527 id> 18(1->0.70821)
// 	0.705262 0.768994 0.571553 0.770641 0.0144336 0.087315 0.917196 0.871071 id> 19(1->0.722083)
// 	0.0999346 0.0480942 0.690132 0.705262 0.871071 0.271182 0.11007 0.25657 id> 20(1->0.411472)
// 	0.405527 0.690132 0.705262 0.719276 0.917196 0.785275 0.271182 0.11007 id> 22(1->0.87236)
// 	0.871071 0.705262 0.719276 0.0144336 0.087315 0.710842 0.785275 0.271182 id> 1(0->0.926368)
// 	0.25657 0.0999346 0.405527 0.871071 0.271182 0.422712 0.577949 0.857636 id> 14(1->0.118969)
// 	0.11007 0.405527 0.871071 0.917196 0.785275 0.784359 0.422712 0.577949 id> 2(0->0.273894)
// 	0.271182 0.871071 0.917196 0.087315 0.710842 0.148793 0.784359 0.422712 id> 17(1->0.787422)
// }