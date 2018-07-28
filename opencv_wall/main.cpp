#include <opencv2/opencv.hpp>

int main()
{
	/*Web�J���������ݒ�*/
	cv::VideoCapture cap(0);//�f�o�C�X�̃I�[�v��(�G���[���o��g���ꍇ'1'��������Ȃ�)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280); //�𑜓x ��(�\��)
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960); //�𑜓x �c(�\��)
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	cap.set(CV_CAP_PROP_EXPOSURE, -13); //�I�o�������ăV���b�^�[���x���グ��

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

		std::cout << "color ch:" << color.channels() << std::endl;
		std::cout << "gray ch:" << gray.channels() << std::endl;
		std::cout << "bin ch:" << bin.channels() << std::endl;
		std::cout << "dst ch:" << dst.channels() << std::endl;

		// 2�l�摜���̗֊s�����o
		cv::findContours(dst,     //���͉摜�C8�r�b�g�C�V���O���`�����l���D0�ȊO�̃s�N�Z���� 1 �A0�̃s�N�Z����0�Ƃ��Ĉ����B�������ʂƂ��� image �����������邱�Ƃɒ��ӂ���.
			contours,             // �֊s��_�x�N�g���Ƃ��Ď擾����
			hierarchy,            // hiararchy ? �I�v�V�����D�摜�̃g�|���W�[�Ɋւ�������܂ޏo�̓x�N�g���D
			CV_RETR_EXTERNAL,     // �֊s���o���[�h
			CV_CHAIN_APPROX_NONE  // �֊s�̋ߎ���@
		);



		cv::Vec4f line;
		cv::fitLine(contours[0], line, CV_DIST_L2, 0, 0.01, 0.01);

		int y0 = line.val[3] - line.val[1] * line.val[2];

		std::cout << (int)line.val[1] << std::endl;
		std::cout << y0 << std::endl;

		// ������`��
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
					sum += (unsigned int)dst.at<unsigned char>((div*i)*line.val[1] + y0 + k,div*i+j ); //(y,x)�̃s�N�Z���̒l���擾����
					dst.at<unsigned char>((div*i)*line.val[1] + y0 + k, div*i + j)=127;
				}
			}
			std::cout << "count=" << i << ": sum=" << sum << std::endl;
		}

		//cv::rectangle(dst, cv::Point(10, 10), cv::Point(1500, 20),cv::Scalar(255), -1); //�����`��`��

		std::cout << (unsigned int)dst.at<unsigned char>(15, 15) << std::endl;		

		cv::imshow("window",dst);//�摜��\���D

		int key = cv::waitKey(10);

	}
}

