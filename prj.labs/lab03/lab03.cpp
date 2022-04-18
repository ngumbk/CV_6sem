#include <opencv2/opencv.hpp>

//counting LUT in order to shorten processing time
void countLUT(int* LUT, double k) {
	for (int i = 0; i < 256; i++) {
		LUT[i] = 255 * sin(i / k);
	}
}

//changing brightness of grayscale imgs only
cv::Mat changeBrightnessGS(cv::Mat grayscale_img, int *LUT) {
	cv::Mat destiny_img = grayscale_img.clone();
	for (int i = 0; i < destiny_img.rows; i++) {
		for (int j = 0; j < destiny_img.cols; j++) {
			destiny_img.at<uint8_t>(i, j) = LUT[destiny_img.at<uint8_t>(i, j)];
		}
	}
	return destiny_img;
}

//changing brightness of 3-channel image using LUT
cv::Mat changeBrightness(cv::Mat image, int *LUT) {
	cv::Mat img_channels[3], destiny_img;
	cv::split(image, img_channels);
	img_channels[0] = changeBrightnessGS(img_channels[0], LUT);
	img_channels[1] = changeBrightnessGS(img_channels[1], LUT);
	img_channels[2] = changeBrightnessGS(img_channels[2], LUT);

	cv::merge(img_channels, 3, destiny_img);
	return destiny_img;
}

int main() {
	//reading and saving pic as a new file
	cv::Mat img = cv::imread("../../../data/cross_0256x0256.png");

	for (int i = 1; i < 400; i++) {
		//Calculating LUT for the function and processing BGR img
		int LUT[256];
		countLUT(LUT, i);
		cv::Mat processed_img = changeBrightness(img, LUT).clone();
		cv::imwrite("lab03_rgb_res.png", processed_img);

	//drawing graphic
	/*
	cv::Mat function_graphic(512, 512, CV_8UC1, cv::Scalar(255, 255, 255));
	for (int i = 0; i < function_graphic.cols - 1; i++) {
		cv::line(function_graphic,
			cv::Point(i, function_graphic.cols - LUT[i / 2] * 2),
			cv::Point(i + 1, function_graphic.cols - LUT[(i + 1) / 2] * 2),
			cv::Scalar(0, 0, 0), 1, 8, 0);
	}
	//drawing coordinate axes
	cv::line(function_graphic,
		cv::Point(0, 0),
		cv::Point(0, function_graphic.cols),
		cv::Scalar(0, 0, 0), 4, 8, 0);
	cv::line(function_graphic,
		cv::Point(0, function_graphic.cols),
		cv::Point(function_graphic.cols, function_graphic.cols),
		cv::Scalar(0, 0, 0), 4, 8, 0);
	cv::imwrite("lab03_viz_func.png", function_graphic);
	*/
}