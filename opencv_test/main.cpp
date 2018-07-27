#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>
#include <winsock2.h>
#include <Windows.h>

#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "wsock32.lib")

#pragma warning(disable:4996)

using namespace cv;

const double PI = 3.14159265359;//円周率

//スクリーン座標系の赤外線LEDの間隔[px]
const int PROJECTION_INTERVAL_ROWS = 800;

//スクリーン座標系の幅と高さの半分[pixel]
const int PROJECTION_COLS_HALF = 768 / 2;//縦の解像度の半分[px]
const int PROJECTION_ROWS_HALF = 1024 / 2;//横の解像度の半分[px]


										 //3点のLEDによって作られる二等辺三角形において
const int BASE_ANGLE = 120;//鈍角のなす角[deg]
const int THRESHOLD_ANGLE = 20;//鈍角のなす角の許容角度[deg]

							   //ベクトルの定義
struct Vector2D {
	double x;
	double y;
};

//ベクトルの大きさを計算する
double get_vector_length(Vector2D v) {
	return pow((v.x * v.x) + (v.y * v.y), 0.5);
}

//ベクトル内積
double dot_product(Vector2D vl, Vector2D vr) {
	return vl.x * vr.x + vl.y * vr.y;
}

//2つのベクトルABのなす角度θを求める
int angle_of_two_vector(Vector2D A, Vector2D B)
{
	//!!ベクトルの長さが0だと答えが出ないので注意!!

	//ベクトルAとBの長さを計算する
	double length_A = get_vector_length(A);
	double length_B = get_vector_length(B);

	//内積とベクトル長さを使ってcosθを求める
	double cos_sita = dot_product(A, B) / (length_A * length_B);

	//cosθからθを求める
	double sita = acos(cos_sita);

	//ラジアンでなく0～180の角度で出力
	sita = sita * 180.0 / PI;

	return static_cast<int>(sita);
}

//ベクトルの回転変換(反時計周りが正)
Vector2D rotate_of_vector(Vector2D Vec, double sita) {
	Vector2D DstVec;//変換後のベクトル
	double RadAngle = sita * PI / 180;

	DstVec.x = Vec.x*cos(RadAngle) - Vec.y*sin(RadAngle);
	DstVec.y = Vec.x*sin(RadAngle) + Vec.y*cos(RadAngle);

	return DstVec;
}

//2点間に線を引く
void draw_line(Vector2D v1, Vector2D v2, cv::Mat img) {
	cv::line(img, cv::Point(static_cast<int>(v1.x), static_cast<int>(v1.y)),
		cv::Point(static_cast<int>(v2.x), static_cast<int>(v2.y)), cv::Scalar(0, 255, 0), 3, 8);
}
//(0)



