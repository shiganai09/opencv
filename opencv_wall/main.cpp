#include <opencv2/opencv.hpp>

int main()
{
	/*Web�J���������ݒ�*/
	cv::VideoCapture cap(0);//�f�o�C�X�̃I�[�v��(�G���[���o��g���ꍇ'1'��������Ȃ�)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	cap.set(CV_CAP_PROP_EXPOSURE, -13); //�I�o�������ăV���b�^�[���x���グ��

	/*����͎g��Ȃ�
	if (!cap.isOpened())//�J�����f�o�C�X������ɃI�[�v���������m�F
	{
		//�ǂݍ��݂Ɏ��s�����Ƃ��̏���
		return -1;
	}
	*/

	cv::Mat pict = cv::imread("testPict.png"); //�摜�̓ǂݍ���

	while (1) {
		cv::Mat color;//�J���[�摜Mat
		cv::Mat gray;//�O���[�X�P�[���摜Mat

		//pict >> color;//USB�J��������̉摜�����
		color = pict.clone(); //�J��������̓��͂̑��

		cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);//�O���[�X�P�[����
		cv::Mat bin;//��l���摜Mat
		cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		cv::Mat dst=bin.clone(); //�o�͗p�̉摜Mat��p�ӂ��Ă���

		//�`�����l�����m�F�p�i����Ȃ��j
		std::cout << "color ch:" << color.channels() << std::endl;
		std::cout << "gray ch:" << gray.channels() << std::endl;
		std::cout << "bin ch:" << bin.channels() << std::endl;
		std::cout << "dst ch:" << dst.channels() << std::endl;


		std::vector<std::vector<cv::Point>>contours;
		std::vector<cv::Vec4i> hierarchy;

		// 2�l�摜���̗֊s�����o
		cv::findContours(bin,     //���͉摜�C8�r�b�g�C�V���O���`�����l���D0�ȊO�̃s�N�Z���� 1 �A0�̃s�N�Z����0�Ƃ��Ĉ����B�������ʂƂ��� image �����������邱�Ƃɒ��ӂ���.
			contours,             // �֊s��_�x�N�g���Ƃ��Ď擾����
			hierarchy,            // hiararchy ? �I�v�V�����D�摜�̃g�|���W�[�Ɋւ�������܂ޏo�̓x�N�g���D
			CV_RETR_EXTERNAL,     // �֊s���o���[�h
			CV_CHAIN_APPROX_NONE  // �֊s�̋ߎ���@
		);


		cv::Vec4f line; //�t�B�b�e�B���O������[vx,vy,x0,y0]{vx,vy(x,y�����̕����x�N�g��),x0,y0(�ǂ����킩��Ȃ�������̓_)}

		cv::fitLine(contours[0], line, CV_DIST_L2, 0, 0.01, 0.01);

		int y0 = line.val[3] - line.val[1] * line.val[2]; //y�ؕЂ����߂�(y0-m*x0) �Fm�͈ꎟ�֐��̌X��

		std::cout << (int)line.val[1] << std::endl; //�ꎟ�֐��̌X��
		std::cout << y0 << std::endl; //y�ؕ�(x=0�ł�y�̒l(�I�t�Z�b�g))

		/*
		// ������`��(x=0 -> x=imageWidth)
		cv::line(dst,
			cv::Point(0, y0),
			cv::Point(dst.cols, line.val[1]+y0),
			cv::Scalar(255),
			3,
			cv::LINE_8
		);
		*/
		
		//��ɂ���ĉe�ɂȂ����ʒu�����o���邽�߂ɁC���C������ړ����鏬���Ȑ����`���l����D
		//���̐����`�Ɋ܂܂���f�l(���m�N��)�̑��a�����߂Ă����i��������΂����͉e�j
		const int DIV = 2; //�����̕�
		const int  MAX_COUNT= dst.cols /DIV; //�[�܂ōs�����߂ɕK�v�Ȑ�

		std::vector<int> fallPoint;
		std::vector<int> raisePoint;
		bool falling = false;

		for (int i = 1; i < MAX_COUNT; i++) {
			int sum = 0;
			for (int j = -DIV / 2; j < DIV / 2; j++) {
				for (int k = -DIV / 2; k < DIV / 2; k++) {
					sum += (unsigned int)dst.at<unsigned char>((DIV*i)*line.val[1] + y0 + k,DIV*i+j ); //(y,x)�̃s�N�Z���̒l���擾����
					dst.at<unsigned char>((DIV*i)*line.val[1] + y0 + k, DIV*i + j)=127; //�v�Z���s�����������O���[�œh��Ԃ�
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


		//cv::rectangle(dst, cv::Point(10, 10), cv::Point(1500, 20),cv::Scalar(255), -1); //�����`��`��		

		cv::imshow("window",dst);//�摜��\���D

		int key = cv::waitKey(10);

	}
}

