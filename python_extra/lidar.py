import laspy

from BinState import BinStateWriter
import numpy as np

# Specify the path to your .las file

def save_las_to_bin(las_file_path, output):
	lasdata = BinStateWriter(output)
	# Open the .las file
	with laspy.open(las_file_path) as las:
		print(f"File contains {las.header.point_count} points.")
		print(f"Bounds: {las.header.mins} to {las.header.maxs}")

		# Access points if needed
		points = las.read().points
		# print(points)
		if str(points.array.dtype.fields.get('Z')[0]) == 'int32':
			print("Z is int32")
			for p in points.array:
				lasdata.p_bool(True)
				lasdata.p_int(p[0]) #X
				lasdata.p_int(p[1]) #Y
				lasdata.p_int(p[2]) #Z
		else:
			print("Z is not int32")
			for p in points.array:
				lasdata.p_bool(True)
				lasdata.p_float(p[0])
				lasdata.p_float(p[1])
				lasdata.p_float(p[2])

	lasdata.p_bool(False)


# create a geotiff from las file
def create_geotiff_from_las(las_file_path, output_tif_path):
	from osgeo import gdal, osr

	with laspy.open(las_file_path) as las:
		points = las.read().points
		x = points.array['X']
		y = points.array['Y']
		z = points.array['Z']

		# Normalize coordinates
		x_min, y_min = x.min(), y.min()
		x_max, y_max = x.max(), y.max()
		x -= x_min
		y -= y_min

		# Create a grid for the raster
		grid_x = x_max - x_min + 1
		grid_y = y_max - y_min + 1
		raster = np.full((grid_y, grid_x), np.nan)

		# Populate the raster with Z values
		for xi, yi, zi in zip(x, y, z):
			raster[int(yi), int(xi)] = zi

		# Replace NaN with a default value (e.g., 0)
		raster = np.nan_to_num(raster, nan=0)

		# Create a GeoTIFF file
		driver = gdal.GetDriverByName("GTiff")
		dataset = driver.Create(output_tif_path, grid_x, grid_y, 1, gdal.GDT_Float32)

		# Set geotransform and projection
		geotransform = (x_min, 1, 0, y_min, 0, 1)
		dataset.SetGeoTransform(geotransform)
		srs = osr.SpatialReference()
		srs.ImportFromEPSG(4326)  # WGS84
		dataset.SetProjection(srs.ExportToWkt())

		# Write the raster data
		dataset.GetRasterBand(1).WriteArray(raster)
		dataset.FlushCache()
		dataset = None



# save_las_to_bin('/Users/sam/Edu/Аспирантура/Arch/LiDAR_BIGDATA_GN/Безклассификации/2020_1 без классификации_ч1.las', "las1.raw")
# save_las_to_bin('/Users/sam/Edu/Аспирантура/Arch/LiDAR_BIGDATA_GN/Безклассификации/2020_1 без классификации_ч2.las', "las2.raw")




lasFiles = [
	'/Users/sam/Edu/Аспирантура/Arch/LiDAR_BIGDATA_GN/Безклассификации/2020_1 без классификации_ч1.las',
	'/Users/sam/Edu/Аспирантура/Arch/LiDAR_BIGDATA_GN/Безклассификации/2020_1 без классификации_ч2.las',
	'/Users/sam/Edu/Аспирантура/Arch/LiDAR_BIGDATA_GN/Ground_2020_Участок 1.las'
]


void processAll()
{
	std::array<std::string_view, 3> files = {
		"/Users/sam/H/Programs/imgui/SatHomology/lidara.raw",
		"/Users/sam/H/Programs/imgui/SatHomology/las1.raw",
		"/Users/sam/H/Programs/imgui/SatHomology/las2.raw"
	};

	std::array<bc::ProcType, 3> procs = {
		bc::ProcType::f0t255,
		bc::ProcType::f255t0,
		bc::ProcType::Radius
	};

	std::array<std::string_view, 9> labels = {
		"Ground_2020 f0t255",
		"Ground_2020 f255t0",
		"Ground_2020 Radius",
		"2020_1 ch1 f0t255",
		"2020_1 ch1 f255t0",
		"2020_1 ch1 Radius",
		"2020_1 ch2 f0t255",
		"2020_1 ch2 f255t0",
		"2020_1 ch2 Radius"
	};

	static_assert(files.size() == procs.size());
	static_assert(files.size() * procs.size() == labels.size());

	int labelCounter = 0;
	for (size_t i = 0; i < files.size(); ++i)
	{
		std::cout << "Processing file: " << files[i] << std::endl;

		std::string_view lasFilePath = files[i];
		LasOut outlas = LasParser::parseLasFile(lasFilePath);

		bc::barstruct bcs;
		bcs.coltype = bc::ColorType::native;
		bcs.createBinaryMasks = true;
		bcs.createGraph = true;

		for (size_t j = 0; j < procs.size(); ++j)
		{
			std::cout << "Step: " << j << std::endl;
			bcs.proctype = procs[j];

			const std::string_view label = labels[labelCounter++];
			ShapeFile shp(label);

			int a = 0;

			processLas(outlas, bcs, 1, false, [&](bc::Baritem& item, const TileProvider iolProv)
			{
				for (size_t i = 0; i < item.barlines.size(); ++i)
				{
					const bc::barline& curLine = *item.barlines[i];
					shp.writePolygonRecord(curLine, iolProv);
				}
				return false;
			});
			shp.close();
			std::cout << "Write to " << label << std::endl;
			return;
		}
	}
}
