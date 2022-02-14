#include <opencv2/opencv.hpp>

int main() {
  cv::Mat I_1(60, 768, CV_8UC1);
  // draw dummy image
  I_1 = 0;
  int colour = 0;
  for (int i = 0; i < I_1.rows; i++) {
	  for (int j = 0; j < I_1.cols; j++) {
		  I_1.at<uchar>(i, j) = j / 3;
	  }
  }
  // save result
  cv::imwrite("I_1.png", I_1);
  
  cv::TickMeter tm;

  //Гамма-коррекция с помощью cv::pow()
	
  cv::Mat G_1 = I_1.clone();
  tm.start();
  I_1.convertTo(G_1, CV_32FC1, 1/255.0);
  cv::pow(G_1, 2.3, G_1);
  G_1.convertTo(G_1, CV_8UC1, 255.0);
  tm.stop();
  std::cout << "Total time G_1: " << tm.getTimeSec() << std::endl;
  tm.reset();
  cv::imwrite("G_1.png", G_1);

  //Попиксельная гамма-коррекция

  cv::Mat G_2 = I_1.clone();
  tm.start();
  for (int i = 0; i < G_2.rows; i++) {
	  for (int j = 0; j < G_2.cols; j++) {
		  G_2.at<uint8_t>(i, j) = (std::pow(G_2.at<uint8_t>(i, j) / 255.0, 2.3) * 255.0); //коэф 2.2
	  }
  }
  tm.stop();
  std::cout << "Total time G_2: " << tm.getTimeSec() << std::endl;
  cv::imwrite("G_2.png", G_2);

  cv::Mat img(180, 768, CV_8UC1);
  img = 0;
  cv::Mat matArray[] = { I_1, G_1, G_2 };
  cv::vconcat(matArray, 3, img);

  cv::imwrite("lab01.png", img);

  cv::waitKey(0);
}
