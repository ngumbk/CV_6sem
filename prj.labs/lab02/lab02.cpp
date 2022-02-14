#include <opencv2/opencv.hpp>

void getBrightness(cv::Mat grayscale_image, int *pixel_array) {
	for (int i = 0; i < 256; i++) {
		pixel_array[i] = 0;
	}
	for (int i = 0; i < grayscale_image.rows; i++) {
		for (int j = 0; j < grayscale_image.cols; j++) {
			pixel_array[grayscale_image.at<uchar>(i, j)]++;
		}
	}
}

int main() {
	cv::Mat image_png = cv::imread("../../../data/cross_0256x0256.png"); //C:/Users/ngumbk/Documents/Ã»—»—/6_sem/opencv/polevoy_d_v

	//jpeg with 25% quality
	std::vector<int> params_jpeg;
	params_jpeg.push_back(cv::IMWRITE_JPEG_QUALITY);
	params_jpeg.push_back(25);
	cv::imwrite("cross_0256x0256_025.jpg", image_png, {cv::IMWRITE_JPEG_QUALITY, 25}); //params_jpeg
	
	//png 3-channel mosaic
	
	//splitting 3-channel image into 3 1C
	cv::Mat single_channel_png[3]; //BGR
	cv::split(image_png, single_channel_png);

	//adding zerochannels to merge a 3-channel monochrome image
	cv::Mat zeroChannel(256, 256, CV_8UC1);
	zeroChannel = 0;

	cv::Mat monochrome_png[3];
	cv::Mat png_array_blue[3] = { single_channel_png[0], zeroChannel, zeroChannel };
	cv::Mat png_array_green[3] = { zeroChannel, single_channel_png[1], zeroChannel };
	cv::Mat png_array_red[3] = { zeroChannel, zeroChannel, single_channel_png[2] };
	cv::merge(png_array_blue, 3, monochrome_png[0]);
	cv::merge(png_array_green, 3, monochrome_png[1]);
	cv::merge(png_array_red, 3, monochrome_png[2]);

	//concatenating the mosaic
	cv::Mat mosaic_png_top;
	cv::Mat mosaic_png_bot;
	cv::Mat mosaic_png;
	
	cv::hconcat(image_png, monochrome_png[2], mosaic_png_top);
	cv::hconcat(monochrome_png[1], monochrome_png[0], mosaic_png_bot);
	cv::vconcat(mosaic_png_top, mosaic_png_bot, mosaic_png);
	cv::imwrite("cross_0256x0256_png_channels.png", mosaic_png);

	//jpeg 3-channel mosaic
	params_jpeg.pop_back();
	params_jpeg.push_back(100);
	cv::imwrite("cross_0256x0256.jpg", image_png, params_jpeg);
	cv::Mat image_jpeg = cv::imread("cross_0256x0256.jpg");
	
	//splitting 3-channel image into 3 1C
	cv::Mat single_channel_jpeg[3]; //BGR
	cv::split(image_jpeg, single_channel_jpeg);

	//adding zerochannels to merge a 3-channel monochrome image
	cv::Mat monochrome_jpeg[3];
	cv::Mat jpeg_array_blue[3] = { single_channel_jpeg[0], zeroChannel, zeroChannel };
	cv::Mat jpeg_array_green[3] = { zeroChannel, single_channel_jpeg[1], zeroChannel };
	cv::Mat jpeg_array_red[3] = { zeroChannel, zeroChannel, single_channel_jpeg[2] };
	cv::merge(jpeg_array_blue, 3, monochrome_jpeg[0]);
	cv::merge(jpeg_array_green, 3, monochrome_jpeg[1]);
	cv::merge(jpeg_array_red, 3, monochrome_jpeg[2]);

	//concatenating the mosaic
	cv::Mat mosaic_jpeg_top;
	cv::Mat mosaic_jpeg_bot;
	cv::Mat mosaic_jpeg;

	cv::hconcat(image_jpeg, monochrome_jpeg[2], mosaic_jpeg_top);
	cv::hconcat(monochrome_jpeg[1], monochrome_jpeg[0], mosaic_jpeg_bot);
	cv::vconcat(mosaic_jpeg_top, mosaic_jpeg_bot, mosaic_jpeg);
	cv::imwrite("cross_0256x0256_jpg_channels.png", mosaic_jpeg);

	//png histogram

	cv::Mat histogram_png(256, 256, CV_8UC3, cv::Scalar(240, 240, 240));
	int blue_pixels_count[256];
	int green_pixels_count[256];
	int red_pixels_count[256];
	getBrightness(single_channel_png[0], blue_pixels_count);
	getBrightness(single_channel_png[1], green_pixels_count);
	getBrightness(single_channel_png[2], red_pixels_count);

	//getting max height
	int max = 0;
	for (int i = 0; i < 256; i++) {
		if (max < blue_pixels_count[i]) {
			max = blue_pixels_count[i];
		}
	}
	for (int i = 0; i < 256; i++) {
		if (max < green_pixels_count[i]) {
			max = green_pixels_count[i];
		}
	}
	for (int i = 0; i < 256; i++) {
		if (max < red_pixels_count[i]) {
			max = red_pixels_count[i];
		}
	}
	
	//downscaling considering max height
	for (int i = 0; i < 256; i++) {
		blue_pixels_count[i] = ((double)blue_pixels_count[i] / max) * histogram_png.rows;
	}
	for (int i = 0; i < 256; i++) {
		green_pixels_count[i] = ((double)green_pixels_count[i] / max) * histogram_png.rows;
	}
	for (int i = 0; i < 256; i++) {
		red_pixels_count[i] = ((double)red_pixels_count[i] / max) * histogram_png.rows;
	}

	//drawing lines in histogram
	for (int i = 0; i < 255; i++)
	{
		cv::line(histogram_png, cv::Point(i, histogram_png.rows - blue_pixels_count[i]),
			cv::Point(i + 1, histogram_png.rows - blue_pixels_count[i + 1]),
			cv::Scalar(255, 0, 0), 1, 8, 0);
	}
	for (int i = 0; i < 255; i++)
	{
		cv::line(histogram_png, cv::Point(i, histogram_png.rows - green_pixels_count[i]),
			cv::Point(i + 1, histogram_png.rows - green_pixels_count[i + 1]),
			cv::Scalar(0, 255, 0), 1, 8, 0);
	}
	for (int i = 0; i < 255; i++)
	{
		cv::line(histogram_png, cv::Point(i, histogram_png.rows - red_pixels_count[i]),
			cv::Point(i + 1, histogram_png.rows - red_pixels_count[i + 1]),
			cv::Scalar(0, 0, 255), 1, 8, 0);
	}

	//jpeg histogram

	cv::Mat histogram_jpeg(256, 256, CV_8UC3, cv::Scalar(240, 240, 240));
	getBrightness(single_channel_jpeg[0], blue_pixels_count);
	getBrightness(single_channel_jpeg[1], green_pixels_count);
	getBrightness(single_channel_jpeg[2], red_pixels_count);

	//getting max height
	max = 0;
	for (int i = 0; i < 256; i++) {
		if (max < blue_pixels_count[i]) {
			max = blue_pixels_count[i];
		}
	}
	for (int i = 0; i < 256; i++) {
		if (max < green_pixels_count[i]) {
			max = green_pixels_count[i];
		}
	}
	for (int i = 0; i < 256; i++) {
		if (max < red_pixels_count[i]) {
			max = red_pixels_count[i];
		}
	}

	//downscaling considering max height
	for (int i = 0; i < 256; i++) {
		blue_pixels_count[i] = ((double)blue_pixels_count[i] / max) * histogram_jpeg.rows;
	}
	for (int i = 0; i < 256; i++) {
		green_pixels_count[i] = ((double)green_pixels_count[i] / max) * histogram_jpeg.rows;
	}
	for (int i = 0; i < 256; i++) {
		red_pixels_count[i] = ((double)red_pixels_count[i] / max) * histogram_jpeg.rows;
	}

	//drawing lines in histogram
	for (int i = 0; i < 255; i++)
	{
		cv::line(histogram_jpeg, cv::Point(i, histogram_jpeg.rows - blue_pixels_count[i]),
			cv::Point(i + 1, histogram_jpeg.rows - blue_pixels_count[i + 1]),
			cv::Scalar(255, 0, 0), 1, 8, 0);
	}
	for (int i = 0; i < 255; i++)
	{
		cv::line(histogram_jpeg, cv::Point(i, histogram_jpeg.rows - green_pixels_count[i]),
			cv::Point(i + 1, histogram_jpeg.rows - green_pixels_count[i + 1]),
			cv::Scalar(0, 255, 0), 1, 8, 0);
	}
	for (int i = 0; i < 255; i++)
	{
		cv::line(histogram_jpeg, cv::Point(i, histogram_jpeg.rows - red_pixels_count[i]),
			cv::Point(i + 1, histogram_jpeg.rows - red_pixels_count[i + 1]),
			cv::Scalar(0, 0, 255), 1, 8, 0);
	}
	
	
	//creating hisogram mosaic

	cv::hconcat(image_png, image_jpeg, mosaic_jpeg_top);
	cv::hconcat(histogram_png, histogram_jpeg, mosaic_jpeg_bot);
	cv::vconcat(mosaic_jpeg_top, mosaic_jpeg_bot, mosaic_jpeg);
	cv::imwrite("cross_0256x0256_hists.png", mosaic_jpeg);


	cv::waitKey(0);
}