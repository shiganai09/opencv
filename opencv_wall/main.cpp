#include <opencv2/opencv.hpp>

int main()
{
	/*Webカメラ初期設定*/
	cv::VideoCapture cap(0);//デバイスのオープン(エラーが出る使う場合'1'かもしれない)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);//解像度 横(予備)
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);//解像度 縦(予備)
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap.set(CV_CAP_PROP_EXPOSURE, -13);//露出を下げてシャッター速度を上げる

	if (!cap.isOpened())//カメラデバイスが正常にオープンしたか確認
	{
		//読み込みに失敗したときの処理
		return -1;
	}

	while (1) {
		cv::Mat color;//カラー画像

		cap >> color;//USBカメラからの画像を入力


		cv::imshow("window", color);//画像を表示．

		int key = cv::waitKey(10);

	}
}

