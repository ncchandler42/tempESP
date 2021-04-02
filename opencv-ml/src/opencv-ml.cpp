#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[])
{
	//cv::Ptr<cv::ml::TrainData>
	auto data = cv::ml::TrainData::loadFromCSV("../data.csv", 0/*header lines*/);
	
	// Train the SVM
	/*
	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
	svm->setType(cv::ml::SVM::C_SVC);
	svm->setKernel(cv::ml::SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));
	*/
	
	//cv::Ptr<cv::ml::ANN_MLP> 
	auto mlp = cv::ml::ANN_MLP::create();
	
	int nFeatures = data->getNVars();
	int nClasses = data->getResponses().cols;

	std::cout << nFeatures << ", " << nClasses << "\n";
	
	cv::Mat_<int> layer_s(5, 1);
	layer_s(0) = nFeatures; // input
	layer_s(1) = nClasses*8; //hidden
	layer_s(2) = nClasses*24; //hidden
	layer_s(3) = nClasses*3; //hidden
	layer_s(4) = nClasses; //output

	mlp->setLayerSizes(layer_s);

	auto crit = cv::TermCriteria(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 750, 1e-6);
	mlp->setTermCriteria(crit);
	
	mlp->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM, 1, 1);
	mlp->setTrainMethod(cv::ml::ANN_MLP::BACKPROP, 1e-6);

	mlp->train(data);
	
	// Data for visual representation
	int width = 512, height = 512;
	cv::Mat image = cv::Mat::zeros(height, width, CV_8UC3);
	
	// Show the decision regions given by the mlp
	cv::Vec3b green(0,255,0), blue(255,0,0);
	
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			
			cv::Mat sample = (cv::Mat_<float>(1,2) << j,i);
			cv::Mat resultm;
			
			mlp->predict(sample, resultm);
			float response = resultm.at<float>(0);
			
			//std::cout << "(" << i << ", " << j << ") => " << resultm << "\n";

			image.at<cv::Vec3b>(i, j) = cv::Vec3b(255*response, 255*(1.0-response), 0);
	
			// 
			// if (response < 0.5)
				// image.at<cv::Vec3b>(i,j) = green;
				// 
			// else if (response >= 0.5)
				// image.at<cv::Vec3b>(i,j) = blue;
		}
	}
	
	/*	
	// Show the training data
	int thickness = -1;
	cv::circle( image, cv::Point(501,  10), 5, cv::Scalar(  0,   0,   0), thickness );
	cv::circle( image, cv::Point(255,  10), 5, cv::Scalar(255, 255, 255), thickness );
	cv::circle( image, cv::Point(501, 255), 5, cv::Scalar(255, 255, 255), thickness );
	cv::circle( image, cv::Point( 10, 501), 5, cv::Scalar(255, 255, 255), thickness );
	

	// Show support vectors
	thickness = 2;
	cv::Mat sv = svm->getUncompressedSupportVectors();
	for (int i = 0; i < sv.rows; i++)
	{
		const float* v = sv.ptr<float>(i);
		cv::circle(image,  cv::Point( (int) v[0], (int) v[1]), 6, cv::Scalar(128, 128, 128), thickness);
	}
	*/
	
	//cv::imwrite("result.png", image);		// save the image
	cv::imshow("MLP Simple Example", image); // show it to the user
	cv::waitKey();
	return 0;
}
