#include <opencv2/opencv.hpp>

int main()
{
	/*Web�J���������ݒ�*/
	cv::VideoCapture cap(0);//�f�o�C�X�̃I�[�v��(�G���[���o��g���ꍇ'1'��������Ȃ�)
	cap.set(cv::CAP_PROP_FPS, 30.0);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);//�𑜓x ��(�\��)
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);//�𑜓x �c(�\��)
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap.set(CV_CAP_PROP_EXPOSURE, -13);//�I�o�������ăV���b�^�[���x���グ��

	if (!cap.isOpened())//�J�����f�o�C�X������ɃI�[�v���������m�F
	{
		//�ǂݍ��݂Ɏ��s�����Ƃ��̏���
		return -1;
	}

	while (1) {
		cv::Mat color;//�J���[�摜

		cap >> color;//USB�J��������̉摜�����


		cv::imshow("window", color);//�摜��\���D

		int key = cv::waitKey(10);

	}
}

