
#include "findRect.h"



/**
��ֱ�ߵĽ���

@param a �߶�1
@param b �߶�2
@return ����
*/
cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b)
{
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];

	if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
	{
		cv::Point2f pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		return pt;
	}
	else
		return cv::Point2f(-1, -1);
}

/**
�Զ���㰴˳ʱ������
@param corners ��ļ���
*/
void sortCorners(std::vector<cv::Point2f>& corners)
{
	if (corners.size() == 0) return;
	//���� X������
	cv::Point pl = corners[0];
	int index = 0;
	for (int i = 1; i < corners.size(); i++)
	{
		cv::Point point = corners[i];
		if (pl.x > point.x)
		{
			pl = point;
			index = i;
		}
	}
	corners[index] = corners[0];
	corners[0] = pl;

	cv::Point lp = corners[0];
	for (int i = 1; i < corners.size(); i++)
	{
		for (int j = i + 1; j<corners.size(); j++)
		{
			cv::Point point1 = corners[i];
			cv::Point point2 = corners[j];
			if ((point1.y - lp.y*1.0) / (point1.x - lp.x)>(point2.y - lp.y*1.0) / (point2.x - lp.x))
			{
				cv::Point temp = point1;
				corners[i] = corners[j];
				corners[j] = temp;
			}
		}
	}
}
/**
�㵽��ľ���
@param p1 ��1
@param p2 ��2
@return ����
*/
double getSpacePointToPoint(cv::Point p1, cv::Point p2)
{
	int a = p1.x - p2.x;
	int b = p1.y - p2.y;
	return sqrt(a * a + b * b);
}
int findLargestSquare(const vector<vector<cv::Point> >& squares, vector<cv::Point>& biggest_square)
{
	if (!squares.size()) return -1;

	int max_width = 0;
	int max_height = 0;
	int max_square_idx = 0;
	for (int i = 0; i < squares.size(); i++)
	{
		cv::Rect rectangle = boundingRect(Mat(squares[i]));
		if ((rectangle.width >= max_width) && (rectangle.height >= max_height))
		{
			max_width = rectangle.width;
			max_height = rectangle.height;
			max_square_idx = i;
		}
	}
	biggest_square = squares[max_square_idx];
	return max_square_idx;
}
/**
��������������м��Ǹ���ļн�   pt1 pt0 pt2
*/
double getAngle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}
double getDistance(cv::Point pt1, cv::Point pt2)
{
	double dx = pt1.x - pt2.x;
	double dy = pt1.y - pt2.y;
	return sqrt(dx*dx + dy*dy);
}



