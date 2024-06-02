module;
#include <mutex>
#include <cassert>
#include <unordered_map>
#include "Usings.h"
#include "Barcode/PrjBarlib/include/barstrucs.h"

#include "../CSBind.h"

#include "../MatrImg.h"
export module ClassifierCore;
import IItemModule; //
import Classifiers;
import LayersCore;
import TrainIO;
import GeoprocessorModule;
import VectorLayers;
import RasterLineLayerModule;
import ProjectModule; // BackImage
// import CSBind; // csbind
//import BackBind;
// import MatrModule;

// BarClassses
import ClassifierInterface;


bc::BarRect getBarRect(const bc::barvector& matrix)
{
	int l, r, t, d;
	r = l = matrix[0].getX();
	t = d = matrix[0].getY();
	for (size_t j = 0; j < matrix.size(); ++j)
	{
		if (l > matrix[j].getX())
			l = matrix[j].getX();
		if (r < matrix[j].getX())
			r = matrix[j].getX();

		if (t > matrix[j].getY())
			t = matrix[j].getY();
		if (d < matrix[j].getY())
			d = matrix[j].getY();
	}
	return bc::BarRect(l, t, r - l + 1, d - t + 1);
}

export class ClassifierBackend
{
public:
	std::unordered_map<int, VectorLayer*> classLayers;

	int predict(const IClassItem* item)
	{
		int classType = classifier->predict(item);
		return classType;
	}

	void saveLoadState()
	{
		// is reading
		//classCategs = BarCategories::loadCategories(getPath(BackPath::classifier));
		//settings.metaprov.reset(new MetadataProvider(path, metaCounter));
		//auto path = getMetaPath();
		//classifier.open(path);

		//classifier->open(getMetaPath());
		//classifier->loadData(classCategs);

		//// else
		//classCategs->saveCategories(getPath(BackPath::classifier));
	}

	BarCategories classCategs;
	IBarClassifier* classifier;
	std::mutex addPrimitiveMutex;
	int selectedClassId;
	bool predictForLayer(IClassItem* item, const TileProvider& tileProv, float diplsayToRealFactor)
	{
		auto classId = predict(item);
		if (classId != -1)
		{
			VectorLayer* vl = classLayers.at(classId);
			assert(vl != nullptr);
			CSBinding& cs = vl->cs;

			DrawPrimitive* p;
			{
				std::lock_guard<std::mutex> guard(addPrimitiveMutex);
				p = vl->addPrimitive(vl->color);
			}

			//BackPoint center;
			//float r;
			//getCircle(item->getMatrix(), center, r);
			//BackPoint rb(center.x + r, center.y + r);
			//BackPoint iglob = toGlob(cs, tileProv, diplsayToRealFactor, center);
			//BackPoint globRb = toGlob(cs, tileProv, diplsayToRealFactor, rb);
			//p->setCircle(iglob, globRb);


			mcountor temp;
			getCountour(item->getMatrix(), temp, true);
			for (const auto& pm : temp)
			{
				auto point = bc::barvalue::getStatPoint(pm);

				BackPixelPoint op = tileProv.tileToPreview(point.x, point.y); // To display
				BackPoint iglob((static_cast<float>(op.x) + 0.5f), static_cast<float>(op.y) + 0.5f);

				iglob = cs.toGlobal(iglob.x * diplsayToRealFactor, iglob.y * diplsayToRealFactor); // To real
				p->addPoint(iglob);
			}

			return true;
		}
		else
			return false;
	}


	//void getOffsertByTileIndex(buint tileIndex, buint& offX, buint& offY)
	//{
	//	int tilesInRow = getCon(reader->width(), tileSize);
	//	offY = (tileIndex / tilesInRow) * tileSize;
	//	offX = (tileIndex % tilesInRow) * tileSize;
	//}

	//buint getTileIndexByOffset(buint offX, buint offY)
	//{
	//	int tilesInRow = getCon(reader->width(), tileSize);
	//	return (offY / tileSize) * tilesInRow + offX / tileSize;
	//}


