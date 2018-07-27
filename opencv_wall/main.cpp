#include <opencv2/opencv.hpp>

int main()
{
	/*Webカメラ初期設定*/
	cv::VideoCapture cap(0);//デバイスのオープン(エラーが出る使う場合'1'かもしれない)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);//解像度 横(予備)
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);//解像度 縦(予備)
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	cap.set(CV_CAP_PROP_EXPOSURE, -13);//露出を下げてシャッター速度を上げる

	/*
	if (!cap.isOpened())//カメラデバイスが正常にオープンしたか確認
	{
		//読み込みに失敗したときの処理
		return -1;
	}
	*/
	cv::Mat pict = cv::imread("testPict.png");

	std::vector<std::vector<cv::Point>>contours;
	std::vector<cv::Vec4i> hierarchy;

	while (1) {
		cv::Mat color;//カラー画像
		cv::Mat gray;//グレースケール画像

		//pict >> color;//USBカメラからの画像を入力
		color = pict.clone();

		cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);//グレースケール化
		cv::Mat bin;//二値化画像
		cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		cv::Mat dst=bin.clone();

		// 2値画像中の輪郭を検出
		cv::findContours(dst,     //入力画像，8ビット，シングルチャンネル．0以外のピクセルは 1 、0のピクセルは0として扱う。処理結果として image を書き換えることに注意する.
			contours,             // 輪郭を点ベクトルとして取得する
			hierarchy,            // hiararchy ? オプション．画像のトポロジーに関する情報を含む出力ベクトル．
			CV_RETR_EXTERNAL,     // 輪郭抽出モード
			CV_CHAIN_APPROX_NONE  // 輪郭の近似手法
		);

		cv::Vec4f line;
		cv::fitLine(contours[0], line, CV_DIST_L2, 0, 0.01, 0.01);

		// 線分を描画
		cv::line(dst,
			cv::Point(line.val[2] - line.val[0] * 20.0, line.val[3] - line.val[1] * 20.0),     // １つ目の線分の座標
			cv::Point(line.val[2] + line.val[0] * 20.0, line.val[3] + line.val[1] * 20.0),     // ２つ目の線分の座標
			cv::Scalar(0, 0, 255),   // 色
			10,                       // 太さ
			cv::LINE_8
		);




		//for (int i = 0; i >= 0; i = hierarchy[i][0]) {}

		/*
		for (int i = 0; i >= 0; i = hierarchy[i][0])
		{
			// 2 次元あるいは 3 次元の点集合に直線をフィッティング
			cv::fitLine(contours[i], line, CV_DIST_L2, 0, 0.01, 0.01);

			// 線分を描画
			cv::line(bin,
				cv::Point(line.val[2] - line.val[0] * 20.0, line.val[3] - line.val[1] * 20.0),     // １つ目の線分の座標
				cv::Point(line.val[2] + line.val[0] * 20.0, line.val[3] + line.val[1] * 20.0),     // ２つ目の線分の座標
				cv::Scalar(0, 0, 255),   // 色
				2,                       // 太さ
				cv::LINE_8
			);
		}
		*/

		

		cv::imshow("window",dst);//画像を表示．

		int key = cv::waitKey(10);

	}
}

