#include <opencv2/opencv.hpp>

long GetFileSize(std::string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

void parse_markup(std::string frame_name, std::vector<std::vector<cv::Point>> &points) { //1, 2, 5, 100, 500
	points.resize(3);
	cv::FileStorage fs("../../../data/Banknotes_markup.json", cv::FileStorage::READ);
	cv::FileNode points_x, points_y;
	std::string root;
	std::string file_path;
	for (int i = 0; i < 3; i++) {
		file_path = "../lab04/frames/" + frame_name + "_ORIG_" + std::to_string(i + 1) + ".png";
		root = frame_name + "_ORIG_" + std::to_string(i + 1) + ".png" + std::to_string(GetFileSize(file_path));
		points_x = fs[root]["regions"][0]["shape_attributes"]["all_points_x"];
		points_y = fs[root]["regions"][0]["shape_attributes"]["all_points_y"];
		for (int j = 0; j < 4; j++) {
			points[i].push_back(cv::Point(points_x[j], points_y[j]));
		}
	}
}

cv::Mat Greyscaling(cv::Mat input_img) {
	cv::Mat splitted_img[3], output_img;
	cv::split(input_img, splitted_img);

	cv::hconcat(splitted_img, 3, output_img);
	return output_img;
}

double estimate_error(std::vector<cv::Point> main_contour, std::vector<cv::Point> points_marked) {
	double perimeter = cv::arcLength(points_marked, true);
	double max_dist = 0, distance = 0;
	//std::cout << "POINTS:\n";
	for (int i = 0; i < points_marked.size(); i++) {
		/*
		std::cout << "Main:\t" << main_contour[i].x << " " << main_contour[i].y << "\n";
		std::cout << "Marked:\t" << points_marked[i].x << " " << points_marked[i].y << "\n\n";
		*/
		distance = cv::norm(main_contour[i] - points_marked[i]);
		if (distance > max_dist) {
			max_dist = distance;
		}
	}
	return max_dist / perimeter;
}

int find_boundaries(std::string frame_name) {
	std::vector<std::vector<cv::Point>> points;
	parse_markup(frame_name, points);
	for (int i = 0; i < 3; i++) {
		//loading the frame from the lab04 directory
		cv::Mat frame_orig = cv::imread("../lab04/frames/" + frame_name + "_ORIG_" + std::to_string(i + 1) + ".png");

		//Downscaling image to 0.125 of the original size
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

		//resizing back to original size
		cv::resize(frame_blurred, frame_blurred, cv::Size(), 8, 8);
		cv::resize(frame_Canny, frame_Canny, cv::Size(), 8, 8);
		cv::resize(frame_resized, frame_resized, cv::Size(), 8, 8);


		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		std::cout << frame_name << " " << std::to_string(i + 1) << "\n";

		cv::findContours(frame_Canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

		//approxing found contours
		std::vector<std::vector<cv::Point>> contours_approxed(contours.size());
		double c;
		for (int j = 0; j < contours.size(); j++) {
			c = 0.05;
			do {
				cv::approxPolyDP(contours[j], contours_approxed[j], cv::arcLength(contours[j], true) * c, true);
				c += 0.005;
			 } while (contours_approxed[j].size() > 4);
		}

		//finding the longest contour and drawing it
		int max_size = 0, max_i = 0;
		for (int j = 0; j < contours.size(); j++) {
			if (contours[j].size() > max_size) {
				max_size = contours[j].size();
				max_i = j;
			}
		}
		std::vector<cv::Point> main_contour = contours_approxed[max_i];
		cv::Mat main_contour_drawn(frame_Canny.size(), frame_Canny.type());
		main_contour_drawn = 0;
		for (int j = 0; j < 4; j++) {
			cv::line(main_contour_drawn, main_contour[j], main_contour[(j + 1) % 4], cv::Scalar(255, 255, 255));
		}

		//swapping points 2 and 4 in min_contour vector
		std::swap(main_contour[1], main_contour[3]);
		 
		//marked points correct, main_contour to fix
		std::cout << estimate_error(main_contour, points[i]) << "\n\n";

		cv::drawContours(frame_orig, points, i, cv::Scalar(0, 255, 0));
		cv::drawContours(frame_orig, contours_approxed, max_i, cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
		
		//cv::imshow("main_contour", main_contour_drawn);

		cv::imshow("drawcontours", frame_orig);
		//cv::imshow("Blur", frame_blurred);
		//cv::imshow("Canny", frame_Canny);

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
	return 0;
}