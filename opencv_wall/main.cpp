#include <opencv2/opencv.hpp>

int main()
{
	/*Web�J���������ݒ�*/
	cv::VideoCapture cap(0);//�f�o�C�X�̃I�[�v��(�G���[���o��g���ꍇ'1'��������Ȃ�)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);//�𑜓x ��(�\��)
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);//�𑜓x �c(�\��)
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	cap.set(CV_CAP_PROP_EXPOSURE, -13);//�I�o�������ăV���b�^�[���x���グ��

	/*
	if (!cap.isOpened())//�J�����f�o�C�X������ɃI�[�v���������m�F
	{
		//�ǂݍ��݂Ɏ��s�����Ƃ��̏���
		return -1;
	}
	*/
	cv::Mat pict = cv::imread("testPict.png");

	std::vector<std::vector<cv::Point>>contours;
	std::vector<cv::Vec4i> hierarchy;

	while (1) {
		cv::Mat color;//�J���[�摜
		cv::Mat gray;//�O���[�X�P�[���摜

		//pict >> color;//USB�J��������̉摜�����
		color = pict.clone();

		cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);//�O���[�X�P�[����
		cv::Mat bin;//��l���摜
		cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		cv::Mat dst=bin.clone();

		// 2�l�摜���̗֊s�����o
		cv::findContours(dst,     //���͉摜�C8�r�b�g�C�V���O���`�����l���D0�ȊO�̃s�N�Z���� 1 �A0�̃s�N�Z����0�Ƃ��Ĉ����B�������ʂƂ��� image �����������邱�Ƃɒ��ӂ���.
			contours,             // �֊s��_�x�N�g���Ƃ��Ď擾����
			hierarchy,            // hiararchy ? �I�v�V�����D�摜�̃g�|���W�[�Ɋւ�������܂ޏo�̓x�N�g���D
			CV_RETR_EXTERNAL,     // �֊s���o���[�h
			CV_CHAIN_APPROX_NONE  // �֊s�̋ߎ���@
		);

		cv::Vec4f line;
		cv::fitLine(contours[0], line, CV_DIST_L2, 0, 0.01, 0.01);

		// ������`��
		cv::line(dst,
			cv::Point(line.val[2] - line.val[0] * 20.0, line.val[3] - line.val[1] * 20.0),     // �P�ڂ̐����̍��W
			cv::Point(line.val[2] + line.val[0] * 20.0, line.val[3] + line.val[1] * 20.0),     // �Q�ڂ̐����̍��W
			cv::Scalar(0, 0, 255),   // �F
			10,                       // ����
			cv::LINE_8
		);




		//for (int i = 0; i >= 0; i = hierarchy[i][0]) {}

		/*
		for (int i = 0; i >= 0; i = hierarchy[i][0])
		{
			// 2 �������邢�� 3 �����̓_�W���ɒ������t�B�b�e�B���O
			cv::fitLine(contours[i], line, CV_DIST_L2, 0, 0.01, 0.01);

			// ������`��
			cv::line(bin,
				cv::Point(line.val[2] - line.val[0] * 20.0, line.val[3] - line.val[1] * 20.0),     // �P�ڂ̐����̍��W
				cv::Point(line.val[2] + line.val[0] * 20.0, line.val[3] + line.val[1] * 20.0),     // �Q�ڂ̐����̍��W
				cv::Scalar(0, 0, 255),   // �F
				2,                       // ����
				cv::LINE_8
			);
		}
		*/

		

		cv::imshow("window",dst);//�摜��\���D

		int key = cv::waitKey(10);

	}
}

