#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>
#include <ctime>

using namespace std;
using namespace cv;

int main()
{
	clock_t begin, end;
	double msec;
	begin = clock();

	Mat frame, hsv, y_thres, gray, w_thres, sum, canny, gauss;
	vector<Mat> planes(3);
	vector<Mat> masked(3);
	VideoCapture capture("주간1.avi");

	int cnt_pos_lines = 0;
	int cnt_neg_lines = 0;

	vector<Vec4i> lines;

	Vec4i pos_pl1;
	Vec4i pos_pl2;

	Vec4i neg_pl1;
	Vec4i neg_pl2;

	Vec4i pos_prev_pl1;
	Vec4i pos_prev_pl2;

	Vec4i neg_prev_pl1;
	Vec4i neg_prev_pl2;


	while (1)
	{
		if (!capture.read(frame))
			break;

		Rect rect = Rect(frame.cols / 3, frame.rows / 2.5, frame.cols / 2.5, frame.rows / 2.5);
		Mat roi = frame(rect);

		cvtColor(roi, hsv, CV_BGR2HSV);
		cvtColor(roi, gray, CV_BGR2GRAY);

		inRange(hsv, Scalar(0, 25, 165), Scalar(35, 177, 255), y_thres);
		threshold(gray, w_thres, 185, 255, THRESH_BINARY);

		bitwise_or(y_thres, w_thres, sum);

		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
		morphologyEx(sum, sum, CV_MOP_CLOSE, element);

		// 가우시안 - 스무딩
		GaussianBlur(sum, gauss, Size(5, 5), 0);

		//canny - edge검출기
		Canny(gauss, canny, 20, 20); // 다시 조절해보자

									 //hough - 점들로부터 직선검출
		HoughLinesP(canny, lines, 1, CV_PI / 180, 120, 20, 100);
		//imshow("hough", canny);

		for (size_t i = 0; i < lines.size(); i++) // 이번 frame에서 검출된 모든 라인을 그려주기 위한 for문
		{
			Vec4i l = lines[i];
			double a = ((double)l[3] - (double)l[1]) / ((double)l[2] - (double)l[0]); // a : 기울기 cf) 영상에서 기울기는 보통과 다름. 좌표평면이 아님을 유의!


			if (!(a >= 0 && a < 0.35) && !(a <= 0 && a > -0.35)) // 기울기 조건으로 일부라인 제외시킴
			{
				if (a > 0)
				{
					cnt_pos_lines++;

					if (cnt_pos_lines == 1) // prev line
					{
						pos_pl1 = l;
					}
					else
					{
						pos_pl2 = l;
					}

				}
				else
				{
					cnt_neg_lines++;

					if (cnt_neg_lines == 1) // prev line
					{
						neg_pl1 = l;
					}
					else
					{
						neg_pl2 = l;
					}

				}
			}
		}




		////////////// pos /////////////////////////// - 좌상 우하

		// 1. 못찾는 경우
		if (cnt_pos_lines == 0)
		{
			// 이거는 pl1만 그려보는 코드
			Point point1 = Point(pos_prev_pl1[0], pos_prev_pl1[1]);
			Point point2 = Point(pos_prev_pl1[2], pos_prev_pl1[3]);

			if ((point2.y - point1.y) != 0)
			{
				double _x = -point1.y*((point2.x - point1.x) / (point2.y - point1.y)) + point1.x;// x절편
				line(roi, Point(_x, 0), point2, Scalar(0, 0, 255), 3); // 백업해 놓은걸 그린다.
			}
			else {
				line(roi, point1, point2, Scalar(0, 0, 255), 3); // 백업해 놓은걸 그린다.
			}
		}

		// 2. pos_pl1만 존재한다면,
		else if (cnt_pos_lines == 1)
		{
			Point point1 = Point(pos_pl1[0], pos_pl1[1]);
			Point point2 = Point(pos_pl1[2], pos_pl1[3]);

			double _x = -point1.y*((point2.x - point1.x) / (point2.y - point1.y)) + point1.x;// x절편

			line(roi, Point(_x, 0), point2, Scalar(0, 0, 255), 3);

			pos_prev_pl1 = pos_pl1;// 백업1
			pos_prev_pl2 = pos_pl1;// 백업1 !!!!!!!!!!!!
		}

		// 3. pos_pl1, pos_pl2 둘다 존재한다면,
		else
		{
			// 1) 라인 평균내기 - 두 점의 중점을 구한다음에 라인 그리자
			Point pos_mid_point1 = Point(((double)pos_pl1[0] + (double)pos_pl2[0]) / 2, ((double)pos_pl1[1] + (double)pos_pl2[1]) / 2);
			Point pos_mid_point2 = Point(((double)pos_pl1[2] + (double)pos_pl2[2]) / 2, ((double)pos_pl1[3] + (double)pos_pl2[3]) / 2);

			double _x = -pos_mid_point1.y*((pos_mid_point2.x - pos_mid_point1.x) / (pos_mid_point2.y - pos_mid_point1.y)) + pos_mid_point1.x;// x절편

			line(roi, Point(_x, 0), pos_mid_point2, Scalar(0, 0, 255), 3); // 평균낸거 그림

			pos_prev_pl1 = pos_pl1; // 백업1
			pos_prev_pl2 = pos_pl2; // 백업2

		}





		////////////// neg /////////////////////////// - 우상 좌하

		/*
		Point point1 = Point(neg_pl1[0], neg_pl1[1]);
		Point point2 = Point(neg_pl1[2], neg_pl1[3]);

		double _x = -point1.y*((point2.x - point1.x) / (point2.y - point1.y)) + point1.x;// x절편

		//line(roi, Point(_x, 0), point2, Scalar(0, 0, 255), 3);
		//line(roi, point2, point1, Scalar(0, 0, 255), 3);
		//line(roi, point1, point2, Scalar(0, 0, 255), 3);

		line(roi, Point(_x, 0), point1, Scalar(0, 0, 255), 3);
		//line(roi, point1, Point(_x, 0), Scalar(0, 0, 255), 3);


		neg_prev_pl1 = neg_pl1;// 백업

		*/


		// 1. 못찾는 경우
		if (cnt_neg_lines == 0)
		{
			// pl1만 그려보는 코드
			Point point1 = Point(neg_prev_pl1[0], neg_prev_pl1[1]);
			Point point2 = Point(neg_prev_pl1[2], neg_prev_pl1[3]);

			//double _x = -point1.y*((point2.x - point1.x) / (point2.y - point1.y)) + point1.x;// x절편

			line(roi, point2, point1, Scalar(0, 0, 255), 3);
			//line(roi, Point(_x, 0), point1, Scalar(0, 0, 255), 3);


		}

		// 2. pl1 하나만 존재한다면,
		else if (cnt_neg_lines == 1)
		{
			Point point1 = Point(neg_pl1[0], neg_pl1[1]);
			Point point2 = Point(neg_pl1[2], neg_pl1[3]);

			//double _x = -point1.y*((point2.x - point1.x) / (point2.y - point1.y)) + point1.x;// x절편

			//line(roi, Point(_x, 0), point1, Scalar(0, 0, 255), 3);
			line(roi, point2, point1, Scalar(0, 0, 255), 3);


			neg_prev_pl1 = neg_pl1;// 백업
			neg_prev_pl2 = neg_pl1;// 백업1!!!!!!!!!!!
		}

		// 3. pl1, pl2 둘다 존재한다면,
		else
		{
			// 1) 라인 평균내기 - 두 점의 중점을 구한다음에 라인 그리자
			Point neg_mid_point1 = Point(((double)neg_pl1[0] + (double)neg_pl2[0]) / 2, ((double)neg_pl1[1] + (double)neg_pl2[1]) / 2);
			Point neg_mid_point2 = Point(((double)neg_pl1[2] + (double)neg_pl2[2]) / 2, ((double)neg_pl1[3] + (double)neg_pl2[3]) / 2);

			//double _x = -neg_mid_point1.y*((neg_mid_point2.x - neg_mid_point1.x) / (neg_mid_point2.y - neg_mid_point1.y)) + neg_mid_point1.x;// x절편

			//line(roi, Point(_x, 0), neg_mid_point1, Scalar(0, 0, 255), 3); // 평균낸거 그림
			line(roi, neg_mid_point2, neg_mid_point1, Scalar(0, 0, 255), 3);

			neg_prev_pl1 = neg_pl1; // 백업1
			neg_prev_pl2 = neg_pl2; // 백업2

		}



		/////////////////////////////////////////////////////////////////


		cnt_pos_lines = 0;
		cnt_neg_lines = 0;




		rectangle(frame, rect, Scalar(0, 255, 0));

		imshow("My Window", frame);


		char c = (char)waitKey(20);
		if (c == 27)
			break;
		if (capture.get(CV_CAP_PROP_POS_MSEC) > 60000)
			break;

	}

	end = clock();
	msec = 1000.0*(end - begin) / CLOCKS_PER_SEC;
	cout << "Run time: " << msec << " msec" << endl;


	return 0;
}


