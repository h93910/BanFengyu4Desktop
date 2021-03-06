// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <tchar.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>
#include <sstream>

#include "ximage.h"
#pragma comment(lib, "cximaged.lib")
#pragma comment(lib, "cximagecrtd.lib")

#include <urlmon.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib,"urlmon.lib")

using namespace std;

string dateString = "";
string dateS = "";
/*
	return 0:成功 1:下载失败 2:读取失败 3:图片为白
*/
int downloadPic(string url, string fileName, int retry = 0) {
	//https://satellite.nsmc.org.cn/mongoTile_DSS/NOM/TileServer.php?layer=PRODUCT&PRODUCT=FY4A-_AGRI--_N_DISK_1047E_L1C_MTCC_MULT_NOM_YYYYMMDDhhmmss_YYYYMMDDhhmmss_4000M_V0001.JPG&DATE=20200418&TIME=0245&&ENDTIME=0245&SERVICE=WMS&VERSION=1.1.1&REQUEST=GetMap&FORMAT=image%2Fjpeg&TRANSPARENT=true&LAYERS=satellite&NOTILE=BLACK&TILED=true&WIDTH=256&HEIGHT=256&SRS=EPSG%3A11111&STYLES=&BBOX=2750%2C-2750%2C5500%2C0
	//url = "http://pic104.nipic.com/file/20160715/6171480_185807154956_2.jp";
	//============ wchart_t* 转换================= 
	//size_t len = url.length();//获取字符串长度
	//int nmlen = multibytetowidechar(cp_acp, 0, url.c_str(), len + 1, null, 0);//如果函数运行成功，并且cchwidechar为零，
	////返回值是接收到待转换字符串的缓冲区所需求的宽字符数大小。
	//wchar_t* buffer = new wchar_t[nmlen];
	//multibytetowidechar(cp_acp, 0, url.c_str(), len + 1, buffer, nmlen);
	//============ wchart_t*  End================= 

	LPCTSTR u = url.c_str();
	LPCTSTR n = fileName.c_str();
	HRESULT hr = URLDownloadToFile(NULL, u, n, 0, NULL);
	//HRESULT hr = URLDownloadToFile(NULL, lpc, _T("sky.jpg"), 0, NULL);
	if (hr == S_OK) {
		//查询是否是空图,即全白图
		CxImage image;
		image.Load(n, CXIMAGE_FORMAT_JPG);
		if (image.IsValid()) {//是否正确初始化
			if (image.GetWidth() == 512) {
				cout << "download ok:" << fileName << endl;
				return 0;
			}
			else {
				cout << "download：空图" << endl;
				return 3;
			}
		}
		return 2;
	}
	if (retry == 5) {
		return 1;
	}
	else {
		return downloadPic(url, fileName, ++retry);
	}
}

