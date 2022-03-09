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

int process_video(std::string video_name, int threshold) {
	//opening an instance of video by the calculated path
	cv::VideoCapture video("../../../data/videos_lab04/" + video_name);

	video_name = video_name.substr(0, video_name.size() - 4);

	//if it's not opened, returning error
	if (!video.isOpened()) {
		std::cout << "ERROR! THE VIDEO IS NOT OPENED.\n";
		return -1;
	}

	//getting the total amount of frames in the video
	int nFrames = video.get(cv::CAP_PROP_FRAME_COUNT);
	
	//arrays for each stage of frame processing
	cv::Mat frames[3], framesGS[3], framesBin[3], framesMorph[3], framesCC[3];

	for (int i = 0; i < 3; i++) {
		//counting current frame
		int frame_number = nFrames / 5 * (i + 2); 

		//getting the current frame
		video.set(cv::CAP_PROP_POS_FRAMES, frame_number);
		video >> frames[i];

		//creating a filename to write frame
		//std::string frame_path = "frames/" + video_name + "." + std::to_string(i + 1) + ".png";
		cv::imwrite("frames/" + video_name + "_ORIG_" + std::to_string(i + 1) + ".png", frames[i]); //saving orig img

		
		//color reduction & binarization
		cv::cvtColor(frames[i], framesGS[i], cv::COLOR_BGR2GRAY); //color reduction
		cv::imwrite("frames/" + video_name + "_GS_" + std::to_string(i + 1) + ".png", framesGS[i]); //saving gs img

		//cv::imwrite("hist " + video_name + std::to_string(i + 1) + ".png", makeHist(framesGS[i])); //saving hist

		cv::threshold(framesGS[i], framesBin[i], threshold, 255, cv::THRESH_BINARY); //binarization
		cv::imwrite("frames/" + video_name + "_BIN_" + std::to_string(i + 1) + ".png", framesBin[i]); //saving bin img


		//morphology
		
		cv::Mat structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(6, 10));
		cv::morphologyEx(framesBin[i], framesMorph[i], cv::MORPH_CLOSE, structuring_element);
		cv::morphologyEx(framesMorph[i], framesMorph[i], cv::MORPH_OPEN, structuring_element);

		structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(50, 50));
		cv::morphologyEx(framesMorph[i], framesMorph[i], cv::MORPH_DILATE, structuring_element);
		
		cv::imwrite("frames/" + video_name + "_MORPH_" + std::to_string(i + 1) + ".png", framesMorph[i]); //saving bin img

		//arrays for connecteedComponentsWithStats output
		cv::Mat labeledImage(framesMorph[i].size(), CV_32S), stats, centroids;
		//labeling connected components and gathering stats
		int nLabels = cv::connectedComponentsWithStats(framesMorph[i], labeledImage, stats, centroids, 8, CV_32S); 
		
		//writing info about CCs
		std::cout << "\nnLables: " << nLabels << "\n";
		std::cout << video_name << std::endl;
		for (int j = 0; j < nLabels; j++) {
			std::cout << "frame: " << i + 1 << " label: " << j + 1 << " area: " << stats.at<int>(j, cv::CC_STAT_AREA) << std::endl;
		}

		//finding main connected components by area, providing it's the biggest except background
		int max_area = 0, max_label = 1;

		for (int j = 1; j < nLabels; j++) {
			if (max_area < stats.at<int>(j, cv::CC_STAT_AREA)) {
				max_area = stats.at<int>(j, cv::CC_STAT_AREA);
				max_label = j;
			}
		}
		
		//creating vector of colors
		std::vector<cv::Vec3b> colors(nLabels);
		for (int j = 0; j < nLabels; j++) {
			colors[j] = cv::Vec3b(0, 0, 0);
		}
		colors[max_label] = cv::Vec3b(255, 255, 255);

		//test
		framesCC[i] = frames[i].clone();
		framesCC[i] = 0;
		//cv::Mat dst(frames[i].size(), frames[i].type());
		//dyeing CCs with random colors
		for (int j = 0; j < labeledImage.rows; j++) {
			for (int k = 0; k < labeledImage.cols; k++) {
				int label = labeledImage.at<int>(j, k);
				cv::Vec3b& pixel = framesCC[i].at<cv::Vec3b>(j, k);
				pixel = colors[label];
			}
		}
		cv::imwrite("frames/" + video_name + "_CC_" + std::to_string(i + 1) + ".png", framesCC[i]); //saving img with CCs

	}
	cv::waitKey(0);
	video.release();
	return 0;
}

int main() {
	process_video("100Rub.MOV", 200);
	process_video("1kRub.MOV", 190);
	process_video("1kRub2.MOV", 210);
	process_video("2kRub.MOV", 127);
	process_video("5kRub.MOV", 185);
	}	