/*
1. roi영역 세밀하게 조절 - 하지만 다른 영상도 고려해서 조절해야함.
2. 한줄로 나타나도록 - 평균값(Vec4i로 line을 저장하는 방법, int[]로 point를 저장하는 방법)
3.

*/

//////////////////////////
/*
// 2) pos_pl1, pos_pl2 누가 더 기나?
int dis_pl1 = (pos_pl1[2] - pos_pl1[0])*(pos_pl1[2] - pos_pl1[0]) - (pos_pl1[3] - pos_pl1[1])*(pos_pl1[3] - pos_pl1[1]);
int dis_pl2 = (pos_pl2[2] - pos_pl2[0])*(pos_pl2[2] - pos_pl2[0]) - (pos_pl2[3] - pos_pl2[1])*(pos_pl2[3] - pos_pl2[1]);

// 1) 더 긴거 찾아서 그려줄게
if (dis_pl1 > dis_pl2)
{
Point point1 = Point(pos_pl1[0], pos_pl1[1]);
Point point2 = Point(pos_pl1[2], pos_pl1[3]);

double _x = -point1.y*((point2.x - point1.x) / (point2.y - point1.y)) + point1.x;// x절편

line(roi, Point(_x, 0), point2, Scalar(0, 0, 255), 3);
prev_pl = pos_pl1; // 백업

}else
{

Point point1 = Point(pos_pl2[0], pos_pl2[1]);
Point point2 = Point(pos_pl2[2], pos_pl2[3]);

double _x = -point1.y*((point2.x - point1.x) / (point2.y - point1.y)) + point1.x;// x절편

line(roi, Point(_x, 0), point2, Scalar(0, 0, 255), 3);
prev_pl = pos_pl2; // 백업
}
*/
