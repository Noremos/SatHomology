int maxAllowed = 10;
BackPathStr srcleanb("/Users/sam/Edu/datasets/objects/eurosat_augment");
for (const auto& entry : std::filesystem::directory_iterator(srcleanb))
{
	auto path = entry.path();
	auto fileName = path.filename().string();

	// int number_.pos = fileName.find_first_of('_');
	int subtype_pos = fileName.find_first_of('-');
	int ext_type = fileName.find_last_of('.');

	string ext = fileName.substr(ext_type + 1);
	if (ext != "png" && ext != "jpg")
	{
		continue;
	}

	int lastPos = subtype_pos == std::string::npos ? ext_type : subtype_pos;
	BackString onlyName = fileName.substr(0, lastPos);
	auto& img = sourceFiles[onlyName];
	// std::cout << onlyName << endl;
	if (img == nullptr)
	{
		img.reset(new ImageC);
	}

	if (subtype_pos == std::string::npos)
	{
		img->name = fileName;
	}
	else
	{
		BackString sub_type = fileName.substr(subtype_pos + 1, ext_type - subtype_pos - 1);
		if (augmentNumber.count(sub_type) == 0)
		{
			augmentNumber.insert(std::pair(sub_type, augmentNumber.size()));
		}

		int sub_type_pos = augmentNumber[sub_type];
		img->augments[sub_type_pos] = fileName;
	}
}

std::vector<Results> numToNameMap(augmentNumber.size());
for (auto augName : augmentNumber)
{
	numToNameMap[augName.second] = augName.first;
}

int augSize = numToNameMap.size();

for (auto& filePair : sourceFiles)
{
	ImageC& img = *(filePair.second.get());
	BackImage src = imread(srcleanb / img.name);
	auto baseImg = bc::BarcodeCreator::create(src, constr);
	baseImg->normalize();

	for (int i = 0; i < augSize; i++)
	{
		assert(!img.augments[i].empty());

		BackImage aug = imread(srcleanb / img.augments[i]);
		auto augImg = bc::BarcodeCreator::create(aug, constr);
		augImg->normalize();

		float res = baseImg->compireFull(augImg.get(), bc::CompireStrategy::CommonToLen);
		numToNameMap[i].add(res);
	}
}

for (auto& res : numToNameMap)
{
	res.printResult();
}
}