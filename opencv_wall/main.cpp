#include <opencv2/opencv.hpp>

int main()
{
	/*Webカメラ初期設定*/
	cv::VideoCapture cap(0);//デバイスのオープン(エラーが出る使う場合'1'かもしれない)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280); //解像度 横(予備)
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960); //解像度 縦(予備)
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	cap.set(CV_CAP_PROP_EXPOSURE, -13); //露出を下げてシャッター速度を上げる

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

		std::cout << "color ch:" << color.channels() << std::endl;
		std::cout << "gray ch:" << gray.channels() << std::endl;
		std::cout << "bin ch:" << bin.channels() << std::endl;
		std::cout << "dst ch:" << dst.channels() << std::endl;

		// 2値画像中の輪郭を検出
		cv::findContours(dst,     //入力画像，8ビット，シングルチャンネル．0以外のピクセルは 1 、0のピクセルは0として扱う。処理結果として image を書き換えることに注意する.
			contours,             // 輪郭を点ベクトルとして取得する
			hierarchy,            // hiararchy ? オプション．画像のトポロジーに関する情報を含む出力ベクトル．
			CV_RETR_EXTERNAL,     // 輪郭抽出モード
			CV_CHAIN_APPROX_NONE  // 輪郭の近似手法
		);



		cv::Vec4f line;
		cv::fitLine(contours[0], line, CV_DIST_L2, 0, 0.01, 0.01);

		int y0 = line.val[3] - line.val[1] * line.val[2];

		std::cout << (int)line.val[1] << std::endl;
		std::cout << y0 << std::endl;

		// 線分を描画
		cv::line(dst,
			cv::Point(0, y0),
			cv::Point(dst.cols, line.val[1]+y0),
			cv::Scalar(255),
			3,
			cv::LINE_8
		);
		
		const int div = 5;
		const int  count= dst.cols /div;

		for (int i = 1; i < count; i++) {
			int sum = 0;
			for (int j = -1 * div / 2 - 1; j < div / 2 - 1; j++) {
				for (int k = -1 * div / 2 - 1; k < div / 2 - 1; k++) {
					sum += (unsigned int)dst.at<unsigned char>((div*i)*line.val[1] + y0 + k,div*i+j ); //(y,x)のピクセルの値を取得する
					dst.at<unsigned char>((div*i)*line.val[1] + y0 + k, div*i + j)=127;
				}
			}
			std::cout << "count=" << i << ": sum=" << sum << std::endl;
		}

		//cv::rectangle(dst, cv::Point(10, 10), cv::Point(1500, 20),cv::Scalar(255), -1); //長方形を描く

		std::cout << (unsigned int)dst.at<unsigned char>(15, 15) << std::endl;		

		cv::imshow("window",dst);//画像を表示．

		int key = cv::waitKey(10);

	}
}

