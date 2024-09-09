#include "LandscapeItem.h"


using InputLandLineSet = std::vector<InputLandLine>;
InputLandLineSet sortInputLandscape(InputLandData& landscape)
{
	landscape.sort();

	std::vector<InputLandLine> landset;
	size_t prev = 0; // Skip same lines
	for (size_t i = 1; i < landscape.size(); i++)
	{
		auto& line1 = landscape.get(prev);
		auto& line2 = landscape.get(i);
		if (line1.start == line2.start && line1.len == line2.len)
		{
			line1.matrix.insert(line1.matrix.end(), line2.matrix.begin(), line2.matrix.end());
			continue;
		}
		landset.push_back(line1);
		prev = i;
	}

	landset.push_back(landscape.get(prev));

	return landset;
}

void LandscapeCollection::convertToLandscape(InputLandData& landscape, LandscapeClass* source)
{
	int k = 0;

	std::vector<bool> used;
	used.resize(landscape.size(), false);

	InputLandLineSet landset = sortInputLandscape(landscape);

	const size_t landSize = landset.size();
	for (size_t i = 0; i < landSize; i++)
	{
		if (used[i])
			continue;

		used[i] = true;

		auto* line = &landset[i];

		float start = line->getStart();
		float end = line->getEnd();
		float beginning = start;
		float crossWithPrev = start;
		// if (i == 18)
		// 	std::cout << start << " " << end << std::endl;
		// std::cout << start << " " << end << std::endl;
		// float matrstart = start;
		// float ending = end;
		// float lastMin = beginning;
		// float lastStart = start;

		//std::vector<LandPoint> landPath = { {start, 0}, {middle, h} };

		if (source == nullptr)
		{
			Base::items.push_back({});
			// Base::items.back().maxEnd = maxEnd;
		}

		LandscapeClass& cl = source ? *source : Base::items.back();
		cl.id = i;
		cl.landscape.push_back({});
		cl.add(start, 0, round);
		cl.depth = k;
		cl.landscape.back().k = k;
		k++;
		// cl.AddE = resolution;

		int curI = i;
		int prevId = -1;
		int backScanStart = i;
		bool curLineCatechd = false;
		while (true)
		{
			// Asc, началось до текущего, окончилось внутри
			bool reachedPeak = true;
			for (int ascI = backScanStart - 1; ascI >= 0; --ascI)
			{
				// c - crossWithPrev
				//  CUR->                C-------------E
				//  PRV->   S-------------E

				//  CUR->                C-------M------E
				//  PRV->   S-----------M-------------E
				// if (ascI == prevId)
				// 	continue;

				auto& prevLine = landset[ascI];

				assert(prevLine.len);

				if (prevLine.getStart() <= beginning && between(prevLine.getEnd(), crossWithPrev, end)) // Между концом предыдущего и концном текущего
				{
					float cross = prevLine.getEnd() - start;
					float midPoint = cross / 2.f;
					cl.add(start + midPoint, midPoint, round);
					end = prevLine.getEnd();
					reachedPeak = false;

					// if (i == 18)
					// 	std::cout << "prev:" << prevLine.getStart() << " " << end << std::endl;
					break;
				}
			}

			float len = end - start;
			float middle = start + len / 2;

			// Select common values (all when there is no prev)
			// from line.satrt to prevLine.end
			// from end to line->end
			auto& matr = line->matrix; //srcLine.getChild(startI)->getMatrix();
			// for (auto& val : matr)
			// {
			// 	if (val.value < end)
			// 		cl.matrix.push_back(val);
			// }
			curLineCatechd = true;


			if (reachedPeak)
			{
				// if (i == 18)
				// 	std::cout << "peak:" << start << " " << end << std::endl;

				cl.add(middle, len / 2, round);
			}

			// dsc, те, которые начались внутри и окончились снаружи текущего
			prevId = curI;
			size_t nextI = static_cast<size_t>(curI) + 1;

			for (;nextI < landSize; nextI++)
			{
				// Checks for
				//  CUR->      |-------M------|
				//  NEXT->      |-------M-------|
				auto& nextLand = landset[nextI];
				if (nextLand.getStart() > end)
				{
					break;
				}

				// *** Same start ***
				if (nextLand.getStart() == start)
				{
					// Когда начинается с одинакового значения
					continue;
				}

				// *** Same end ***
				// The next one ends at the same pos as the current one ends
				if (nextLand.getEnd() == end)
				{
					continue;
				}


				// Checks for
				//  CUR->      |-------M------|
				//  NEXT->                    |-------M-------|
				if (nextLand.getEnd() < end)
					continue;



				// The next one starts at the same pos as the current one ends
				if (nextLand.getStart() == end)
				{
					beginning = nextLand.start;
					backScanStart = nextI;
					used[nextI] = true;
				}


				//float crossGip = end - convertLand->getStart();
				//float leh = crossGip / 2;

				// from line matrstart(cur line start) to convertLand.start
				// From this line start to the next line start

				float cross = end - nextLand.getStart(); // длина пересечения
				crossWithPrev = end; // Смотрим предыдущий, который до окончания текущего
				cl.add(end - cross / 2.f, cross / 2.f, round);
				auto& matr = line->matrix;
				for (auto& val : matr)
				{
					if (val.value < end - cross / 2.f)
						cl.matrix.push_back(val);
				}


				// lastMin = end;
				start = nextLand.getStart();
				end = nextLand.getEnd();
				curLineCatechd = false;
				curI = nextI;
				line = &landset[nextI];


				// if (i == 18)
				// 	std::cout << "next:" << start << " " << end << std::endl;
				// matrstart = line->getStart();

				break;

				//landPath.push_back({ convertLand->getStart() +  leh, leh});
				//auto lastH = landPath.back();
				//cl.paths.push_back({ lastH.x + lastH.y, 0 });
				//cl.paths.push_back({ lastH.x + lastH.y, 0 });
			}

			// Не нашли линии внутри
			if (prevId == curI)
			{
				// if (i == 18)
				// 		std::cout << start << " " << end << std::endl;
				// assert(end <= maxEnd);

				// Добавляем конце линии
				cl.add(end, 0, round);

				auto& matr = line->matrix;
				for (auto& val : matr)
				{
					if (val.value < end)
						cl.matrix.push_back(val);

				}


				// Ищем следующую на том же уровне K
				for (;curI < landSize; curI++)
				{
					if (used[curI])
						continue;

					//  CUR->      |-------M------|
					//  NEXT->                   |-------M-------|

					//  CUR->      |-------M------|
					//  NEXT->      |-------M-------|
					auto& nextLand = landset[curI];
					// Следующая должна начинаться после текущей
					if (nextLand.getStart() >= end)
					{
						// Found the next line
						used[curI] = true;
						if (nextLand.getStart() > end)
							cl.add(nextLand.getStart(), 0, round);

						start = nextLand.getStart();
						end = nextLand.getEnd();
						beginning = start;
						crossWithPrev = start;

						backScanStart = curI;
						curLineCatechd = false;
						line = &nextLand;
						break;
					}
				}
				if (curI == landSize)
					break;
			}
		} // Inf while

	}
}
