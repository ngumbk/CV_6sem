#include <opencv2/opencv.hpp>

double estimate_quality(cv::Mat mask, cv::Mat standard_mask) {
	double quality = 0;
	
	for (int i = 0; i < mask.rows; i++) {
		for (int j = 0; j < mask.cols; j++) {
			if (mask.at<uint8_t>(i, j) == standard_mask.at<uint8_t>(i, j)) {
				quality++;
			}
		}
	}
	quality = quality / (mask.rows * mask.cols) * 100;

	return quality;
}

int process_video(std::string video_name) {
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

		//saving orig img
		cv::imwrite("frames/" + video_name + "_ORIG_" + std::to_string(i + 1) + ".png", frames[i]);

		//color reduction & binarization
		cv::cvtColor(frames[i], framesGS[i], cv::COLOR_BGR2GRAY); //color reduction
		cv::imwrite("frames/" + video_name + "_GS_" + std::to_string(i + 1) + ".png", framesGS[i]); //saving gs img

		cv::threshold(framesGS[i], framesBin[i], 188, 255, cv::THRESH_BINARY); //binarization

			//inverting a binarization in case of high percentage of white pixels
		double white_pixels_percentage = 0;
		for (int j = 0; j < framesBin[i].rows; j++) {
			for (int k = 0; k < framesBin[i].cols; k++) {
				if (framesBin[i].at<uint8_t>(j, k) == 255) {
					white_pixels_percentage++;
				}
			}
		}
		white_pixels_percentage = white_pixels_percentage / (framesBin[i].rows * framesBin[i].cols) * 100;
		if (white_pixels_percentage > 50) {
			cv::bitwise_not(framesBin[i], framesBin[i]);
		}

		cv::imwrite("frames/" + video_name + "_BIN_" + std::to_string(i + 1) + ".png", framesBin[i]); //saving bin img

		//morphology

		cv::Mat structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(6, 12));

		cv::morphologyEx(framesBin[i], framesMorph[i], cv::MORPH_CLOSE, structuring_element);
		cv::morphologyEx(framesMorph[i], framesMorph[i], cv::MORPH_OPEN, structuring_element);

		structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(50, 50));
		cv::morphologyEx(framesMorph[i], framesMorph[i], cv::MORPH_DILATE, structuring_element);

		cv::imwrite("frames/" + video_name + "_MORPH_" + std::to_string(i + 1) + ".png", framesMorph[i]); //saving bin img

		//finding the main connected component
			//arrays for connecteedComponentsWithStats output
		cv::Mat labeledImage(framesMorph[i].size(), CV_32S), stats, centroids;
			//labeling connected components and gathering stats
		int nLabels = cv::connectedComponentsWithStats(framesMorph[i], labeledImage, stats, centroids, 8, CV_32S);

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
		
		framesCC[i] = frames[i].clone();
		framesCC[i] = 0;

			//leaving the main component only visible
		for (int j = 0; j < labeledImage.rows; j++) {
			for (int k = 0; k < labeledImage.cols; k++) {
				int label = labeledImage.at<int>(j, k);
				cv::Vec3b& pixel = framesCC[i].at<cv::Vec3b>(j, k);
				pixel = colors[label];
			}
		}
		cv::imwrite("frames/" + video_name + "_CC_" + std::to_string(i + 1) + ".png", framesCC[i]); //saving img with CCs

		//estimating the quality of the mask, we've got
		cv::Mat standard_mask = cv::imread("../../../data/masks_lab04/" + video_name + "_MASK_" + std::to_string(i + 1) + ".png");
		std::cout << "Quality of " + video_name + " " << i + 1 << ": " << estimate_quality(framesCC[i], standard_mask) << std::endl;
	}
	std::cout << std::endl;

	video.release();
	return 0;
}

int main() {
	process_video("100Rub.MOV");
	process_video("500Rub.mp4");
	process_video("1kRub.MOV");
	process_video("2kRub.MOV");
	process_video("5kRub.MOV");
}