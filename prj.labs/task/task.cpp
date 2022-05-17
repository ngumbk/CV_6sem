#include <opencv2/opencv.hpp>

int main() {
	//(1, 1) black
	cv::Mat OneChannelImg(300, 450, CV_32FC1);
	for (int i = 0; i < 150; i++) {
		for (int j = 0; j < 150; j++) {
			OneChannelImg.at<float>(i, j) = 0;
		}
	}
	//(3, 2) black
	for (int i = 150; i < 300; i++) {
		for (int j = 300; j < 450; j++) {
			OneChannelImg.at<float>(i, j) = 0;
		}
	}
	//(1, 2), (2, 1) gray
	for (int i = 0; i < 150; i++) {
		for (int j = 150; j < 300; j++) {
			OneChannelImg.at<float>(i, j) = 127 / 255.0;
			OneChannelImg.at<float>(j, i) = 127 / 255.0;
		}
	}
	//(2, 2), (3, 1) white
	for (int i = 150; i < 300; i++) {
		for (int j = 150; j < 300; j++) {
			OneChannelImg.at<float>(i, j) = 255 / 255.0;
			OneChannelImg.at<float>(i - 150, j + 150) = 255 / 255.0;
		}
	}
	
	//circles
	//black squares
	cv::circle(OneChannelImg, cv::Point(75, 75), 70, 255 / 255.0, -1);
	cv::circle(OneChannelImg, cv::Point(375, 225), 70, 200 / 255.0, -1);

	//gray squares
	cv::circle(OneChannelImg, cv::Point(225, 75), 70, 180 / 255.0, -1);
	cv::circle(OneChannelImg, cv::Point(75, 225), 70, 100 / 255.0, -1);

	//white squares
	cv::circle(OneChannelImg, cv::Point(225, 225), 70, 50 / 255.0, -1);
	cv::circle(OneChannelImg, cv::Point(375, 75), 70, 0, -1);

	//filters
	cv::Mat I_1(2, 2, CV_32S);
	I_1 = 0;
	I_1.at<int>(0, 0) = 1;
	I_1.at<int>(1, 1) = -1;

	cv::Mat I_2(2, 2, CV_32S);
	I_2 = 0;
	I_2.at<int>(0, 1) = 1;
	I_2.at<int>(1, 0) = -1;

	//filtering
	cv::Mat I_1_filtered = OneChannelImg.clone();
	I_1_filtered = 0;
	cv::filter2D(OneChannelImg, I_1_filtered, -1, I_1, cv::Point(0, 0));

	cv::Mat I_2_filtered = OneChannelImg.clone();
	I_2_filtered = 0;
	cv::filter2D(OneChannelImg, I_2_filtered, -1, I_2, cv::Point(0, 0));

	//making middle matrix
	cv::Mat middle(OneChannelImg.size(), CV_32FC1);
	for (int i = 0; i < I_1_filtered.rows; i++) {
		for (int j = 0; j < I_1_filtered.cols; j++) {
			middle.at<float>(i, j) = std::sqrt(I_1_filtered.at<float>(i, j) * I_1_filtered.at<float>(i, j) + I_2_filtered.at<float>(i, j) * I_2_filtered.at<float>(i, j));
		}
	}

	I_1_filtered = (I_1_filtered + 1) / 2;
	I_2_filtered = (I_2_filtered + 1) / 2;

	
	cv::imwrite("original.png", OneChannelImg * 255);
	cv::imwrite("I_1.png", I_1_filtered * 255);
	cv::imwrite("I_2.png", I_2_filtered * 255);
	cv::imwrite("middle.png", middle * 255);
	

	return 0;
}