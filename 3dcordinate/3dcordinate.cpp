#include <string>
#include <opencv2/opencv.hpp>
#include <vector>



#define PAT_ROW    (7)          /* パターンの行数 */
#define PAT_COL    (10)         /* パターンの列数 */
#define PAT_SIZE   (PAT_ROW*PAT_COL)
#define ALL_POINTS (IMAGE_NUM*PAT_SIZE)
#define CHESS_SIZE (20.0)       /* パターン1マスの1辺サイズ[mm] */

using namespace std;


int main() {

	//（１）事前にキャリブレーションしたカメラの内部パラメータと歪み係数を取得
	cv::Mat intrinsic;
	cv::Mat distCoeffs;
	const std::string fileName = "../Camera_Calibration/camera.xml";

	cv::FileStorage fs(fileName, cv::FileStorage::READ);
	if (!fs.isOpened()) {
		std::cout << "File cannot be opened.\n";
		return -1;
	}
	fs["intrinsic"] >> intrinsic;
	fs["distCoeffs"] >> distCoeffs;
	fs.release();

	cout << "get parameter" << endl;

	// (２)3次元空間座標の設定

	std::vector<cv::Point3f> object;
	for (int j = 0; j < PAT_ROW; j++)
	{
		for (int k = 0; k < PAT_COL; k++)
		{
			cv::Point3f p(
				j * CHESS_SIZE,
				k * CHESS_SIZE,
				0.0);
			object.push_back(p);
		}
	}
	std::vector<cv::Point3f> object3d;
	for (int j = 0; j < PAT_ROW; j++)
	{
		for (int k = 0; k < PAT_COL; k++)
		{
			cv::Point3f p(
				j * CHESS_SIZE,
				k * CHESS_SIZE,
				3 * CHESS_SIZE);
			object3d.push_back(p);
		}
	}

	cout << "get 3d cordinate" << endl;

	
	
	cv::VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cerr << "Unable to connect to camera" << endl;
		return 1;
	}
	else {
		cout << "camera open" << endl;
	}
	cv::Mat src;
	cv::Size pattern_size = cv::Size2i(PAT_COL, PAT_ROW);
	int key;

	do {
		cap.read(src);
		
		cv::flip(src, src, 1);
		
		
		//（３）2次元空間座標の設定
		vector<cv::Point2f> corners;
		auto found = cv::findChessboardCorners(src, pattern_size, corners);
		if (found)
		{
			cout << "get 2d cordinate" << endl;
			//（４）PnP問題を解く
			cv::Mat rvec, tvec;
			cv::solvePnP(object, corners, intrinsic, distCoeffs, rvec, tvec);
			cout << "get rvec & tvec" << endl;

			//（５）3次元点の投影

			vector<cv::Point2f> imagePoints, imagePoints3d;
			cv::projectPoints(object, rvec, tvec, intrinsic, distCoeffs, imagePoints);
			cv::projectPoints(object3d, rvec, tvec, intrinsic, distCoeffs, imagePoints3d);
			cv::line(src, imagePoints[35], imagePoints[38], cv::Scalar(255, 0, 0), 4, 16);
			cv::line(src, imagePoints[35], imagePoints[35 + 3 * PAT_COL], cv::Scalar(0, 255, 0), 4, 16);
			cv::line(src, imagePoints[35], imagePoints3d[35], cv::Scalar(0, 0, 255), 4, 16);
			cv::line(src, imagePoints[0], imagePoints[3], cv::Scalar(255, 0, 0), 4, 16);
			cv::line(src, imagePoints[0], imagePoints[0 + 3 * PAT_COL], cv::Scalar(0, 255, 0), 4, 16);
			cv::line(src, imagePoints[0], imagePoints3d[0], cv::Scalar(0, 0, 255), 4, 16);
		}
		else
		{
			cout << "can't get 2d cordinate" << endl;
			
		}
		
		cv::imshow("draw", src);

		const int key = cv::waitKey(1);
		if(key == 'q'/*113*/)//qボタンが押されたとき
		{
		break;//whileループから抜ける．
		}
		
	} while (1);
	
	return 0;
}