int main(int argc, const char* argv[])
{
	//送信用座標(0-255)
	int SendCoord_X = 0;
	int SendCoord_Y = 0;

	Vector2D ScreenCoord;//スクリーン座標[px]
	ScreenCoord.x = 0;
	ScreenCoord.y = 0;

	//step(1)Webカメラ初期設定
	cv::VideoCapture cap(0);//デバイスのオープン(USBWebカメラを使う場合'1'かもしれない)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap.set(CV_CAP_PROP_EXPOSURE, -13);

	if (!cap.isOpened())//カメラデバイスが正常にオープンしたか確認
	{
		//読み込みに失敗したときの処理
		return -1;
	}
	//(1)

	//(2)ソケット通信設定
	//IPアドレス、ポート番号、ソケット
	unsigned short port = 7000;
	SOCKET destSocket;

	//sockaddr_in 構造体
	struct sockaddr_in destSockAddr;

	//各種パラメータ 
	const char *toSendText = "test";//あとで消して?

							  //Windows 独自の設定
	WSADATA data;
	WSAStartup(MAKEWORD(2, 0), &data);

	//sockaddr_in 構造体のセット
	memset(&destSockAddr, 0, sizeof(destSockAddr));
	//destSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	destSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	destSockAddr.sin_port = htons(port);
	destSockAddr.sin_family = AF_INET;

	//ソケット生成
	destSocket = socket(AF_INET, SOCK_DGRAM, 0);
	//(2)

	while (1) {
		cv::Mat frame;//出力画像(チェック用)
		cv::Mat color;//カラー画像
		cv::Mat gray;//グレースケール画像

		cap >> color;//USBカメラからの画像を入力

		DWORD start = timeGetTime();//あとで消して//

									//二値化処理
		cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);//グレースケール化
		cv::Mat bin;//二値化画像
		cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		//ラベリング処理
		cv::Mat label;
		cv::Mat stats;
		cv::Mat centor;//重心
		int nLabel = cv::connectedComponentsWithStats(bin, label, stats, centor);

		//出力画像の生成(作業用)
		cv::Mat Dst(bin.size(), CV_8UC3);
		Dst = cv::Scalar(0, 0, 0);//黒で埋める(ゼロフィル)

								  // ラベリング結果の描画色を決定
		std::vector<cv::Vec3b> colors(nLabel);
		colors[0] = cv::Vec3b(0, 0, 0);
		for (int i = 1; i < nLabel; ++i) {
			colors[i] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
		}
		// ラベリング結果の描画
		for (int i = 0; i < Dst.rows; ++i) {
			int *lb = label.ptr<int>(i);
			cv::Vec3b *pix = Dst.ptr<cv::Vec3b>(i);
			for (int j = 0; j < Dst.cols; ++j) {
				pix[j] = colors[lb[j]];
			}
		}
		//std::cout << nLabel << std::endl;
		//赤外線LEDの点を3つ認識した場合(LED3点+背景をラベリング)
		if (nLabel == 4) {
			//それぞれのLEDの点の座標
			int x[3];
			int y[3];

			int topAngleNum;//鈍角のラベリング番号
			int A, B;//鈍角以外のラベリング番号

					 //赤外線LEDの座標を作業用変数に格納しに赤丸をつける
			for (int i = 1; i < nLabel; ++i) {
				double *param = centor.ptr<double>(i);
				x[i - 1] = static_cast<int>(param[0]);
				y[i - 1] = static_cast<int>(param[1]);
				cv::circle(Dst, cv::Point(x[i - 1], y[i - 1]), 3, cv::Scalar(0, 0, 255), -1);
			}

			for (int i = 1; i < nLabel; ++i) {
				//それぞれ3つの点を上部の頂点とみなして内積をとることにより検証する
				if (i == 1) {
					topAngleNum = 0;
					A = 1;
					B = 2;
				}
				else if (i == 2) {
					A = 0;
					topAngleNum = 1;
					B = 2;
				}
				else if (i == 3) {
					A = 0;
					B = 1;
					topAngleNum = 2;
				}

				Vector2D v1, v2;//鈍角から他の点へのベクトル
				v1.x = x[A] - x[topAngleNum];
				v1.y = y[A] - y[topAngleNum];
				v2.x = x[B] - x[topAngleNum];
				v2.y = y[B] - y[topAngleNum];

				double sita = angle_of_two_vector(v1, v2);//２つのベクトルのなす角を計算


				if (((BASE_ANGLE - THRESHOLD_ANGLE) < sita) && (sita < (BASE_ANGLE + THRESHOLD_ANGLE))) {//上部の頂点(鈍角)かどうか
																										 //std::cout << "int " << i << "Angle=" << sita << std::endl;
					break;
				}
			}

			//それぞれのLEDの点への位置ベクトル(第四象限)
			Vector2D Led1, Led2, Common, centorOfLowerLedCoord, lower2LedVector;
			Led1.x = x[A]; Led1.y = -1 * y[A];
			Led2.x = x[B]; Led2.y = -1 * y[B];
			Common.x = x[topAngleNum]; Common.y = -1 * y[topAngleNum];

			//位置ベクトルLed1 , Led2の重心を計算
			centorOfLowerLedCoord.x = (Led1.x + Led2.x) / 2;
			centorOfLowerLedCoord.y = (Led1.y + Led2.y) / 2;
			cv::circle(Dst, cv::Point(static_cast<int>(centorOfLowerLedCoord.x), static_cast<int>(-centorOfLowerLedCoord.y)), 3, cv::Scalar(0, 0, 255), -1);//重心の描画

																																							//ベクトルLed1 , Led2において y軸に近いほうから見た他方のベクトル
			if (Led1.x < Led2.x) {
				lower2LedVector.x = Led2.x - Led1.x;
				lower2LedVector.y = Led2.y - Led1.y;
			}
			else {
				lower2LedVector.x = Led1.x - Led2.x;
				lower2LedVector.y = Led1.y - Led2.y;
			}
			//lower2LedVectorの角度を計算
			double Angle = atan2(lower2LedVector.y, lower2LedVector.x);
			Angle = 180 * Angle / PI;

			//キャプチャ画面の中心(第四象限)
			Vector2D capCentorCoord;
			capCentorCoord.x = Dst.cols / 2;
			capCentorCoord.y = -1 * Dst.rows / 2;

			cv::circle(Dst, cv::Point(static_cast<int>(capCentorCoord.x), static_cast<int>(-capCentorCoord.y)), 4, cv::Scalar(255, 0, 0), -1);


			double dist2point = get_vector_length(lower2LedVector);//キャプチャ画像の下部二点の距離[px]
			double CoefExchangeCoord = PROJECTION_INTERVAL_ROWS / dist2point;//USBカメラとゲーム画像の座標へ変換する係数

																			 //LEDの点間の重心からキャプチャ画面中心へのベクトル
			Vector2D vecCapCentor;
			vecCapCentor.x = capCentorCoord.x - centorOfLowerLedCoord.x;
			vecCapCentor.y = capCentorCoord.y - centorOfLowerLedCoord.y;

			//回転行列でキャプチャ画面の座標系に変換
			Vector2D CapCoord;
			CapCoord = rotate_of_vector(vecCapCentor, -Angle);

			//出力用の座標(LEDの点からキャプチャ画像中心へのスクリーン座標)
			ScreenCoord.x = CapCoord.x*CoefExchangeCoord + PROJECTION_ROWS_HALF;//オフセット
			ScreenCoord.y = -1 * CapCoord.y*CoefExchangeCoord + PROJECTION_COLS_HALF;//オフセット

																					 //出力座標の範囲外の処理(X座標，Y座標)
			if (ScreenCoord.x < 0) {
				ScreenCoord.x = 0;
			}
			else if (ScreenCoord.x > PROJECTION_ROWS_HALF * 2) {
				ScreenCoord.x = PROJECTION_ROWS_HALF * 2;
			}

			if (ScreenCoord.y < 0) {
				ScreenCoord.y = 0;
			}
			else if (ScreenCoord.y > PROJECTION_COLS_HALF * 2) {
				ScreenCoord.y = PROJECTION_COLS_HALF * 2;
			}

			//std::cout << "CapCoord x:" << ScreenCoord.x << std::endl;
			//std::cout << "CapCoord y:" << ScreenCoord.y << std::endl;

			Vector2D TopLeft, TopRight, BottomLeft, BottomRight;
			double halfIntervalLedDistance = get_vector_length(lower2LedVector) / 2;//LEDの点A, Bの点間の距離を基準とする

																					//スクリーン範囲の座標を仮に代入

			TopLeft.x = BottomLeft.x = (-1)*halfIntervalLedDistance*static_cast<double>((PROJECTION_ROWS_HALF / (PROJECTION_INTERVAL_ROWS / 2.0f)));
			TopRight.x = BottomRight.x = halfIntervalLedDistance * static_cast<double>((PROJECTION_ROWS_HALF / (PROJECTION_INTERVAL_ROWS / 2.0f)));
			TopLeft.y = TopRight.y = halfIntervalLedDistance * static_cast<double>(((3 / 4.0f)*PROJECTION_ROWS_HALF / (PROJECTION_INTERVAL_ROWS / 2.0f)));
			BottomLeft.y = BottomRight.y = (-1)*halfIntervalLedDistance*static_cast<double>(((3 / 4.0f)*PROJECTION_ROWS_HALF / (PROJECTION_INTERVAL_ROWS / 2.0f)));

			//仮想スクリーン範囲を回転する
			TopLeft = rotate_of_vector(TopLeft, Angle);
			TopRight = rotate_of_vector(TopRight, Angle);
			BottomLeft = rotate_of_vector(BottomLeft, Angle);
			BottomRight = rotate_of_vector(BottomRight, Angle);

			//オフセット
			TopLeft.x += centorOfLowerLedCoord.x;
			TopRight.x += centorOfLowerLedCoord.x;
			BottomLeft.x += centorOfLowerLedCoord.x;
			BottomRight.x += centorOfLowerLedCoord.x;
			TopLeft.y += centorOfLowerLedCoord.y;
			TopRight.y += centorOfLowerLedCoord.y;
			BottomLeft.y += centorOfLowerLedCoord.y;
			BottomRight.y += centorOfLowerLedCoord.y;

			//第四象限から第一象限へ変更
			TopLeft.y = -TopLeft.y;
			TopRight.y = -TopRight.y;
			BottomLeft.y = -BottomLeft.y;
			BottomRight.y = -BottomRight.y;

			//仮想スクリーン範囲描画
			draw_line(TopLeft, TopRight, Dst);
			draw_line(BottomLeft, BottomRight, Dst);
			draw_line(TopLeft, BottomLeft, Dst);
			draw_line(TopRight, BottomRight, Dst);

		}
		//バイト型に収まるようにマッピング
		SendCoord_X = static_cast<int>(ScreenCoord.x / 4);//0-1280 -> 0-256
		SendCoord_Y = static_cast<int>(ScreenCoord.y / 3);//0-768 -> 0-256
		if (SendCoord_X == 256)SendCoord_X = 255;//オーバーフロー防止
		if (SendCoord_Y == 256)SendCoord_Y = 255;//オーバーフロー防止
												 //std::cout << "CapCoord x:" << SendCoord_X << std::endl;
												 //std::cout << "CapCoord y:" << SendCoord_Y << std::endl;

		char SendVal[3];
		SendVal[0] = SendCoord_X;
		SendVal[1] = SendCoord_Y;
		SendVal[2] = '\0';

		//Unityに座標を送信
		sendto(destSocket, SendVal, 3, 0, (struct sockaddr *)&destSockAddr, sizeof(destSockAddr));

		frame = Dst;
		cv::imshow("window", frame);//画像を表示．

		DWORD end = timeGetTime();//あとで消して//
		std::cout << (double)(end - start) << std::endl;

		int key = cv::waitKey(10);

	}

	cv::destroyAllWindows();

	// Windows 独自の設定 
	closesocket(destSocket);
	WSACleanup();
	return 0;
}
