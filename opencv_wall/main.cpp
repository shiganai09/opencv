#include <opencv2/opencv.hpp>

int main()
{
	/*Web�J���������ݒ�*/
	cv::VideoCapture cap(0);//�f�o�C�X�̃I�[�v��(�G���[���o��g���ꍇ'1'��������Ȃ�)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	//cap.set(CV_CAP_PROP_EXPOSURE, -10); //�I�o�������ăV���b�^�[���x���グ��

	if (!cap.isOpened())//�J�����f�o�C�X������ɃI�[�v���������m�F
	{
		//�ǂݍ��݂Ɏ��s�����Ƃ��̏���
		return -1;
	}

	/*Web�J���������ݒ�*/
	cv::VideoCapture cap2(1);//�f�o�C�X�̃I�[�v��(�G���[���o��g���ꍇ'1'��������Ȃ�)
	cap2.set(cv::CAP_PROP_FPS, 30.0);
	cap2.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap2.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	//cap2.set(CV_CAP_PROP_EXPOSURE, -10); //�I�o�������ăV���b�^�[���x���グ��

	if (!cap2.isOpened())//�J�����f�o�C�X������ɃI�[�v���������m�F
	{
		//�ǂݍ��݂Ɏ��s�����Ƃ��̏���
		return -1;
	}

	//cv::Mat pict = cv::imread("testPict.png"); //�摜�̓ǂݍ���(web�J�����Ȃ��Ƃ�)

	while (1) {
		cv::Mat color;//�J���[�摜Mat
		cv::Mat gray;//�O���[�X�P�[���摜Mat
		cap >> color; //USB�J��������̉摜�����
					  //color = pict.clone(); //�J��������̓��͂̑��(web�J�����Ȃ��Ƃ�)

		cv::Mat color2;
		cap2 >> color2;

		cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);//�O���[�X�P�[����
		cv::Mat bin;//��l���摜Mat
		cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
		cv::Mat dst = bin.clone(); //�o�͗p�̉摜Mat��p�ӂ��Ă���

		std::vector<std::vector<cv::Point>>contours;
		std::vector<cv::Vec4i> hierarchy;

		// 2�l�摜���̗֊s�����o
		cv::findContours(bin,     //���͉摜�C8�r�b�g�C�V���O���`�����l���D0�ȊO�̃s�N�Z���� 1 �A0�̃s�N�Z����0�Ƃ��Ĉ����B�������ʂƂ��� image �����������邱�Ƃɒ��ӂ���.
			contours,             // �֊s��_�x�N�g���Ƃ��Ď擾����
			hierarchy,            // hiararchy ? �I�v�V�����D�摜�̃g�|���W�[�Ɋւ�������܂ޏo�̓x�N�g���D
			CV_RETR_EXTERNAL,     // �֊s���o���[�h
			CV_CHAIN_APPROX_NONE  // �֊s�̋ߎ���@
		);

		cv::Vec4f line; //�t�B�b�e�B���O�������̏��[vx,vy,x0,y0]{vx,vy(x,y�����̕����x�N�g��),x0,y0(�ǂ����킩��Ȃ�������̓_)}

		cv::fitLine(contours[0], line, CV_DIST_L2, 0, 0.01, 0.01);//�t�B�b�e�B���O����

		float grad_m = line.val[1] / line.val[0]; // dy/dx
		int y0 = line.val[3] - grad_m * line.val[2]; //y�ؕЂ����߂�(y0-m*x0) �Fgrad_m�͈ꎟ�֐��̌X��

		const int Y_LIMIT = 50;
		//�t�B�b�e�B���O�������������͈͓��ɓ����Ă��邩�ǂ���(true�͓����Ă���j
		bool in_range = true;
		if (y0 < Y_LIMIT) {
			in_range = false;
		}
		if (grad_m * dst.cols + y0 >= dst.rows) {
			in_range = false;
		}
		if (y0 > dst.rows - Y_LIMIT) {
			in_range = false;
		}
		if (grad_m * dst.cols + y0 <0) {
			in_range = false;
		}

		/*
		// �t�B�b�e�B���O��������`��(x=0 -> x=imageWidth)
		cv::line(dst,
		cv::Point(0, y0),
		cv::Point(dst.cols, line.val[1]* dst.cols +y0),
		cv::Scalar(127),
		1,
		cv::LINE_8
		);
		*/

		if (in_range) {
			//�^�b�`������ɂ���ĉe�ɂȂ����ʒu�����o���邽�߂ɁC�t�B�b�e�B���O����������ړ����鏬���Ȑ����`���l����D
			//���̐����`�Ɋ܂܂���f�l(���m�N��)�̑��a�����߂Ă����i��������΂����͉e�j
			const int DIV = 3; //�����̕�
			const int  MAX_COUNT = dst.cols / DIV - 1; //�[�܂ōs�����߂ɕK�v�Ȑ�

			std::vector<int> fallPoint;
			std::vector<int> raisePoint;
			bool falling = false;

			for (int i = 1; i < MAX_COUNT; i++) {
				int sum = 0;
				for (int j = -(DIV / 2); j < DIV / 2 + 1; j++) {
					for (int k = -(DIV / 2); k < DIV / 2 + 1; k++) {
						sum += (int)dst.at<unsigned char>((int)(DIV*i)*grad_m + y0 + k, DIV * i + j);//(y,x)�̃s�N�Z���̒l���擾����
						dst.at<unsigned char>((DIV*i)*line.val[1] + y0 + k, DIV*i + j) = 127; //�v�Z���s�����������O���[�œh��Ԃ�
					}
				}
				if (!falling && sum == 0) { fallPoint.emplace_back(i); falling = true; }
				if (falling && sum > 0) { raisePoint.emplace_back(i); falling = false; }
				//std::cout << "Count=" << i << ": sum=" << sum << std::endl;
			}

			int touchCentor = 0;
			if (fallPoint.size() == 0) { std::cout << "no point" << std::endl; }
			else if ((int)fallPoint.size() != (int)raisePoint.size()) { std::cout << "error(don't match fallPoint and raisePoint )" << (int)fallPoint.size() << " : " << (int)raisePoint.size() << std::endl; }
			else if (fallPoint.size() > 1) { std::cout << "error(too many point)" << std::endl; }
			else {
				touchCentor = (raisePoint[0] + fallPoint[0]) / 2;
				int theta = touchCentor * 60 / MAX_COUNT;
				//std::cout << "CentorPoint = " << touchCentor << std::endl;
				std::cout << "theta = " << theta << std::endl;
			}

		}
		cap >> color;
		cv::imshow("window", color); //USB�J��������̉摜�����//�摜��\���D
		cv::imshow("window2", color2);
		int key = cv::waitKey(10);
	}
}