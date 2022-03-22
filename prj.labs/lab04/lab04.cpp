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
	cv::Mat frame, frameGS, frameBin, frameMorph, frameCC;

	for (int i = 0; i < 3; i++) {
		//counting current frame
		int frame_number = nFrames / 5 * (i + 2);

		//getting the current frame
		video.set(cv::CAP_PROP_POS_FRAMES, frame_number);
		video >> frame;

		//saving orig img
		cv::imwrite("frames/" + video_name + "_ORIG_" + std::to_string(i + 1) + ".png", frame);

		//color reduction & binarization
		cv::cvtColor(frame, frameGS, cv::COLOR_BGR2GRAY); //color reduction
		cv::imwrite("frames/" + video_name + "_GS_" + std::to_string(i + 1) + ".png", frameGS); //saving gs img

		cv::threshold(frameGS, frameBin, 188, 255, cv::THRESH_BINARY); //binarization

			//inverting a binarization in case of high percentage of white pixels
		double white_pixels_percentage = 0;
		for (int j = 0; j < frameBin.rows; j++) {
			for (int k = 0; k < frameBin.cols; k++) {
				if (frameBin.at<uint8_t>(j, k) == 255) {
					white_pixels_percentage++;
				}
			}
		}
		white_pixels_percentage = white_pixels_percentage / (frameBin.rows * frameBin.cols) * 100;
		if (white_pixels_percentage > 50) {
			cv::bitwise_not(frameBin, frameBin);
		}

		cv::imwrite("frames/" + video_name + "_BIN_" + std::to_string(i + 1) + ".png", frameBin); //saving bin img

		//morphology

		cv::Mat structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(6, 12));

		cv::morphologyEx(frameBin, frameMorph, cv::MORPH_CLOSE, structuring_element);
		cv::morphologyEx(frameMorph, frameMorph, cv::MORPH_OPEN, structuring_element);

		structuring_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(100, 50));
		cv::morphologyEx(frameMorph, frameMorph, cv::MORPH_CLOSE, structuring_element);

		cv::imwrite("frames/" + video_name + "_MORPH_" + std::to_string(i + 1) + ".png", frameMorph); //saving bin img

		//finding the main connected component
			//arrays for connecteedComponentsWithStats output
		cv::Mat labeledImage(frameMorph.size(), CV_32S), stats, centroids;
			//labeling connected components and gathering stats
		int nLabels = cv::connectedComponentsWithStats(frameMorph, labeledImage, stats, centroids, 8, CV_32S);

			//finding main connected components by area, providing it's the biggest except background
		int max_area = 0, max_label = 1;

		for (int j = 1; j < nLabels; j++) {
			if (max_area < stats.at<int>(j, cv::CC_STAT_AREA)) {
				max_area = stats.at<int>(j, cv::CC_STAT_AREA);
				max_label = j;
			}
		}

			//creating vector of colors
		std::vector<uint8_t> colors(nLabels);
		for (int j = 0; j < nLabels; j++) {
			colors[j] = 0;
		}
		colors[max_label] = 255;
		
		frameCC = frame.clone();
		cv::cvtColor(frameCC, frameCC, cv::COLOR_BGR2GRAY);
		frameCC = 0;

			//leaving the main component only visible
		for (int j = 0; j < labeledImage.rows; j++) {
			for (int k = 0; k < labeledImage.cols; k++) {
				int label = labeledImage.at<int>(j, k);
				uint8_t& pixel = frameCC.at<uint8_t>(j, k);
				pixel = colors[label];
			}
		}
		cv::imwrite("frames/" + video_name + "_CC_" + std::to_string(i + 1) + ".png", frameCC); //saving img with CCs

		//estimating the quality of the mask, we've got
		cv::Mat standard_mask = cv::imread("../../../data/masks_lab04/" + video_name + "_MASK_" + std::to_string(i + 1) + ".png").clone();
		cv::cvtColor(standard_mask, standard_mask, cv::COLOR_BGR2GRAY);
		std::cout << "Quality of " + video_name + " " << i + 1 << ": " << estimate_quality(frameCC, standard_mask) << std::endl;

		//creating an image with mask overlaying original
		cv::Mat mask_over_original(frameCC.size(), CV_8UC3);
		mask_over_original = 0;
			//4 cases - 4 colors
		for (int j = 0; j < frame.rows; j++) {
			for (int k = 0; k < frame.cols; k++) {
				if ((standard_mask.at<uint8_t>(j, k) == 0) && (frameCC.at<uint8_t>(j, k) == 0)) {
					mask_over_original.at<cv::Vec3b>(j, k) = cv::Vec3b(0, 0, 0);
				}
				else if ((standard_mask.at<uint8_t>(j, k) == 0) && (frameCC.at<uint8_t>(j, k) == 255)) {
					mask_over_original.at<cv::Vec3b>(j, k) = cv::Vec3b(0, 0, 255);
				}	
				else if ((standard_mask.at<uint8_t>(j, k) == 255) && (frameCC.at<uint8_t>(j, k) == 0)) {
					mask_over_original.at<cv::Vec3b>(j, k) = cv::Vec3b(255, 0, 0);
				}
				else if ((standard_mask.at<uint8_t>(j, k) == 255) && (frameCC.at<uint8_t>(j, k) == 255)) {
					mask_over_original.at<cv::Vec3b>(j, k) = cv::Vec3b(255, 255, 255);
				}
			}
		}
			//combining with the original
		cv::addWeighted(frame, 0.5, mask_over_original, 0.5, 0.0, mask_over_original);
		cv::imwrite("frames/" + video_name + "_final_" + std::to_string(i + 1) + ".png", mask_over_original); //saving the final img
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