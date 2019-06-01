#include"image.h"
#include<conio.h>
#include <C:\opencv\build\include\opencv2/opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>


int Is_MInv_calculated;
M33 M_Inv;

int upload_image(string path, Mat &image) {
	Mat image_temp;
	string path_temp = path;
	image_temp = imread(path_temp.c_str(), IMREAD_COLOR);
	if (!image_temp.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	image = image_temp;


	return 0;
}

int ERI2Conv_forward_mapped(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1) {
	   
	for (int i = 0; i < source_image_mat.rows; ++i)
	{
		for (int j = 0; j < source_image_mat.cols; ++j)
		{

			V3 p = eri_image.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;

			if (pp[0] < camera1.w && pp[0] >= 0 && pp[1] >= 0 && pp[1] < camera1.h)
			{
				output_image_mat.at<cv::Vec3b>((int)pp[1], (int)pp[0]) = source_image_mat.at<cv::Vec3b>(i, j);  //pp[0]=column				
			}
		}
		
	}
	return 0;
}


int Conv2ERI(Mat conv_image, Mat &output_eri_image, Mat source_eri_image, ERI blank_eri_image, PPC camera1)
{	// two ERI image: source one is in Mat format blank one is ERI format.... this is done to use function unproject and vec3b operation. One type is suited for each one.
	for (int i = 0; i < blank_eri_image.h; ++i)
	{		
		for (int j = 0; j < blank_eri_image.w; ++j)
		{			
			V3 p = blank_eri_image.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;

			if (pp[0] < conv_image.cols && pp[0] >= 0 && pp[1] >= 0 && pp[1] < conv_image.rows)
			{	
				output_eri_image.at<cv::Vec3b>(i, j) = conv_image.at<cv::Vec3b>(pp[1], pp[0]);  //pp[0]=column				
			}			

		}
	}

	return 0;

}


int EachPixelConv2ERI(ERI eri_image, PPC camera1,int u, int v, int &pixelI, int &pixelJ)
{						
			V3 p = camera1.UnprojectPixel(0.5f+u,0.5f+v, camera1.GetFocalLength());		
			p = p.UnitVector();
			pixelI = eri_image.Lat2PixI(eri_image.GetXYZ2Latitude(p));
			pixelJ = eri_image.Lon2PixJ(eri_image.GetXYZ2Longitude(p));

			return 0;
}




int ERI2Conv(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, PPC camera1)
{
	int pixelI, pixelJ = 0;

	for (int v = 0; v < camera1.h; v++)
	{
		for (int u = 0; u < camera1.w; u++)
		{
			
			EachPixelConv2ERI(eri_image, camera1, u, v, pixelI, pixelJ);
			output_image_mat.at<cv::Vec3b>(v, u) = source_image_mat.at<cv::Vec3b>(pixelI, pixelJ);		
					
		}
	}
	

	return 0;
}

void mouse_control(Mat source_image_mat, Mat output_image_mat, ERI eri_image,PPC camera1)
{
	int cond = 1;
	while (cond == 1)
	{

		ERI2Conv(source_image_mat, output_image_mat, eri_image, camera1); 			
		imshow("CONV_image", output_image_mat);
		waitKey(1);

		int ch = getch();
		cout << ch << endl;
		switch (ch)
		{
		case 77:
			camera1.Pan(5.0f);
			break;
		case 75:
			camera1.Pan(-5.0f);
			break;
		case 72:
			camera1.Tilt(5.0f);
			break;
		case 80:
			camera1.Tilt(-5.0f);
			break;
		case 113:
			cond = 0;
			break;

		}
	}
	
}
void forward_backward(Mat source_image_mat, Mat output_image_mat, Mat output_image_mat_reverse, ERI eri_image, PPC camera1)
{
	ERI2Conv(source_image_mat, output_image_mat, eri_image, camera1);
	imshow("CONV_image", output_image_mat);
	img_write("./Image/CONV_image.png", output_image_mat);// write an image
	Conv2ERI(output_image_mat, output_image_mat_reverse, source_image_mat, eri_image, camera1);
	imshow("ERI_image_reverse", output_image_mat_reverse);
	img_write("./Image/reverse_image.png", output_image_mat_reverse);// write an image
	waitKey(0);
	system("pause");
	
}

void img_write(const char *s1, cv::InputArray s2) {

	vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	string str1 ="\""+string(s1)+ "\"";
	imwrite(s1, s2, compression_params);
	waitKey(1);
}



void play(Mat &source_image_mat, Mat &output_image_mat, ERI eri_image, Path path_given)
{
	for (int i = 0; i < path_given.cam_array_size(); i++)
	{
		path_given.cams[i].Pan(i * 30);
		ERI2Conv(source_image_mat, output_image_mat, eri_image, path_given.cams[i]);
		imshow("CONV_image", output_image_mat);
		waitKey(100);

	}
}
//*/

void read_file(Path &path1) {

	

	cout << "hukka" << endl;
	vector<vector<double> >     data;
	ifstream  file("1.txt");
	if (!file)
	{
		cout << "error" << endl;
		system("pause");
	}
	string   line;
	while (getline(file, line))
	{
		vector<double>   lineData;
		stringstream  lineStream(line);

		double value;
		// Read an integer at a time from the line
		while (lineStream >> value)
		{
			
			// Add the integers from a line to a 1D array (vector)
			lineData.push_back(value);
		}
		// When all the integers have been read, add the 1D array
		// into a 2D array (as one line in the 2D array)
		float theta, v1, v2, v3;
		for (int i = 0; i < lineData.size(); i++)
		{

			//cout << lineData[i]<<" ";
			if (i == 2) {
				theta =  acosf(lineData[i])*180.0f/PI;
			}
			else if (i == 3) {
				v1 = lineData[i] / sin(theta);
			}
			else if (i == 4) {
				v2 = lineData[i] / sin(theta);
			}
			else if (i == 5) {
				v3 = lineData[i] / sin(theta);
			}
		}
		PPC camera1(120.0f, 320, 240);
		V3 v(v1, v2, v3);
		camera1.RotateAboutAxisThroughEye(v, theta);
		path1.AppendCamera(camera1,10);
		//cout << endl;
		//cout << theta << v1 << v2 << v3<<endl;

		data.push_back(lineData);
	}


		//system("pause");
	
}