bool findRect(Mat& mat)
{

	static unsigned long long frameID = 0;
	++frameID;
	bool isfind = false;
	static unsigned int voteCount = 0;

	Mat grayImg;
	//��ȡ�Ҷ�ͼ��
	cvtColor(mat, grayImg, CV_BGR2GRAY);

	GaussianBlur(grayImg, grayImg, Size(3, 3), 2, 2);

	normalize(grayImg, grayImg, 0, 255, NORM_MINMAX, CV_8UC1);

	threshold(grayImg, grayImg, 200, 255, CV_THRESH_BINARY); //��ֵ��
	//�������״ �����̫������ʱ����ͨosd�����Ŀ��
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(-1, -1));
	//��ʴ����
	dilate(grayImg, grayImg, element);
	erode(grayImg, grayImg, element);

	int thresh = 50;
	//��Ե���  ���ص��ݶȴ��ڵ�һ����ֵ�ı�Ȼ�Ǳ�Ե���ݶ�ֵ�ڵ�һ����ֵ��ڶ�����ֵ֮����Ϊ���Ʊ�Ե�㣬�ٸ�����ͨ���ж�
	//Canny(grayImg, grayImg, thresh, thresh * 3, 3);

	imshow("������", grayImg);

	vector<vector<cv::Point> > contours;
	//Ѱ�ұ߿�
	findContours(grayImg, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	//cout << contours.size() << endl;

	vector<cv::Point> hull, tmpPolygon, polygon;

	//�趨��С���
	double minArea = grayImg.rows * grayImg.cols / 2500;
	//cout << grayImg.rows << endl<< grayImg.cols << endl << minArea << endl << endl;

	//��ѡ����
	vector<vector<Point>> final_cont;
	//���ɸѡ
	for (int i = 0; i < contours.size(); ++i)
	{
		double area = contourArea(contours[i]);
		//cout << area << endl;
		if (area > minArea) {
			final_cont.push_back(contours[i]);
			Scalar color = Scalar(0,0,255);
			drawContours(mat, contours, i, color,1, 8);
		}
	}

	//�������������
	if (final_cont.size() == 0)
	{
		cout << "  No target found !" << endl;
		return isfind;
	}

	int idx = 0;
	for (idx = 0; idx < final_cont.size(); idx++)
	{
		//�߿��͹��
		convexHull(final_cont[idx], hull);
		//��������͹���߿�(��ʱ����ϵľ��Ƚϵ�)
		//��Ҫ�����ǰ�һ�������⻬�������߻�����ͼ����������ж�������
		approxPolyDP(final_cont[idx], polygon, 2, true);


		for (int i = 0; i < polygon.size(); ++i)
		{
			circle(mat, polygon[i], 4, Scalar(0, 255, 0));
		}
		//ɸѡ�������Ƕȶ��ӽ�ֱ�ǵ�͹�ı���
		bool isConvex = isContourConvex(Mat(polygon));
		if (polygon.size() == 4 && isConvex)
		{
			cout << frameID << "  find candidate   " << endl;
			//�߳��ж�
			double maxL = arcLength(polygon, true)*0.05;//�ú����������߳��Ȼ�պ������ܳ�

														//�ҵ�ʣ�ඥ�������У��߳����� 2 * maxL����������Ϊ�ı��������������
			vector<Vec4i> lines;
			for (int i = 0; i < polygon.size(); i++)
			{
				cv::Point p1 = polygon[i];
				cv::Point p2 = polygon[(i + 1) % polygon.size()];
				if (getSpacePointToPoint(p1, p2) > maxL)
				{
					lines.push_back(Vec4i(p1.x, p1.y, p2.x, p2.y));
				}
			}

			//�Ƕ��ж�
			double maxCosine = 0;

			for (int j = 2; j < 5; j++)
			{
				double cosine = fabs(getAngle(polygon[j % 4], polygon[j - 2], polygon[j - 1]));
				maxCosine = MAX(maxCosine, cosine);
			}

			cout << "vote:  " << voteCount << endl;
			//Ҫ����С�Ǵ���45��,���ڼ��������
			if (maxCosine < 0.7 && lines.size() == 4)
			{
				cout << frameID << "  Find target   " << endl;
				//ɸѡ��ӽ�Բ������
				//�ҵ�Ŀ��
				isfind = true;

				if (voteCount > 2)
				{
					//��������������� ���������ߵĽ��㣬��������ĸ�����
					vector<cv::Point> vCorners;
					for (int i = 0; i < lines.size(); i++)
					{
						cv::Point cornor = computeIntersect(lines[i], lines[(i + 1) % lines.size()]);
						vCorners.push_back(cornor);
					}
					int targetX = (vCorners[0].x + vCorners[2].x) / 2;
					int targetY = (vCorners[0].y + vCorners[2].y) / 2;

					cout << idx << "   x = " << targetX << "  y = " << targetY << endl;

					char str[64] = { 0 };
					sprintf(str, "%d x:%d y:%d", idx, targetX, targetY);
					cv::Point mark = cv::Point(targetX+20, targetY);
					putText(mat, str, mark, cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
					
					//���Ƴ�������
					for (int i = 0; i < vCorners.size(); i++)
					{
						line(mat, vCorners[i], vCorners[(i + 1) % vCorners.size()], Scalar(0, 0, 255), 2);
					}
				}
			}
		}
		else
		{
			cout << frameID << "  No target   " << endl;
		}
	}

	if (isfind) ++voteCount;
	else voteCount = 0;

	/*
	//�ҵ���������ı��ζ�Ӧ��͹�߿��ٴν��ж������ϣ��˴ξ��Ƚϸߣ���ϵĽ�������Ǵ���4���ߵĶ����
	//�������Ĳ�������Ҫ��Ϊ�˽��Բ�Ƕ�������߻����бߵ�����
	hull = hulls[idex];
	approxPolyDP(hull, polygon, 3, true);
	vector<cv::Point> newApprox;



	//�ҵ��߾������ʱ�õ��Ķ����� ����С�� �;�����ϵõ����ĸ����� maxL�Ķ��㣬�ų����ֶ���ĸ���
	//c++11�±�׼
	for (cv::Point p : polygon)
	{
	if (!(getSpacePointToPoint(p, largest_square[0]) > maxL &&
	getSpacePointToPoint(p, largest_square[1]) > maxL &&
	getSpacePointToPoint(p, largest_square[2]) > maxL &&
	getSpacePointToPoint(p, largest_square[3]) > maxL))
	{
	newApprox.push_back(p);
	}
	}

	*/


	return isfind;
}
