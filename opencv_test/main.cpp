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

const double PI = 3.14159265359;//�~����

//�X�N���[�����W�n�̐ԊO��LED�̊Ԋu[px]
const int PROJECTION_INTERVAL_ROWS = 800;

//�X�N���[�����W�n�̕��ƍ����̔���[pixel]
const int PROJECTION_COLS_HALF = 768 / 2;//�c�̉𑜓x�̔���[px]
const int PROJECTION_ROWS_HALF = 1024 / 2;//���̉𑜓x�̔���[px]


										 //3�_��LED�ɂ���č����񓙕ӎO�p�`�ɂ�����
const int BASE_ANGLE = 120;//�݊p�̂Ȃ��p[deg]
const int THRESHOLD_ANGLE = 20;//�݊p�̂Ȃ��p�̋��e�p�x[deg]

							   //�x�N�g���̒�`
struct Vector2D {
	double x;
	double y;
};

//�x�N�g���̑傫�����v�Z����
double get_vector_length(Vector2D v) {
	return pow((v.x * v.x) + (v.y * v.y), 0.5);
}

//�x�N�g������
double dot_product(Vector2D vl, Vector2D vr) {
	return vl.x * vr.x + vl.y * vr.y;
}

//2�̃x�N�g��AB�̂Ȃ��p�x�Ƃ����߂�
int angle_of_two_vector(Vector2D A, Vector2D B)
{
	//!!�x�N�g���̒�����0���Ɠ������o�Ȃ��̂Œ���!!

	//�x�N�g��A��B�̒������v�Z����
	double length_A = get_vector_length(A);
	double length_B = get_vector_length(B);

	//���ςƃx�N�g���������g����cos�Ƃ����߂�
	double cos_sita = dot_product(A, B) / (length_A * length_B);

	//cos�Ƃ���Ƃ����߂�
	double sita = acos(cos_sita);

	//���W�A���łȂ�0�`180�̊p�x�ŏo��
	sita = sita * 180.0 / PI;

	return static_cast<int>(sita);
}

//�x�N�g���̉�]�ϊ�(�����v���肪��)
Vector2D rotate_of_vector(Vector2D Vec, double sita) {
	Vector2D DstVec;//�ϊ���̃x�N�g��
	double RadAngle = sita * PI / 180;

	DstVec.x = Vec.x*cos(RadAngle) - Vec.y*sin(RadAngle);
	DstVec.y = Vec.x*sin(RadAngle) + Vec.y*cos(RadAngle);

	return DstVec;
}

//2�_�Ԃɐ�������
void draw_line(Vector2D v1, Vector2D v2, cv::Mat img) {
	cv::line(img, cv::Point(static_cast<int>(v1.x), static_cast<int>(v1.y)),
		cv::Point(static_cast<int>(v2.x), static_cast<int>(v2.y)), cv::Scalar(0, 255, 0), 3, 8);
}
//(0)



