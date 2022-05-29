#include <opencv2/opencv.hpp>

void parse_markup(std::string frame_name, std::vector<std::vector<cv::Point>> points) { //1, 2, 5, 100, 500
	//сначала сопоставление fid и fname
	//"fid":"1","fname":"1kRub_ORIG_1.png"
	cv::FileStorage fs("../../../data/All_banknotes.json", cv::FileStorage::READ);
	cv::FileNode root = fs["file"];
	std::string fname, fid, vid;
	cv::Point coords;
	for (int i = 0; i < root.size(); i++) {
		cv::FileNode data = root[std::to_string(i + 1)];
		data["fname"] >> fname;
		fname = fname.substr(0, fname.find("_"));
		if (fname == frame_name) {
			data["fid"] >> fid;
			root = fs["metadata"];
			for (int j = 0; j < 4; j++) {
				
				data = root[fid + "_" + std::to_string(j)];
				/*
				data["vid"] >> vid;
				
				if (vid == fid) 
				*/
				coords.x = data["xy"][1];
				coords.y = data["xy"][2];
				
			}
		}
	}
}

cv::Mat Greyscaling(cv::Mat input_img) {
	cv::Mat splitted_img[3], output_img;
	cv::split(input_img, splitted_img);

	cv::hconcat(splitted_img, 3, output_img);
	return output_img;
}

int find_boundaries(std::string frame_name) {
	std::vector<std::vector<cv::Point>> points;
	parse_markup(frame_name, points);
	for (int i = 0; i < 3; i++) {
		//loading the frame from the lab04 directory
		cv::Mat frame_orig = cv::imread("../lab04/frames/" + frame_name + "_ORIG_" + std::to_string(i + 1) + ".png");

		//Shrinking image to 0.125 of the original size
		cv::Mat frame_resized; //(854, 480, CV_8UC3);
		cv::resize(frame_orig, frame_resized, cv::Size(), 0.125, 0.125);

		//Color reduction
		cv::Mat frame_gs;
		cv::cvtColor(frame_resized, frame_gs, cv::COLOR_BGR2GRAY);

		//Bluring image to sort out tresh boundaries
		cv::Mat frame_blurred;
		//cv::GaussianBlur(frame_resized, frame_blurred, cv::Size(5, 5), 7, 3, cv::BORDER_REPLICATE);
		cv::bilateralFilter(frame_resized, frame_blurred, 5, 1000, 10, cv::BORDER_REPLICATE);

		//Using Canny operator
		cv::Mat frame_Canny;
		cv::Canny(frame_blurred, frame_Canny, 140, 200); //140-200

		//dilating 1 px to restore some edges
		cv::morphologyEx(frame_Canny, frame_Canny, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 3)));
		cv::morphologyEx(frame_Canny, frame_Canny, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 1)));


		//resizing back so that we'll get the x0.5 size comparing to original
		cv::resize(frame_blurred, frame_blurred, cv::Size(), 4, 4);
		cv::resize(frame_Canny, frame_Canny, cv::Size(), 4, 4);
		cv::resize(frame_resized, frame_resized, cv::Size(), 4, 4);

		/*
		//Using Harris corner detector
		cv::Mat output_Harris;
		cv::cornerHarris(frame_Canny, output_Harris, 7, 5, 0.04, cv::BORDER_REPLICATE);
		cv::resize(output_Harris, output_Harris, cv::Size(), 0.25, 0.25);
		cv::imshow("Harris", output_Harris);
		*/

		
		

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		//cv::findContours(frame_Canny, contours, hierarchy, );
		//cv::imshow("Blur", frame_blurred);
		cv::imshow("Canny", frame_Canny);
		//cv::imshow("1-channels", Greyscaling(frame_resized));

		cv::waitKey(0);
		



	}
	return 0;
}


int main() {
	//take the same names as in the lab04 .cpp file
	find_boundaries("1kRub");
	find_boundaries("2kRub");
	find_boundaries("5kRub");
	find_boundaries("100Rub");
	find_boundaries("500Rub");
}