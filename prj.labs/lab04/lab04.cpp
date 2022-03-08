#include <opencv2/opencv.hpp>

cv::Mat makeHist(cv::Mat img) {
	//making brightness array
	int brightness_array[256] = { 0 };
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			brightness_array[img.at<uint8_t>(i, j)]++;
		}
	}
	//counting the highest point on a histogram for downscaling
	int max = 0;
	for (int i = 0; i < 256; i++) {
		if (brightness_array[i] > max) {
			max = brightness_array[i];
		}
	}
	//downscaling to 256x512
	for (int i = 0; i < 256; i++) {
		brightness_array[i] = (double)brightness_array[i] / max * 512;
	}
	cv::Mat hist(512, 256, CV_8UC1);
	hist = 0;
	//drawing hist
	for (int i = 0; i < 256; i++) {
		cv::line(hist, cv::Point(i, 512), 
			cv::Point(i, 512 - brightness_array[i]),
			200, 1, 8, 0);
	}
	return hist;
}

int process_video(std::string video_name, int threshold, int morphology_type) {
	//opening an instance of video by the calculated path
	cv::VideoCapture video("../../../data/" + video_name);

	video_name = video_name.substr(0, video_name.size() - 4);

	//if it's not opened, returning error
	if (!video.isOpened()) {
		std::cout << "ERROR! THE VIDEO IS NOT OPENED.\n";
		return -1;
	}
	int nFrames = video.get(cv::CAP_PROP_FRAME_COUNT);
	
	cv::Mat frames[3]; //arrays for 3 frames from video
	cv::Mat framesGS[3];
	cv::Mat framesBin[3];

	//arrays for connecteedComponentsWithStats output
	cv::Mat connectedComponents[3];
	cv::Mat stats[3];
	cv::Mat centroids[3];

	for (int i = 0; i < 3; i++) {
		//counting current frame
		int frame_number = nFrames / 5 * (i + 2); 

		//getting the current frame
		video.set(cv::CAP_PROP_POS_FRAMES, frame_number);

		video >> frames[i];

		//creating a filename to write frame
		//std::string frame_path = "frames/" + video_name + "." + std::to_string(i + 1) + ".png";
		cv::imwrite("frames/" + video_name + "_orig_" + std::to_string(i + 1) + ".png", frames[i]); //saving img

		
		//color reduction & binarization
		cv::cvtColor(frames[i], frames[i], cv::COLOR_BGR2GRAY); //color reduction
		cv::imwrite("frames/" + video_name + "_gs_" + std::to_string(i + 1) + ".png", frames[i]); //saving img

		cv::imshow("hist " + std::to_string(i + 1), makeHist(frames[i])); //showing hist
		cv::threshold(frames[i], frames[i], threshold, 255, 0); //binarization
		cv::imwrite("frames/" + video_name + "_bin_" + std::to_string(i + 1) + ".png", frames[i]); //saving img


		//morphology
		cv::Mat structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(6, 10));
		cv::morphologyEx(frames[i], frames[i], morphology_type, structuring_element);
		cv::morphologyEx(frames[i], frames[i], cv::MORPH_OPEN, structuring_element);
		structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(45, 45));
		cv::morphologyEx(frames[i], frames[i], cv::MORPH_DILATE, structuring_element);
		
		//int nLables = cv::connectedComponentsWithStats(frames[i], connectedComponents[i], stats[i], centroids[i], 8, CV_16U);

		//test
		cv::Mat labelImage(frames[i].size(), CV_32S);
		int nLabels = cv::connectedComponents(frames[i], labelImage, 8);
		std::vector<cv::Vec3b> colors(nLabels);
		colors[0] = cv::Vec3b(0, 0, 0);//background
		for (int label = 1; label < nLabels; ++label) {
			colors[label] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
		}
		cv::Mat dst(frames[i].size(), CV_8UC3);
		for (int r = 0; r < dst.rows; ++r) {
			for (int c = 0; c < dst.cols; ++c) {
				int label = labelImage.at<int>(r, c);
				cv::Vec3b& pixel = dst.at<cv::Vec3b>(r, c);
				pixel = colors[label];
			}
		}
		imshow("Connected Components " + std::to_string(i + 1), dst);
		

	}
	cv::waitKey(0);
	video.release();
	return 0;
}

int main() {
	
	process_video("100Rub.MOV", 180, cv::MORPH_CLOSE);
	process_video("1kRub.MOV", 190, cv::MORPH_CLOSE);
	process_video("2kRub.MOV", 127, cv::MORPH_CLOSE); //+OPEN
	process_video("5kRub.MOV", 170, cv::MORPH_CLOSE);
	
	// еще 5 видео

	}	