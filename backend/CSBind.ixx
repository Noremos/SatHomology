module;
#include <proj.h>
#include "../Bind/Common.h"
#include "sqlite/sqlite3.h"

export module CSBind;


template<typename T>
PJ_COORD getPjCoord(T x, T y)
{
	PJ_COORD cd{};
	cd.xy.x = x;
	cd.xy.y = y;
	return cd;
}

template<typename T>
PJ_COORD getPjCoord(const T& p)
{
	PJ_COORD cd{};
	cd.xy.x = p.x;
	cd.xy.y = p.y;
	return cd;
}

export class BackProj
{
private:
	PJ* proj = nullptr;
	friend class CSBindnig;
	PJ_CONTEXT* ctx;
public:
	bool isInited() const
	{
		return proj != nullptr;
	}

	bool init(int id)
	{
		// Initialize Proj context
		sqlite3* db = NULL;
		char* strerr = NULL;
		sqlite3_stmt* stmt = prepareSelect(db, "srtext", " WHERE srid = ?");

		auto rc = sqlite3_bind_int(stmt, 1, id); // bind the rowid value to the first placeholder
		if (rc != SQLITE_OK)
		{
			printf("Failed to bind: %s\n", sqlite3_errmsg(db));
			return false;
		}

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_ROW)
		{
			printf("There is not row with id %d\n", id);
			return false;
		}

		const uchar* wtkContext = sqlite3_column_text(stmt, 0);

		//char* vcpkg_dir = std::getenv("VCPKG_INSTALLED_DIR");
		//if (vcpkg_dir != nullptr)
		//{
		//	printf("Vcpkg installation directory:  %s\n", vcpkg_dir);
		//}
		//else {
		//	printf("Vcpkg installation directory not found: %s\n", vcpkg_dir);
		//}

		const char* cpath = Variables::prodDbPath.c_str();

		ctx = proj_context_create();
		proj_context_set_search_paths(ctx, 1, &cpath);
		proj = proj_create_from_wkt(ctx, (const char*)wtkContext, nullptr, nullptr, nullptr);

		sqlite3_finalize(stmt);
		return true;
	}

	const char* getName() const
	{
		return proj_get_name(proj);
	}

	BackPoint getThisProj(const BackProj& item, BackPoint itemPos) const
	{
		PJ* ctc_proj = proj_create_crs_to_crs_from_pj(item.ctx, item.proj, proj, nullptr, nullptr);
		PJ_COORD real = proj_trans(ctc_proj, PJ_IDENT, getPjCoord(itemPos));
		return BackPoint{ real.xy.x, real.xy.y };
	}

	~BackProj()
	{
		if (proj)
		{
			proj = proj_destroy(proj);
			proj = nullptr;
		}

		if (ctx)
		{
			proj_context_destroy(ctx);
			ctx = nullptr;
		}
	}

	static sqlite3_stmt* prepareSelect(sqlite3* db, const char* name, const char* where = NULL)
	{
		auto rc = sqlite3_open_v2("meta/proj.sqlite", &db, SQLITE_OPEN_READONLY, NULL);
		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "������ ��������/�������� ��: %s\n", sqlite3_errmsg(db));
			return NULL;
		}

		BackString sql = "SELECT ";
		sql += name;
		sql += " FROM spatial_ref_sys ";
		if (where)
			sql += where;

		sqlite3_stmt* stmt;
		rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
			return NULL;
		}

		return stmt;
	}

	static std::vector<BackString> getWtkNames()
	{
		sqlite3* db = NULL;
		sqlite3_stmt* stmt = prepareSelect(db, "srid");

		std::vector<BackString> names;
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			const uchar* wtkContext = sqlite3_column_text(stmt, 0);
			names.push_back(BackString((const char*)wtkContext));
		}

		return names;
	}
};


//
//export class CoordSystem
//{
//	float stX, stY;
//	float maxX, maxY;
//	float uPerPixX;
//	float uPerPixY;
//
//	bc::point toLocal(float x, float y)
//	{
//		int x_loc = (x - stX) / uPerPixX;
//		int y_loc = (y - stY) / uPerPixY;
//		return { x_loc, y_loc };
//	}
//
//	BackPoint toGlobal(int x, int y)
//	{
//		float x_glob = x * uPerPixX + stX;
//		float y_glob = y * uPerPixY + stY;
//		// y_glob = y_maximum - y * uPerPixY
//		// return QgsPointXY(rectangle.xMinimum() + p[0] * pixelSizeX, rectangle.yMaximum() - p[1] * pixelSizeY)
//		return { x_glob, y_glob };
//	}
//};


export struct CSBindnig
{
	// Projection of the local image for coord system
	BackProj proj;
	BackPoint globOrigin = {0,0};
	double img_transform[6] = {0.0,0.0,0.0,0.0,0.0,0.0};

	void setScale(double x, double y)
	{
		img_transform[1] = x;
		img_transform[5] = y;
	}

	void setOrigin(double x, double y)
	{
		globOrigin.x = x;
		globOrigin.y = y;
	}

	double* getScaleX()
	{
		return &img_transform[1];
	}
	double* getScaleY()
	{
		return &img_transform[5];
	}


	BackPixelPoint toLocal(const BackPoint& bp) const
	{
		//int x_loc = (x - stX) / uPerPixX;
		//int y_loc = (y - stY) / uPerPixY;
		//return { x_loc, y_loc };

		PJ_COORD cd = getPjCoord(bp);

		float pixel_x, pixel_y;
		PJ_COORD local = proj_trans(proj.proj, PJ_FWD, cd);
		pixel_x = ((local.xy.x - globOrigin.x) / img_transform[1]);
		pixel_y = ((local.xy.y - globOrigin.y) / img_transform[5]);

		return BackPixelPoint(pixel_x, pixel_y);
	}

	BackPoint toGlobal(int x, int y) const
	{
		//float x_glob = x * uPerPixX + stX;
		//float y_glob = y * uPerPixY + stY;
		// y_glob = y_maximum - y * uPerPixY
		// return QgsPointXY(rectangle.xMinimum() + p[0] * pixelSizeX, rectangle.yMaximum() - p[1] * pixelSizeY)

		 // Convert the pixel coordinates to the real-world coordinates
		x = globOrigin.x + (x * img_transform[1]) + (y * img_transform[2]);
		y = globOrigin.y + (x * img_transform[4]) + (y * img_transform[5]);

		PJ_COORD real = proj_trans(proj.proj, PJ_INV, getPjCoord(x, y));
		return {real.xy.x, real.xy.y};
	}
};

export using CoordSystem = CSBindnig;
