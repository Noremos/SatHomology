#include "SKLearnInterface.h"


class SklearnClassifier : public ISklearnClassifier
{
public:
	SklearnClassifier()
	{
	}

	virtual void writeToTemp(const IClusterItemHolder& iallItems, BackFileWriter &tempFile)
	{
		const TreeSignatureCollection& allItems = dynamic_cast<const TreeSignatureCollection&>(iallItems);
		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& sign = allItems.getRItem(i).signature;

			for (const auto& num : sign)
			{
				tempFile << num << " ";
			}
			tempFile.seekp(-1, tempFile.cur); // ������� ��������� �������
			tempFile << std::endl;

		}
	}
};

GlobalClusterRegister<TreeClass, TreeSignatureCollection, SklearnClassifier> SKLearnReg("SKLearn");