int main(int argc, const char* argv[])
{
	//���M�p���W(0-255)
	int SendCoord_X = 0;
	int SendCoord_Y = 0;

	Vector2D ScreenCoord;//�X�N���[�����W[px]
	ScreenCoord.x = 0;
	ScreenCoord.y = 0;

	//step(1)Web�J���������ݒ�
	cv::VideoCapture cap(0);//�f�o�C�X�̃I�[�v��(USBWeb�J�������g���ꍇ'1'��������Ȃ�)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap.set(CV_CAP_PROP_EXPOSURE, -13);

	if (!cap.isOpened())//�J�����f�o�C�X������ɃI�[�v���������m�F
	{
		//�ǂݍ��݂Ɏ��s�����Ƃ��̏���
		return -1;
	}
	//(1)

	//(2)�\�P�b�g�ʐM�ݒ�
	//IP�A�h���X�A�|�[�g�ԍ��A�\�P�b�g
	unsigned short port = 7000;
	SOCKET destSocket;

	//sockaddr_in �\����
	struct sockaddr_in destSockAddr;

	//�e��p�����[�^ 
	const char *toSendText = "test";//���Ƃŏ�����?

							  //Windows �Ǝ��̐ݒ�
	WSADATA data;
	WSAStartup(MAKEWORD(2, 0), &data);

	//sockaddr_in �\���̂̃Z�b�g
	memset(&destSockAddr, 0, sizeof(destSockAddr));
	//destSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	destSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	destSockAddr.sin_port = htons(port);
	destSockAddr.sin_family = AF_INET;

	//�\�P�b�g����
	destSocket = socket(AF_INET, SOCK_DGRAM, 0);
	//(2)

	while (1) {
		cv::Mat frame;//�o�͉摜(�`�F�b�N�p)
		cv::Mat color;//�J���[�摜
		cv::Mat gray;//�O���[�X�P�[���摜

		cap >> color;//USB�J��������̉摜�����

		DWORD start = timeGetTime();//���Ƃŏ�����//

									//��l������
		cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);//�O���[�X�P�[����
		cv::Mat bin;//��l���摜
		cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		//���x�����O����
		cv::Mat label;
		cv::Mat stats;
		cv::Mat centor;//�d�S
		int nLabel = cv::connectedComponentsWithStats(bin, label, stats, centor);

		//�o�͉摜�̐���(��Ɨp)
		cv::Mat Dst(bin.size(), CV_8UC3);
		Dst = cv::Scalar(0, 0, 0);//���Ŗ��߂�(�[���t�B��)

								  // ���x�����O���ʂ̕`��F������
		std::vector<cv::Vec3b> colors(nLabel);
		colors[0] = cv::Vec3b(0, 0, 0);
		for (int i = 1; i < nLabel; ++i) {
			colors[i] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
		}
		// ���x�����O���ʂ̕`��
		for (int i = 0; i < Dst.rows; ++i) {
			int *lb = label.ptr<int>(i);
			cv::Vec3b *pix = Dst.ptr<cv::Vec3b>(i);
			for (int j = 0; j < Dst.cols; ++j) {
				pix[j] = colors[lb[j]];
			}
		}
		//std::cout << nLabel << std::endl;
		//�ԊO��LED�̓_��3�F�������ꍇ(LED3�_+�w�i�����x�����O)
		if (nLabel == 4) {
			//���ꂼ���LED�̓_�̍��W
			int x[3];
			int y[3];

			int topAngleNum;//�݊p�̃��x�����O�ԍ�
			int A, B;//�݊p�ȊO�̃��x�����O�ԍ�

					 //�ԊO��LED�̍��W����Ɨp�ϐ��Ɋi�[���ɐԊۂ�����
			for (int i = 1; i < nLabel; ++i) {
				double *param = centor.ptr<double>(i);
				x[i - 1] = static_cast<int>(param[0]);
				y[i - 1] = static_cast<int>(param[1]);
				cv::circle(Dst, cv::Point(x[i - 1], y[i - 1]), 3, cv::Scalar(0, 0, 255), -1);
			}

			for (int i = 1; i < nLabel; ++i) {
				//���ꂼ��3�̓_���㕔�̒��_�Ƃ݂Ȃ��ē��ς��Ƃ邱�Ƃɂ�茟�؂���
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

				Vector2D v1, v2;//�݊p���瑼�̓_�ւ̃x�N�g��
				v1.x = x[A] - x[topAngleNum];
				v1.y = y[A] - y[topAngleNum];
				v2.x = x[B] - x[topAngleNum];
				v2.y = y[B] - y[topAngleNum];

				double sita = angle_of_two_vector(v1, v2);//�Q�̃x�N�g���̂Ȃ��p���v�Z


				if (((BASE_ANGLE - THRESHOLD_ANGLE) < sita) && (sita < (BASE_ANGLE + THRESHOLD_ANGLE))) {//�㕔�̒��_(�݊p)���ǂ���
																										 //std::cout << "int " << i << "Angle=" << sita << std::endl;
					break;
				}
			}

			//���ꂼ���LED�̓_�ւ̈ʒu�x�N�g��(��l�ی�)
			Vector2D Led1, Led2, Common, centorOfLowerLedCoord, lower2LedVector;
			Led1.x = x[A]; Led1.y = -1 * y[A];
			Led2.x = x[B]; Led2.y = -1 * y[B];
			Common.x = x[topAngleNum]; Common.y = -1 * y[topAngleNum];

			//�ʒu�x�N�g��Led1 , Led2�̏d�S���v�Z
			centorOfLowerLedCoord.x = (Led1.x + Led2.x) / 2;
			centorOfLowerLedCoord.y = (Led1.y + Led2.y) / 2;
			cv::circle(Dst, cv::Point(static_cast<int>(centorOfLowerLedCoord.x), static_cast<int>(-centorOfLowerLedCoord.y)), 3, cv::Scalar(0, 0, 255), -1);//�d�S�̕`��

																																							//�x�N�g��Led1 , Led2�ɂ����� y���ɋ߂��ق����猩�������̃x�N�g��
			if (Led1.x < Led2.x) {
				lower2LedVector.x = Led2.x - Led1.x;
				lower2LedVector.y = Led2.y - Led1.y;
			}
			else {
				lower2LedVector.x = Led1.x - Led2.x;
				lower2LedVector.y = Led1.y - Led2.y;
			}
			//lower2LedVector�̊p�x���v�Z
			double Angle = atan2(lower2LedVector.y, lower2LedVector.x);
			Angle = 180 * Angle / PI;

			//�L���v�`����ʂ̒��S(��l�ی�)
			Vector2D capCentorCoord;
			capCentorCoord.x = Dst.cols / 2;
			capCentorCoord.y = -1 * Dst.rows / 2;

			cv::circle(Dst, cv::Point(static_cast<int>(capCentorCoord.x), static_cast<int>(-capCentorCoord.y)), 4, cv::Scalar(255, 0, 0), -1);


			double dist2point = get_vector_length(lower2LedVector);//�L���v�`���摜�̉�����_�̋���[px]
			double CoefExchangeCoord = PROJECTION_INTERVAL_ROWS / dist2point;//USB�J�����ƃQ�[���摜�̍��W�֕ϊ�����W��

																			 //LED�̓_�Ԃ̏d�S����L���v�`����ʒ��S�ւ̃x�N�g��
			Vector2D vecCapCentor;
			vecCapCentor.x = capCentorCoord.x - centorOfLowerLedCoord.x;
			vecCapCentor.y = capCentorCoord.y - centorOfLowerLedCoord.y;

			//��]�s��ŃL���v�`����ʂ̍��W�n�ɕϊ�
			Vector2D CapCoord;
			CapCoord = rotate_of_vector(vecCapCentor, -Angle);

			//�o�͗p�̍��W(LED�̓_����L���v�`���摜���S�ւ̃X�N���[�����W)
			ScreenCoord.x = CapCoord.x*CoefExchangeCoord + PROJECTION_ROWS_HALF;//�I�t�Z�b�g
			ScreenCoord.y = -1 * CapCoord.y*CoefExchangeCoord + PROJECTION_COLS_HALF;//�I�t�Z�b�g

																					 //�o�͍��W�͈̔͊O�̏���(X���W�CY���W)
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
			double halfIntervalLedDistance = get_vector_length(lower2LedVector) / 2;//LED�̓_A, B�̓_�Ԃ̋�������Ƃ���

																					//�X�N���[���͈͂̍��W�����ɑ��

			TopLeft.x = BottomLeft.x = (-1)*halfIntervalLedDistance*static_cast<double>((PROJECTION_ROWS_HALF / (PROJECTION_INTERVAL_ROWS / 2.0f)));
			TopRight.x = BottomRight.x = halfIntervalLedDistance * static_cast<double>((PROJECTION_ROWS_HALF / (PROJECTION_INTERVAL_ROWS / 2.0f)));
			TopLeft.y = TopRight.y = halfIntervalLedDistance * static_cast<double>(((3 / 4.0f)*PROJECTION_ROWS_HALF / (PROJECTION_INTERVAL_ROWS / 2.0f)));
			BottomLeft.y = BottomRight.y = (-1)*halfIntervalLedDistance*static_cast<double>(((3 / 4.0f)*PROJECTION_ROWS_HALF / (PROJECTION_INTERVAL_ROWS / 2.0f)));

			//���z�X�N���[���͈͂���]����
			TopLeft = rotate_of_vector(TopLeft, Angle);
			TopRight = rotate_of_vector(TopRight, Angle);
			BottomLeft = rotate_of_vector(BottomLeft, Angle);
			BottomRight = rotate_of_vector(BottomRight, Angle);

			//�I�t�Z�b�g
			TopLeft.x += centorOfLowerLedCoord.x;
			TopRight.x += centorOfLowerLedCoord.x;
			BottomLeft.x += centorOfLowerLedCoord.x;
			BottomRight.x += centorOfLowerLedCoord.x;
			TopLeft.y += centorOfLowerLedCoord.y;
			TopRight.y += centorOfLowerLedCoord.y;
			BottomLeft.y += centorOfLowerLedCoord.y;
			BottomRight.y += centorOfLowerLedCoord.y;

			//��l�ی�������ی��֕ύX
			TopLeft.y = -TopLeft.y;
			TopRight.y = -TopRight.y;
			BottomLeft.y = -BottomLeft.y;
			BottomRight.y = -BottomRight.y;

			//���z�X�N���[���͈͕`��
			draw_line(TopLeft, TopRight, Dst);
			draw_line(BottomLeft, BottomRight, Dst);
			draw_line(TopLeft, BottomLeft, Dst);
			draw_line(TopRight, BottomRight, Dst);

		}
		//�o�C�g�^�Ɏ��܂�悤�Ƀ}�b�s���O
		SendCoord_X = static_cast<int>(ScreenCoord.x / 4);//0-1280 -> 0-256
		SendCoord_Y = static_cast<int>(ScreenCoord.y / 3);//0-768 -> 0-256
		if (SendCoord_X == 256)SendCoord_X = 255;//�I�[�o�[�t���[�h�~
		if (SendCoord_Y == 256)SendCoord_Y = 255;//�I�[�o�[�t���[�h�~
												 //std::cout << "CapCoord x:" << SendCoord_X << std::endl;
												 //std::cout << "CapCoord y:" << SendCoord_Y << std::endl;

		char SendVal[3];
		SendVal[0] = SendCoord_X;
		SendVal[1] = SendCoord_Y;
		SendVal[2] = '\0';

		//Unity�ɍ��W�𑗐M
		sendto(destSocket, SendVal, 3, 0, (struct sockaddr *)&destSockAddr, sizeof(destSockAddr));

		frame = Dst;
		cv::imshow("window", frame);//�摜��\���D

		DWORD end = timeGetTime();//���Ƃŏ�����//
		std::cout << (double)(end - start) << std::endl;

		int key = cv::waitKey(10);

	}

	cv::destroyAllWindows();

	// Windows �Ǝ��̐ݒ� 
	closesocket(destSocket);
	WSACleanup();
	return 0;
}
