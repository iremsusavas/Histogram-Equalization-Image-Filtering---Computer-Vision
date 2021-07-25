#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include<vector>
#include "filter.h"
using namespace std;
using namespace cv;

Mat RGB_image2;



// hists = vector of 3 cv::mat of size nbins=256 with the 3 histograms
// e.g.: hists[0] = cv:mat of size 256 with the red histogram
//       hists[1] = cv:mat of size 256 with the green histogram
//       hists[2] = cv:mat of size 256 with the blue histogram
void showHistogram(std::vector<cv::Mat>& hists)
{
	// Min/Max computation
	double hmax[3] = { 0,0,0 };
	double min;
	cv::minMaxLoc(hists[0], &min, &hmax[0]);
	cv::minMaxLoc(hists[1], &min, &hmax[1]);
	cv::minMaxLoc(hists[2], &min, &hmax[2]);

	std::string wname[3] = { "blue", "green", "red" };
	cv::Scalar colors[3] = { cv::Scalar(255,0,0), cv::Scalar(0,255,0),
							 cv::Scalar(0,0,255) };

	std::vector<cv::Mat> canvas(hists.size());

	// Display each histogram in a canvas
	for (int i = 0, end = hists.size(); i < end; i++)
	{
		canvas[i] = cv::Mat::ones(125, hists[0].rows, CV_8UC3);

		for (int j = 0, rows = canvas[i].rows; j < hists[0].rows - 1; j++)
		{
			cv::line(
				canvas[i],
				cv::Point(j, rows),
				cv::Point(j, rows - (hists[i].at<float>(j) * rows / hmax[i])),
				hists.size() == 1 ? cv::Scalar(200, 200, 200) : colors[i],
				1, 8, 0
			);
		}

		cv::imshow(hists.size() == 1 ? "value" : wname[i], canvas[i]);
		waitKey(0);
	}
}



enum Filters {MEDIAN, GAUSSIAN,BILATERAL};
struct Filterparameters
{	Filters type;
	Filter* filter;
	string name_of_window;
	int kernel_size;
	int sigmax;
	int sigmay;
};

//Calculating Histograms and showing the graph results
void histogram(cv::Mat image, vector<Mat> planes, string window_) {
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	split(image, planes);
	cv::Mat blue, green, red;
	calcHist(&planes[0], 1, 0, Mat(), blue, 1, &histSize, &histRange);
	calcHist(&planes[1], 1, 0, Mat(), green, 1, &histSize, &histRange);
	calcHist(&planes[2], 1, 0, Mat(), red, 1, &histSize, &histRange);

	std::vector<cv::Mat> lists;
	lists.push_back(blue);
	lists.push_back(green);
	lists.push_back(red);

	showHistogram(lists);


}

//Trackbar OnChange method
static void onChangetrackbar(int, void*parameters) {
	Filterparameters* filter_p = static_cast<Filterparameters*>(parameters);
	Filter* filter = filter_p->filter;

	switch(filter_p->type) {
	case MEDIAN: //median blur
	{
		filter->setSize(filter_p->kernel_size);
		break;
	}
	case GAUSSIAN: //Gaussian Blur
	{
		GaussianFilter *gaussian_filter = dynamic_cast<GaussianFilter*>(filter);
		gaussian_filter->setSize(filter_p->kernel_size);
		gaussian_filter->setSigma(filter_p->sigmax);
		break;

	}
	case BILATERAL: {  //bilateral
		BilateralFilter *bilateral_filter = dynamic_cast<BilateralFilter*>(filter);
		bilateral_filter->setSigmaRange(filter_p->sigmax);
		bilateral_filter->setSigmaSpace(filter_p->sigmay);
		break;
	}
	}
	filter->doFilter();
	imshow(filter_p->name_of_window, filter->getResult());

}

//Creating trackbars
void applying_filters(cv::Mat input_image) {

	MedianFilter median_filter(input_image, 1);
	Filterparameters median_params = { MEDIAN, &median_filter, "median", 1 };

	GaussianFilter gaussian_filter(input_image, 1);
	Filterparameters gaussian_params = { GAUSSIAN, &gaussian_filter, "gaussian", 1, 1 };

	BilateralFilter bilateral_filter(input_image, 1);
	Filterparameters bilateral_params = { BILATERAL, &bilateral_filter, "bilateral", 8, 1, 1 };
	
	namedWindow("median");
	namedWindow("gaussian");
	namedWindow("bilateral");

	// Add parameters trackbars to windows
	createTrackbar("kernel size", "median", &median_params.kernel_size, 99, onChangetrackbar, (void*)&median_params);

	createTrackbar("kernel size", "gaussian", &gaussian_params.kernel_size, 16, onChangetrackbar, (void*)&gaussian_params);
	createTrackbar("sigma", "gaussian", &gaussian_params.sigmax, 99, onChangetrackbar, (void*)&gaussian_params);

	createTrackbar("range", "bilateral", &bilateral_params.sigmax, 200, onChangetrackbar, (void*)&bilateral_params);
	createTrackbar("space", "bilateral", &bilateral_params.sigmay, 200, onChangetrackbar, (void*)&bilateral_params);
	
	onChangetrackbar(0, &median_params);
	onChangetrackbar(0, &gaussian_params);
	onChangetrackbar(0, &bilateral_params);

	waitKey(0);

}
int main() {

		//1 loading the image
	Mat image = imread("data/countryside.jpg", IMREAD_COLOR);
	

	if (image.empty()) {  //check if the image successfully loaded

		cout << "Image not loaded succesfully";
	}


		//Calculating the histogram of original image
	std::vector<cv::Mat> histograms;
	imshow("Original Image", image);
	waitKey(0);
	histogram(image,histograms,"OriginalImage"); //histogram function is called
	
		//Histogram Equalization of channels
	vector<Mat> channels;
	split(image, channels);
	Mat B, G, R;

	equalizeHist(channels[0], B); //Equalization applied on R,G,B channels
	equalizeHist(channels[1], G);
	equalizeHist(channels[2], R);

	std::vector<cv::Mat> combined;
	combined.push_back(B);
	combined.push_back(G);
	combined.push_back(R);
	Mat result;

	merge(combined, result);
	imshow("Histogram Equalization Applied", result);  //Showing the equalized image
	waitKey(1);
	cv::Mat blue_hist2, green_hist2, red_hist2;


	std::vector<cv::Mat> histograms2;
	histogram(result, histograms2, "Histogram Equalization Applied");  //Showing the histogram of the channels
	

	

		// WORKING WITH HSV COLOR SPACE
	Mat hsv_image;
	cvtColor(image, hsv_image, COLOR_BGR2HSV); //converting colorspace to hsv
	vector<Mat> histograms_hsv;
	split(hsv_image, histograms_hsv);
	int equalize_channel = 2;
	
	equalizeHist(histograms_hsv[equalize_channel], histograms_hsv[equalize_channel]); //Equalization applied on V channel
	merge(histograms_hsv, hsv_image);
	cvtColor(hsv_image, hsv_image, COLOR_HSV2BGR); // Switch back to the RGB color space and visualize the resulting image
	imshow("Equalization of HSV image on V channel", hsv_image);
	waitKey(0);
	histogram(hsv_image, histograms_hsv, "Equalized Image");
	
	// Equalizing only on Value channel which seems to be the best choice. And show histogram.


	//Creating trackbar
	RGB_image2 = hsv_image;  //will be used in trackbar
	applying_filters(RGB_image2);  //to see the trackbars
		


			return 0;
	}