	// BarcodeHolder threasholdLines(bc::Baritem* item)
	// {
	// 	BarcodeHolder vec;
	// 	for (size_t i = 0; i < item->barlines.size(); ++i)
	// 	{
	// 		if (item->barlines[i]->len() < 10)
	// 		{
	// 			vec.lines.push_back(item->barlines[i]->clone());
	// 		}
	// 	}
	// 	return vec;
	// }

	int addClassType(const BackString& name)
	{
		int classId = classCategs.addValue(name);
		classifier->addClass(classId);
		auto* layer = addClassLayer(classId);
		layer->name = "Class: " + name;

		return classId;
	}

	ClassVectorLayer* addClassLayer(int classId, bool addToList = true)
	{
		ClassVectorLayer* layer = Project::proj->addLayerData<ClassVectorLayer>(Project::proj->getDisplay().sysProj.getId());
		layer->color = BackColor::random();
		//layer->vecType = VectorLayer::VecType::circles;
		layer->vecType = VectorLayer::VecType::polygons;
		if (addToList)
		{
			layer->isSystem = true;
			classLayers[classId] = layer;
		}

		return layer;
	}

	void changeClassName(int classId, const BackString& name)
	{
		classCategs.changeName(classId, name);
	}

	void removeClassType(int classId)
	{
		classCategs.remove(classId);
		classifier->removeClass(classId);

		auto claVec = classLayers.find(classId);
		claVec->second->isSystem = false;
		classLayers.erase(classId);
	}

	void removeLayer(int coreId)
	{
		/*ILayer* lay = layers.at(coreId);
		if (!lay->isSystem)
		{
			layers.remove(coreId);
		}*/
	}

	size_t addTrainData(int layerId, int classId, CachedObjectId srcItemId, BackImage* destIcon)
	{
		auto inLayer = Project::proj->getInTRaster<RasterLineLayer>(layerId);
		assert(inLayer);
		IRasterLayer* sourceLayer = Project::getProject()->getInRaster(inLayer->parentlayerId);
		sourceLayer->setSubImage(0);

		ItemHolderCache cached;
		cached.openRead(inLayer->getCacheFilePath(Project::getProject()->getMeta()));

		std::unique_ptr<IClassItemHolder> item = classFactory.CreateHolder(selectedClassId); // Caching item is raw, nit processed
		cached.loadSpecific(srcItemId.tileId, item.get());

		BackImage* fromSourceImg = nullptr;
		assert(srcItemId.vecId < item->getItemsCount());
		IClassItem* line = item->getItem(srcItemId.vecId);
		if (destIcon != nullptr && sourceLayer)
		{
			auto rect = getBarRect(line->getMatrix());
			auto tileProv = inLayer->prov.tileByIndex(srcItemId.tileId);
			BackPixelPoint st = tileProv.tileToFull(rect.x, rect.y);
			BackPixelPoint ed = tileProv.tileToFull(rect.right(), rect.botton());

			float realFactor = inLayer->subToRealFactor / inLayer->prov.displayFactor; // sub -> display -> real
			printf("Get rect (%d, %d) (%d, %d) with factor %f\n", st.x, st.y, ed.x + st.x, ed.y + st.y, realFactor);
			ed = (ed - st) * realFactor;
			st = st * realFactor;
			printf("Get rect (%d, %d) (%d, %d)\n", st.x, st.y, ed.x + st.x, ed.y + st.y);
			*destIcon = sourceLayer->getRect(st.x, st.y, ed.x, ed.y);
			fromSourceImg = destIcon;
		}
		//rb->barlines[srcItemId.vecId] = nullptr;

		assert(classCategs.size() != 0);
		return classifier->addData(classId, line, fromSourceImg);
	}

	void removeTrainData(int classId, int localId)
	{
		classifier->removeData(classId, localId);
	}
};