/*
	retunr 0全部下载成功　1中断
*/
int downloadPicSet() {
	cout << "===================时刻表====================" << endl;
	char ttt[32];
	int timePoints[5 * 8];//时刻表
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 5; j++) {
			switch (j)
			{
			case 0:
				timePoints[i * 5 + j] = 3 * i * 100;//00:00
				break;
			case 1:
				timePoints[i * 5 + j] = 3 * i * 100 + 15;//00:15
				break;
			case 2:
				timePoints[i * 5 + j] = (3 * i + 1) * 100;//01:00
				break;
			case 3:
				timePoints[i * 5 + j] = (3 * i + 2) * 100;//02:00
				break;
			case 4:
				timePoints[i * 5 + j] = (3 * i + 2) * 100 + 45;//02:45
				break;
			}
			sprintf(ttt, "%04d", timePoints[i * 5 + j]);
			cout << ttt << "\t";
		}
		cout << endl;
	}
	cout << "===================时刻表 End====================" << endl;

	// 基于当前系统的当前日期/时间
	time_t now = time(0);
	//cout << "1970 到目前经过秒数:" << now << endl;
	//本地时间
	//tm *ltm = localtime(&now);
	//UTC
	tm *ltm = gmtime(&now);
	// 输出 tm 结构的各个组成部分
	cout << "年: " << 1900 + ltm->tm_year << endl;
	cout << "月: " << 1 + ltm->tm_mon << endl;
	cout << "日: " << ltm->tm_mday << endl;
	cout << "时间: " << ltm->tm_hour << ":";
	cout << ltm->tm_min << ":";
	cout << ltm->tm_sec << endl;

	int effectiveIndex = 0;
	for (int i = 0; i < 40; i++) {
		int timeValue = ltm->tm_hour * 100 + ltm->tm_min;
		if (timePoints[i] >= timeValue) {
			effectiveIndex = i - 1;
			break;
		}
	}

	int out = 0;
	while (true) {
		if (effectiveIndex < 0) {
			effectiveIndex += 40;//倒数
			now -= 60 * 60 * 24;//减去一天
			tm *ltm = gmtime(&now);
		}
		char date[64];
		strftime(date, 64, "%Y%m%d", ltm);
		char buffer[512];
		char format[] = "https://satellite.nsmc.org.cn/mongoTile_DSS/NOM/TileServer.php?layer=PRODUCT&PRODUCT=FY4A-_AGRI--_N_DISK_1047E_L1C_MTCC_MULT_NOM_YYYYMMDDhhmmss_YYYYMMDDhhmmss_4000M_V0001.JPG&DATE=%s&TIME=%04d&&ENDTIME=%04d&SERVICE=WMS&VERSION=1.1.1&REQUEST=GetMap&FORMAT=image%%2Fjpeg&TRANSPARENT=true&LAYERS=satellite&NOTILE=BLACK&TILED=true&WIDTH=256&HEIGHT=256&SRS=EPSG%%3A11111&STYLES=&BBOX=%d%%2C%d%%2C%d%%2C%d";
		sprintf(buffer, format, date, timePoints[effectiveIndex], timePoints[effectiveIndex], 0, 0, 2750, 2750);
		string link = buffer;
		cout << "测试下载:" << date << "_" << timePoints[effectiveIndex] << endl;
		int testResult = downloadPic(link, "test.jpg");
		if (testResult == 0) {//测试通过,开始下载满套图
			char timeString[64];
			sprintf(timeString, "%04d", timePoints[effectiveIndex]);
			stringstream datess;
			datess << date<<"_" << timeString;
			dateS = datess.str();
			dateString = "UTC:" + dateS;
			cout << "开始下载:" << dateString << endl;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					int x1 = -5500 + 2750 * i;
					int y1 = -5500 + 2750 * j;
					int x2 = x1 + 2750;
					int y2 = y1 + 2750;
					sprintf(buffer, format, date, timePoints[effectiveIndex], timePoints[effectiveIndex], x1, y1, x2, y2);
					string link = buffer;
					string filename = to_string(i * 4 + j) + ".jpg";
					downloadPic(link, filename);
				}
			}
			return 0;
		}
		else if (testResult == 3) {//白图
			effectiveIndex--;
			out++;
			if (out >= 5) {
				return 1;
			}
		}
		else {
			cout << "放弃刷新:result " << testResult << ":";
			return 1;
		}
	}
}

void xxxuuuu() {
	int up = 80;
	CxImage temp, f;
	f.Create(512 * 4, 512*4+up, 24, CXIMAGE_FORMAT_PNG);
	//画黑底
	RGBQUAD black = { 0,0,0,0 };
	for (int x = 0; x < 512 * 4; x++) {
		for (int y = 0; y < up; y++) {
			f.SetPixelColor(x, y, black);
		}
	}
	//拼接图片
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			string filename = to_string(i * 4 + j) + ".jpg";
			temp.Load(filename.c_str(), CXIMAGE_FORMAT_JPG);

			if (temp.IsValid()) {//是否正确初始化
				//image.GrayScale();//变灰白处理
				uint32_t a = temp.GetWidth();
				f.MixFrom(temp, 512 * i, 512 * j+up);
				cout << "拼接成功:" << filename << endl;
			}
		}
	}
	//写文字
	CxImage::CXTEXTINFO  dateTag;//图片叠加文字
	 //初始化叠加文字结构体
	f.InitTextInfo(&dateTag);
	_stprintf(dateTag.lfont.lfFaceName, _T("微软雅黑"));
	dateTag.lfont.lfCharSet = GB2312_CHARSET;
	dateTag.lfont.lfHeight = 40;
	dateTag.fcolor = RGB(255, 0, 0);
	dateTag.opaque = 0;
	//从结构体中输入文字的值
	sprintf_s(dateTag.text, "%s", dateString.c_str());
	f.DrawStringEx(0, 512 * 4-200, 512 * 4, &dateTag);

	char finalName[] = "f.png";
	f.Save(finalName, CXIMAGE_FORMAT_PNG);//保存图片
	cout << "拼接完成" << endl;
	//多保存一份时间为名的
	f.Save((dateS+".png").c_str(), CXIMAGE_FORMAT_PNG);//保存图片
	cout << "备份完成" << endl;

	stringstream imgPath;
	char buf[128];
	GetCurrentDirectory(128, buf);
	imgPath << buf<<"\\"<<finalName;
	cout << "图片地址："<< imgPath.str() << endl;
	//system("pause");
	SystemParametersInfoA(20,//用于设定桌面背景
		0,//默认的参数0
		(PVOID)imgPath.str().c_str(),//桌面的背景
		SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);//立刻改变桌面
	cout << "屏纸刷新完成" << endl;
}

int main(int argc, char* argv[])
{
	//获取参数
	if (argc > 1) {
		cout << argc << endl;
		cout << argv[0] << endl;
	}
	//downloadPic("");
	//testCx();
	if (downloadPicSet()==0) {
		xxxuuuu();
	}
}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
