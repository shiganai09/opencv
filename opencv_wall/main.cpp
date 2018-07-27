#include <opencv2/opencv.hpp>

int main()
{
	/*Webカメラ初期設定*/
	cv::VideoCapture cap(0);//デバイスのオープン(エラーが出る使う場合'1'かもしれない)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);//解像度 横(予備)
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);//解像度 縦(予備)
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap.set(CV_CAP_PROP_EXPOSURE, -13);//露出を下げてシャッター速度を上げる

	if (!cap.isOpened())//カメラデバイスが正常にオープンしたか確認
	{
		//読み込みに失敗したときの処理
		return -1;
	}

	while (1) {
		cv::Mat color;//カラー画像
		cv::Mat gray;//グレースケール画像

		cap >> color;//USBカメラからの画像を入力

		cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);//グレースケール化
		cv::Mat bin;//二値化画像
		cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		

		cv::imshow("window", bin);//画像を表示．

		int key = cv::waitKey(10);

	}
}

