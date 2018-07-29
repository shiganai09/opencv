#include <opencv2/opencv.hpp>

int main()
{
	/*Webカメラ初期設定*/
	cv::VideoCapture cap(0);//デバイスのオープン(エラーが出る使う場合'1'かもしれない)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	cap.set(CV_CAP_PROP_EXPOSURE, -13); //露出を下げてシャッター速度を上げる

	/*今回は使わない
	if (!cap.isOpened())//カメラデバイスが正常にオープンしたか確認
	{
		//読み込みに失敗したときの処理
		return -1;
	}
	*/

	cv::Mat pict = cv::imread("testPict.png"); //画像の読み込み

	while (1) {
		cv::Mat color;//カラー画像Mat
		cv::Mat gray;//グレースケール画像Mat

		//pict >> color;//USBカメラからの画像を入力
		color = pict.clone(); //カメラからの入力の代替

		cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);//グレースケール化
		cv::Mat bin;//二値化画像Mat
		cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		cv::Mat dst=bin.clone(); //出力用の画像Matを用意しておく

		//チャンネル数確認用（いらない）
		std::cout << "color ch:" << color.channels() << std::endl;
		std::cout << "gray ch:" << gray.channels() << std::endl;
		std::cout << "bin ch:" << bin.channels() << std::endl;
		std::cout << "dst ch:" << dst.channels() << std::endl;


		std::vector<std::vector<cv::Point>>contours;
		std::vector<cv::Vec4i> hierarchy;

		// 2値画像中の輪郭を検出
		cv::findContours(bin,     //入力画像，8ビット，シングルチャンネル．0以外のピクセルは 1 、0のピクセルは0として扱う。処理結果として image を書き換えることに注意する.
			contours,             // 輪郭を点ベクトルとして取得する
			hierarchy,            // hiararchy ? オプション．画像のトポロジーに関する情報を含む出力ベクトル．
			CV_RETR_EXTERNAL,     // 輪郭抽出モード
			CV_CHAIN_APPROX_NONE  // 輪郭の近似手法
		);


		cv::Vec4f line; //フィッティングした線[vx,vy,x0,y0]{vx,vy(x,y方向の方向ベクトル),x0,y0(どこかわからないが線上の点)}

		cv::fitLine(contours[0], line, CV_DIST_L2, 0, 0.01, 0.01);

		int y0 = line.val[3] - line.val[1] * line.val[2]; //y切片を求める(y0-m*x0) ：mは一次関数の傾き

		std::cout << (int)line.val[1] << std::endl; //一次関数の傾き
		std::cout << y0 << std::endl; //y切片(x=0でのyの値(オフセット))

		/*
		// 線分を描画(x=0 -> x=imageWidth)
		cv::line(dst,
			cv::Point(0, y0),
			cv::Point(dst.cols, line.val[1]+y0),
			cv::Scalar(255),
			3,
			cv::LINE_8
		);
		*/
		
		//手によって影になった位置を検出するために，ライン上を移動する小さな正方形を考える．
		//その正方形に含まれる画素値(モノクロ)の総和を求めていく（小さければそこは影）
		const int DIV = 2; //分割の幅
		const int  MAX_COUNT= dst.cols /DIV; //端まで行くために必要な数

		std::vector<int> fallPoint;
		std::vector<int> raisePoint;
		bool falling = false;

		for (int i = 1; i < MAX_COUNT; i++) {
			int sum = 0;
			for (int j = -DIV / 2; j < DIV / 2; j++) {
				for (int k = -DIV / 2; k < DIV / 2; k++) {
					sum += (unsigned int)dst.at<unsigned char>((DIV*i)*line.val[1] + y0 + k,DIV*i+j ); //(y,x)のピクセルの値を取得する
					dst.at<unsigned char>((DIV*i)*line.val[1] + y0 + k, DIV*i + j)=127; //計算を行った部分をグレーで塗りつぶす
				}
			}
			if (!falling&&sum == 0) { fallPoint.emplace_back(i); falling = true; }
			if (falling && sum > 0) { raisePoint.emplace_back(i); falling = false; }
			//std::cout << "Count=" << i << ": sum=" << sum << std::endl;
		}

		std::vector<int> touchCentor;
		if ((int)fallPoint.size() != (int)raisePoint.size()) { std::cout << "error(don't match fallPoint and raisePoint )" << std::endl; }
		else if(fallPoint.size() >1){ std::cout << "error(too many point)" << std::endl; }
		else{
			for (int i = 0; i < fallPoint.size(); i++){
				touchCentor.emplace_back((raisePoint[i] + fallPoint[i])/2);
				std::cout << "CentorPoint = " << touchCentor[i] << std::endl;
			}
		}
		std::cout << fallPoint.size() << std::endl;
		std::cout << raisePoint.size() << std::endl;


		//cv::rectangle(dst, cv::Point(10, 10), cv::Point(1500, 20),cv::Scalar(255), -1); //長方形を描く		

		cv::imshow("window",dst);//画像を表示．

		int key = cv::waitKey(10);

	}
}

