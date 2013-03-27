// proprocess.cpp : Defines the entry point for the application.
//

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <map>


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR szTitle[]=_T("PreProcess");
TCHAR szWindowClass[]=_T("MYPREPROCESSWNDCLASS");




#define MAX_SHAPE_COUNT  (1000000)//圆数量的上限

const double	pi = 3.1415926535897932;//PI...
int				radtype = 0; //一共有几种半径
double			fWidth = 0.0; //边长
double			fHeight = 0.0; //Y轴半径
double			fCenterCircleRad = 0.0;   //居中的圆的半径
int				nPointsForWidth = 0;	  //边长分配的点数
int				nPointsForHeight = 0;	  //边长分配的点数
int				nPointsPerCircle = 0;//每个圆需要的点数
double			fRadius[1024]; //存放半径
int				nCircleCount[1024]; //每个半径下的圆球数
int				nPointCount = 0;//点的数量
int				nPointCountNeeded = 0;//总共需要的点的数量
int				nShapeParamCount;//一共定义了几种元素
int				nShapeCount;
int				nShapeToSpawn = 0;
double			fScale = 1.0;
double			fOffsetX = 500.0;
double			fOffsetY = 500.0;
float			fShapeSpace = 0.0;
int				nMButtonDown = 0;
POINT			ptMDownPoint;
double			fMDOffsetX = 0;
double			fMDOffsetY = 0;
int				nBoundType = 0;
double			sin54 = 0.0;
#define 		nSplitLevel  (10)//正方形切成10*10的小块来优化
const int		nCustomShapeCount = 8;

struct PolyRecord_T
{
	std::vector<int> startpointid;
	std::vector<int> endpointid;
};

std::map< int , std::map<float, PolyRecord_T> > polyrecordmap;


struct CustomShape_T
{
	double	fRotate[100];
	double	fLength[100];
	int		nCount;
}CustomShapes[8];


enum BoundType
{
	btRect,
	btCircle,
	btEllipse,
	btRect2,
	btRectWithOneCircle,
	btCircleWithOneCircle,
	btPentagon,
	btEllipsWithRandomY
};

struct Vector2_T
{
	double x;
	double y;
};

struct Point_T
{
	double x;
	double y;
	int nNextID;
	int nSelfID;
}*Points;//存放点;

enum IntersectType
{
	itOutSide,
	itTangent,
	itIntersect,
	itInside,
	itInclude,
};

struct  Circle_T
{
	double x;
	double y;
	double rad;
} circles[100];//存放圆

enum ShapeType
{
	stCircle,
	stEllipse,
	stPentagon,
	stCenteredCircleInRect,
	stFromLib,
};

struct Shape_T
{
	int					nType;
	struct Vector2_T	vPos;
	union
	{
		double			fParam1;
		double			fRad;
		double			fRadX;
	};
	union
	{
		double			fParam2;
		double			fRadY;
	};
	int					iParam1;
	double				fRotate;
	struct Shape_T		*pNext;

} shapes[MAX_SHAPE_COUNT];

struct ShapeParam_T
{
	int nType;
	int nCount;
	union
	{
		double			fParam1;
		double			fRad;
		double			fRadX;
	};
	union
	{
		double			fParam2;
		double			fRadY;
	};
	int	nPolyType;
} shapeparams[1024];

struct Sector

{
	struct Shape_T* pShapes;
};
struct Sector sectors[nSplitLevel][nSplitLevel];

void AddShape( struct Shape_T* pShape )
{
	//int splitx,splity;
	//struct Shape_T* p;
	//struct Vector2_T vTopRight, vBottomLeft;
	//splitx = (int)(pShape->vPos.x / nSplitLevel);
	//splity = (int)(pShape->vPos.y / nSplitLevel);


	//vTopRight.x = (splitx + 1) * 10;
	//vTopRight.y = splity * 10;
	//vBottomLeft.x = splitx * 10;
	//vBottomLeft.y = (splity + 1) * 10;

	//p = sectors[splitx][splity].pShapes;


	//if ( sectors[splitx][splity].pShapes == NULL )
	//{
	//	sectors[splitx][splity].pShapes = pShape;
	//}
	//else
	//{

	//	while( p->pNext != NULL )
	//	{
	//		p = p->pNext;
	//	}
	//	p->pNext = pShape;
	//}
}

double MyRandom( double fmax, double fmin )//求随机数    fmin <= result < fmax
{
	double r = 0;
	int rnd = 0;
	while ( (rnd = rand()) == RAND_MAX );
	return ((double)(rnd)/RAND_MAX)*(fmax-fmin) + fmin;
}


BOOL TestCollision( struct Shape_T *shp1, struct Shape_T *shp2);//判断是否重叠
void Process();
void DrawGraphic( HDC dc );
void OutPut();
void SpawnShape( struct ShapeParam_T* sparam );

int _tmain(int argc, _TCHAR* argv[])
{
	MSG msg;



	HINSTANCE hInstance = GetModuleHandle(NULL);
	MyRegisterClass(hInstance);//注册窗口类
//
	srand( time(0) );//设置随机数种子	
	Points = NULL;
	memset( shapes, 0, sizeof shapes );

	//init custom shapes

	CustomShapes[0].nCount = 8;
	CustomShapes[0].fLength[0] = 0.909;
	CustomShapes[0].fRotate[0] = 21;
	CustomShapes[0].fLength[1] = 0.559;
	CustomShapes[0].fRotate[1] = 188;
	CustomShapes[0].fLength[2] = 0.339;
	CustomShapes[0].fRotate[2] = 128;
	CustomShapes[0].fLength[3] = 0.232;
	CustomShapes[0].fRotate[3] = 102;
	CustomShapes[0].fLength[4] = 0.437;
	CustomShapes[0].fRotate[4] = 266;
	CustomShapes[0].fLength[5] = 1.122;
	CustomShapes[0].fRotate[5] = 43;
	CustomShapes[0].fLength[6] = 0.643;
	CustomShapes[0].fRotate[6] = 147;
	CustomShapes[0].fLength[7] = 0.541;
	CustomShapes[0].fRotate[7] = 143;

	CustomShapes[1].nCount = 7;
	CustomShapes[1].fLength[0] = 0.918;
	CustomShapes[1].fRotate[0] = 30;
	CustomShapes[1].fLength[1] = 0.371;
	CustomShapes[1].fRotate[1] = 165;
	CustomShapes[1].fLength[2] = 0.532;
	CustomShapes[1].fRotate[2] = 148;
	CustomShapes[1].fLength[3] = 0.476;
	CustomShapes[1].fRotate[3] = 153;
	CustomShapes[1].fLength[4] = 1.003;
	CustomShapes[1].fRotate[4] = 77;
	CustomShapes[1].fLength[5] = 0.685;
	CustomShapes[1].fRotate[5] = 147;
	CustomShapes[1].fLength[6] = 0.657;
	CustomShapes[1].fRotate[6] = 137;

	CustomShapes[2].nCount = 8;
	CustomShapes[2].fLength[0] = 0.991;
	CustomShapes[2].fRotate[0] = 61;
	CustomShapes[2].fLength[1] = 0.574;
	CustomShapes[2].fRotate[1] = 106;
	CustomShapes[2].fLength[2] = 0.39;
	CustomShapes[2].fRotate[2] = 205;
	CustomShapes[2].fLength[3] = 0.779;
	CustomShapes[2].fRotate[3] = 100;
	CustomShapes[2].fLength[4] = 1.038;
	CustomShapes[2].fRotate[4] = 140;
	CustomShapes[2].fLength[5] = 0.517;
	CustomShapes[2].fRotate[5] = 105;
	CustomShapes[2].fLength[6] = 0.597;
	CustomShapes[2].fRotate[6] = 173;
	CustomShapes[2].fLength[7] = 0.626;
	CustomShapes[2].fRotate[7] = 49;

	CustomShapes[3].nCount = 8;
	CustomShapes[3].fLength[0] = 0.630;
	CustomShapes[3].fRotate[0] = 55;
	CustomShapes[3].fLength[1] = 0.626;
	CustomShapes[3].fRotate[1] = 146;
	CustomShapes[3].fLength[2] = 0.928;
	CustomShapes[3].fRotate[2] = 141;
	CustomShapes[3].fLength[3] = 0.409;
	CustomShapes[3].fRotate[3] = 84;
	CustomShapes[3].fLength[4] = 0.571;
	CustomShapes[3].fRotate[4] = 227;
	CustomShapes[3].fLength[5] = 1.089;
	CustomShapes[3].fRotate[5] = 97;
	CustomShapes[3].fLength[6] = 0.701;
	CustomShapes[3].fRotate[6] = 34;
	CustomShapes[3].fLength[7] = 0.699;
	CustomShapes[3].fRotate[7] = 198;

	CustomShapes[4].nCount = 7;
	CustomShapes[4].fLength[0] = 1.281;
	CustomShapes[4].fRotate[0] = 30;
	CustomShapes[4].fLength[1] = 0.677;
	CustomShapes[4].fRotate[1] = 150;
	CustomShapes[4].fLength[2] = 0.539;
	CustomShapes[4].fRotate[2] = 118;
	CustomShapes[4].fLength[3] = 1.149;
	CustomShapes[4].fRotate[3] = 90;
	CustomShapes[4].fLength[4] = 0.429;
	CustomShapes[4].fRotate[4] = 169;
	CustomShapes[4].fLength[5] = 0.457;
	CustomShapes[4].fRotate[5] = 143;
	CustomShapes[4].fLength[6] = 0.387;
	CustomShapes[4].fRotate[6] = 137;

	CustomShapes[5].nCount = 7;
	CustomShapes[5].fLength[0] = 1.324;
	CustomShapes[5].fRotate[0] = 24;
	CustomShapes[5].fLength[1] = 0.710;
	CustomShapes[5].fRotate[1] = 144;
	CustomShapes[5].fLength[2] = 0.550;
	CustomShapes[5].fRotate[2] = 105;
	CustomShapes[5].fLength[3] = 0.645;
	CustomShapes[5].fRotate[3] = 66;
	CustomShapes[5].fLength[4] = 0.675;
	CustomShapes[5].fRotate[4] = 178;
	CustomShapes[5].fLength[5] = 0.436;
	CustomShapes[5].fRotate[5] = 137;
	CustomShapes[5].fLength[6] = 0.386;
	CustomShapes[5].fRotate[6] = 152;

	CustomShapes[6].nCount = 7;
	CustomShapes[6].fLength[0] = 0.791;
	CustomShapes[6].fRotate[0] = 64;
	CustomShapes[6].fLength[1] = 1.426;
	CustomShapes[6].fRotate[1] = 113;
	CustomShapes[6].fLength[2] = 0.805;
	CustomShapes[6].fRotate[2] = 101;
	CustomShapes[6].fLength[3] = 0.829;
	CustomShapes[6].fRotate[3] = 130;
	CustomShapes[6].fLength[4] = 0.617;
	CustomShapes[6].fRotate[4] = 136;
	CustomShapes[6].fLength[5] = 0.673;
	CustomShapes[6].fRotate[5] = 147;
	CustomShapes[6].fLength[6] = 0.625;
	CustomShapes[6].fRotate[6] = 135;

	CustomShapes[7].nCount = 7;
	CustomShapes[7].fLength[0] = 0.559;
	CustomShapes[7].fRotate[0] = 43;
	CustomShapes[7].fLength[1] = 0.462;
	CustomShapes[7].fRotate[1] = 141;
	CustomShapes[7].fLength[2] = 0.648;
	CustomShapes[7].fRotate[2] = 226;
	CustomShapes[7].fLength[3] = 1.098;
	CustomShapes[7].fRotate[3] = 81;
	CustomShapes[7].fLength[4] = 0.802;
	CustomShapes[7].fRotate[4] = 111;
	CustomShapes[7].fLength[5] = 1.256;
	CustomShapes[7].fRotate[5] = 109;
	CustomShapes[7].fLength[6] = 0.562;
	CustomShapes[7].fRotate[6] = 176;


	sin54 = sin(54.0);
	while(1)
	{
		int nPointCount = 0;
		int nPointCountNeeded = 0;
		int i = 0;
		int nParamCount = 0;
		int j = 0;

		float fInputTemp;
		HWND hWnd = NULL;
		nShapeToSpawn = 0;

		printf( "输入边框类型:1,正方形 2,圆形 3,椭圆形 4,矩形 5,正方形+圆 6,圆+圆\n" );
		scanf( "%d", &nBoundType );
		nBoundType--;
		if ( nBoundType > btCircleWithOneCircle )
			nBoundType = btRect;
		if ( nBoundType < 0 )
			nBoundType = btRect;
		switch (nBoundType)
		{
		case btRect:
			printf( "输入边长：\n" );
			scanf( "%lf", &fWidth );
			fHeight = fWidth;
			break;
		case btCircle:
			printf( "输入半径：\n" );
			scanf( "%lf", &fWidth );
			fHeight = fWidth;
			break;
		case btEllipse:
			printf( "输入X轴半径：\n" );
			scanf( "%lf", &fWidth );
			printf( "输入Y轴半径：\n" );
			scanf( "%lf", &fHeight );
			break;
		case btPentagon:
			printf( "输入外接圆半径：\n" );
			scanf( "%lf", &fWidth );
			break;
		case btRect2:
			printf( "输入X边长：\n" );
			scanf( "%lf", &fWidth );
			printf( "输入Y边长：\n" );
			scanf( "%lf", &fHeight );
			break;
		case btRectWithOneCircle:
			printf( "输入边长：\n" );
			scanf( "%lf", &fWidth );
			fHeight = fWidth;
			printf( "输入圆半径：\n" );
			scanf( "%lf", &fCenterCircleRad );
			break;
		case btCircleWithOneCircle:
			{
				printf( "输入外圆半径：\n" );
				scanf( "%lf", &fWidth );
				fHeight = fWidth;
				printf( "输入内圆半径：\n" );
				scanf( "%lf", &fCenterCircleRad );
				break;
			}

		}



		if ( nBoundType == btRect2 )
		{
			do{
				printf( "输入X边上点的总数量(必须是2的倍数)：\n" );
				scanf( "%d", &nPointsForWidth );
				if ( (nPointsForWidth % 2) != 0 )		
					printf( "错误，必须是2的倍数\n" );
			}while( (nPointsForWidth % 2) != 0 );

			do{
				printf( "输入Y边上点的总数量(必须是2的倍数)：\n" );
				scanf( "%d", &nPointsForHeight );
				if ( (nPointsForHeight % 2) != 0 )		
					printf( "错误，必须是2的倍数\n" );
			}while( (nPointsForHeight % 2) != 0 );
			nPointCountNeeded += nPointsForWidth + nPointsForHeight;
		}
		else
		{
			do{
				printf( "输入边界上点的总数量(必须是4的倍数)：\n" );
				scanf( "%d", &nPointsForWidth );
				if ( (nPointsForWidth % 4) != 0 )		
					printf( "错误，必须是4的倍数\n" );
			}while( (nPointsForWidth % 4) != 0 );
			nPointsForHeight = nPointsForWidth;
			nPointCountNeeded += nPointsForWidth;
		}


		do{
			printf( "输入图形上点的总数量(必须是4的倍数)：\n" );
			scanf( "%d", &nPointsPerCircle );
			if ( (nPointsPerCircle % 4) != 0 )		
				printf( "错误，必须是4的倍数\n" );
		}while( (nPointsPerCircle % 4) != 0 );


		printf( "输入图形间距（不包括两个半径）：\n" );
		scanf( "%f", &fShapeSpace );


		if ( (nBoundType == btRectWithOneCircle) || (nBoundType == btCircleWithOneCircle) )
		{
			nShapeParamCount = 1;
			shapeparams[0].nCount = 1;
			shapeparams[0].nType = stCenteredCircleInRect;
			shapeparams[0].fParam1 = fCenterCircleRad;
			nPointCountNeeded += nPointsPerCircle;
		}
		else
		{
			do 
			{
				printf( "输入一共有几种圆：\n" );
				scanf( "%d", &j );
				if ( j > 300 )
					printf( "错误，半径最多300种\n" );
			}while( j > 300 );

			nParamCount = j;


			for ( i = 0; i < nParamCount; i++ )
			{
				printf( "输入第%d个半径：\n", i+1 );
				scanf( "%lf", &shapeparams[i + nShapeParamCount].fParam1 );

				printf( "输入第%d个半径下圆的数量：\n", i+1 );
				scanf( "%d", &shapeparams[i + nShapeParamCount].nCount );
				shapeparams[i + nShapeParamCount].nType = stCircle;
				nPointCountNeeded += shapeparams[i + nShapeParamCount].nCount * nPointsPerCircle;
				nShapeToSpawn += shapeparams[i + nShapeParamCount].nCount;
			}
			nShapeParamCount += j;

			do 
			{
				printf( "输入一共有几种椭圆：\n" );
				scanf( "%d", &j );
				if ( j > 300 )
					printf( "错误，半径最多300种\n" );
			}while( j > 300 );

			nParamCount = j;




			for ( i = 0; i < nParamCount; i++ )
			{
				printf( "输入第%d个X半径：\n", i+1 );
				scanf( "%lf", &shapeparams[i + nShapeParamCount].fRadX );
				printf( "输入第%d个Y半径(输入-1表示从X轴半径的0.1~0.9随机取)：\n", i+1 );
				scanf( "%lf", &shapeparams[i + nShapeParamCount].fRadY );

				printf( "输入第%d个半径下椭圆的数量：\n", i+1 );
				scanf( "%d", &shapeparams[i + nShapeParamCount].nCount );
				shapeparams[i + nShapeParamCount].nType = stEllipse;
				nPointCountNeeded += shapeparams[i + nShapeParamCount].nCount * nPointsPerCircle;
				nShapeToSpawn += shapeparams[i + nShapeParamCount].nCount;
			}
			nShapeParamCount += j;
			do 
			{
				printf( "输入一共有几种五边形：\n" );
				scanf( "%d", &j );
				if ( j > 300 )
					printf( "错误，最多30种\n" );
			}while( j > 300 );

			nParamCount = j;

			for ( i = 0; i < nParamCount; i++ )
			{
				printf( "输入第%d个半径：\n", i+1 );
				scanf( "%lf", &shapeparams[i + nShapeParamCount].fRad );

				printf( "输入第%d个半径下五边形的数量：\n", i+1 );
				scanf( "%d", &shapeparams[i + nShapeParamCount].nCount );
				shapeparams[i + nShapeParamCount].nType = stPentagon;
				nPointCountNeeded += shapeparams[i + nShapeParamCount].nCount * nPointsPerCircle/5*5;
				nShapeToSpawn += shapeparams[i + nShapeParamCount].nCount;
			}
			nShapeParamCount += j;
		}

		do 
		{
			printf( "输入一共有几多边形：\n" );
			scanf( "%d", &j );
			if ( j > 300 )
				printf( "错误，最多300种\n" );
		}while( j > 300 );

		nParamCount = j;


		for ( i = 0; i < nParamCount; i++ )
		{
			printf( "输入第%d个多边形类型（0：随机，1~8：指定某个多边形）：\n", i+1 );
			scanf( "%D", &shapeparams[i + nShapeParamCount].nPolyType );
				
			printf( "输入第%d个半径：\n", i+1 );
			scanf( "%lf", &shapeparams[i + nShapeParamCount].fParam1 );

			printf( "输入第%d个半径下多边形的数量：\n", i+1 );
			scanf( "%d", &shapeparams[i + nShapeParamCount].nCount );
			shapeparams[i + nShapeParamCount].nType = stFromLib;
			nPointCountNeeded += shapeparams[i + nShapeParamCount].nCount * nPointsPerCircle;
			nShapeToSpawn += shapeparams[i + nShapeParamCount].nCount;
		}
		nShapeParamCount += j;


		if ( Points != NULL )
		{
			delete [] Points ;
		}

		Points = new Point_T[ nPointCountNeeded  ];
		Process();//处理数据
		OutPut();

		//-开始----创建窗口，绘制数据
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

		if (!hWnd)
		{
			return FALSE;
		}

		ShowWindow(hWnd, SW_NORMAL);
		UpdateWindow(hWnd);



		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0))
		{

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//-结束----创建窗口，绘制数据
	}
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_MOUSEWHEEL:
		{
			if ( MK_CONTROL == (wParam & 0xffff) )
				fScale += ((short)(wParam>>16))/120*8;
			else
				fScale += ((short)(wParam>>16))/120;
			if ( fScale < 1.0 )
				fScale = 1.0;
			if ( fScale > 1000.0 )
				fScale = 1000.0;
			InvalidateRect( hWnd, NULL, TRUE );

		}
		break;
	case WM_MBUTTONDOWN:
		{
			nMButtonDown = 1;
			GetCursorPos( &ptMDownPoint );
			GetCapture();
			fMDOffsetX = fOffsetX ;
			fMDOffsetY = fOffsetY;
		}
		break;
	case WM_MBUTTONUP:
		{
			nMButtonDown = 0;
			ReleaseCapture();
		}
		break;
	case WM_MOUSEMOVE:
		{
			POINT ptCurrent;

			if ( nMButtonDown == 1 )
			{
				GetCursorPos( &ptCurrent );
				fOffsetX = fMDOffsetX + (ptCurrent.x - ptMDownPoint.x);
				fOffsetY = fMDOffsetY + (ptCurrent.y - ptMDownPoint.y);////
				InvalidateRect( hWnd, NULL, TRUE );
			}

		}
		break;
	case WM_KEYDOWN:
		{
			if ( wParam == 'H' )
			{
				RECT rc;
				fScale = 1.0;
				GetClientRect( hWnd, &rc );
				fOffsetX = (rc.right - rc.left)/2;
				fOffsetY = (rc.bottom - rc.top)/2;
				InvalidateRect( hWnd, NULL, TRUE );
			}
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawGraphic( hdc );
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


double MyAbs( double in )
{
	if ( in < 0 )
		in = -in;
	return in;
}

BOOL SimpleCollisionTest( struct Shape_T *shp1, struct Shape_T *shp2)
{
	double r1 = 0;
	double r2 = 0;

	if ( shp1->nType != stEllipse )
		r1 = shp1->fRad;
	else
	{
		r1 = shp1->fRadX;
		if ( r1 < shp1->fRadY )
			r1 = shp1->fRadY;
	}

	if ( shp2->nType != stEllipse )
		r2 = shp2->fRad;
	else
	{
		r2 = shp2->fRadX;
		if ( r2 < shp2->fRadY )
			r2 = shp2->fRadY;
	}

	r1 += fShapeSpace;

	if ( ( shp2->vPos.x-shp1->vPos.x)*( shp2->vPos.x-shp1->vPos.x) + ( shp2->vPos.y-shp1->vPos.y)*( shp2->vPos.y-shp1->vPos.y) 
		< (r1+r2)*(r1+r2) )
		return TRUE;
	else
		return FALSE;
}

enum IntersectType CirecleCirecleCollisionTest( struct Shape_T *circle1, struct Shape_T *circle2)
{
	if ( ( circle2->vPos.x-circle1->vPos.x)*( circle2->vPos.x-circle1->vPos.x) + ( circle2->vPos.y-circle1->vPos.y)*( circle2->vPos.y-circle1->vPos.y) 
		< (circle2->fParam1+circle1->fParam1)*(circle2->fParam1+circle1->fParam1) )
		return itIntersect;
	else
		return itOutSide;
}
enum IntersectType CirecleEllipseCollisionTest( struct Shape_T *circle, struct Shape_T *ellipse )
{
	double ellimaxrad = ellipse->fRadX;
	if ( ellimaxrad < ellipse->fRadY )
		ellimaxrad = ellipse->fRadY;

	if ( ( circle->vPos.x-ellipse->vPos.x)*( circle->vPos.x-ellipse->vPos.x) + ( circle->vPos.y-ellipse->vPos.y)*( circle->vPos.y-ellipse->vPos.y) 
	> (circle->fRad+ellimaxrad)*(circle->fRad+ellimaxrad) )
	return itOutSide;

	return itInside;


}

enum IntersectType CireclePentagonCollisionTest( struct Shape_T *circle, struct Shape_T *ellipse )
{
	return itOutSide;
}


BOOL TestCollision( struct Shape_T *shp1, struct Shape_T *shp2)//判断是否重叠
{
	return SimpleCollisionTest( shp1, shp2 );
	/*
	if ( shp1->nType == stCircle )
	{
	if ( shp2->nType == stCircle )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	if ( shp2->nType == stPentagon )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	if ( shp2->nType == stEllipse )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	}

	if ( shp1->nType == stPentagon )
	{
	if ( shp2->nType == stCircle )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	if ( shp2->nType == stPentagon )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	if ( shp2->nType == stEllipse )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	}
	if ( shp1->nType == stEllipse )
	{
	if ( shp2->nType == stCircle )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	if ( shp2->nType == stPentagon )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	if ( shp2->nType == stEllipse )
	{
	return SimpleCollisionTest( shp1, shp2 );
	}
	}*/
}

//enum IntersectType TestCircleRectangle( struct Shape_T *pCircle, struct Vector2_T vTopLeft, struct Vector2_T vBottomRight )
//{
//	if ( ( vTopLeft.x + pCircle->fParam1 < pCircle->vPos.x ) &&( vBottomRight.x - pCircle->fParam1 > pCircle->vPos.x ) && 
//			(vTopLeft.y + pCircle->fParam1 < pCircle->vPos.y ) &&( vBottomRight.y - pCircle->fParam1 > pCircle->vPos.y ) )
//	{
//		return itInside;//圆在矩形内
//	}
//
//	if ( )
//}

BOOL TestCollisionAll( struct Shape_T *shp )
{
	int i = 0;
	//int splitx,splity;
	//BOOL bCollided = FALSE;
	//struct Shape_T* p;
	//splitx = (int)(shp->vPos.x / nSplitLevel);
	//splity = (int)(shp->vPos.y / nSplitLevel);

	//p = sectors[splitx][splity].pShapes;
	//
	//while ( p != NULL )
	//{
	//	if ( TestCollision( shp, p ) == TRUE )
	//		return TRUE;
	//	p = p->pNext;
	//}
	for ( i = 0; i < nShapeCount; i++ )
	{
		if ( TestCollision( shp, &shapes[i] ) == TRUE )
			return TRUE;
	}
	return FALSE;
}

void Process()
{
	int i = 0;
	for ( i = 0; i < nShapeParamCount; i++ )
	{
		SpawnShape( &shapeparams[i] );
	}
}

void GetSpawnPos( struct ShapeParam_T* sparam, struct Vector2_T *vPos )
{

	double maxx,maxy;
	double minx,miny;

	double rad;

	if ( sparam->nType == stEllipse )
	{
		rad = sparam->fRadX;
		if ( rad < sparam->fRadY )
			rad = sparam->fRadY;
	}
	else
	{
		rad = sparam->fRad;
	}

	rad += fShapeSpace;

	switch (nBoundType)
	{
	case btRect:
		minx = miny = rad;
		maxx = maxy = fWidth - rad;
		vPos->x = MyRandom( maxx, minx );
		vPos->y = MyRandom( maxy, miny );
		break;
	case btCircle:
		{
			double fLen = MyRandom( 0, fWidth - rad );
			double rot = MyRandom( 0, pi*2);

			vPos->x = cos(rot) * fLen;
			vPos->y = sin(rot) * fLen;
		}
		break;
	case btEllipse:
		{
			double fLenx = MyRandom( 0, fWidth - rad );
			double fLeny = MyRandom( 0, fHeight - rad );
			double rot = MyRandom( 0, pi*2);
			vPos->x = cos(rot) * fLenx;
			vPos->y = sin(rot) * fLeny;
		}
		break;
	case btPentagon:
		{
			//不需要做
			//int side = MyRandom( 0, 2 );
			//if ( side == 0 )
			//{

			//}
			//double fLen = MyRandom( 0, fWidth - rad / sin54 );
			//double rot = MyRandom( 0, pi*2);
			//vPos->x = cos(rot) * fLen;
			//vPos->y = sin(rot) * fLen;
		}
		break;
	case btRect2:
		{
			minx = miny = rad;
			maxx = fWidth - rad;
			maxy = fHeight - rad;
			vPos->x = MyRandom( maxx, minx );
			vPos->y = MyRandom( maxy, miny );
		}
		break;

	case btRectWithOneCircle:
		{
			vPos->x = fWidth /2.0f;
			vPos->y = fHeight / 2.0f;
		}
		break;
	case btCircleWithOneCircle:
		{
			vPos->x = 0;
			vPos->y = 0;
		}

		break;
	}
}

void SpawnShape( struct ShapeParam_T* sparam )
{
	int i = 0;
	int nreportstep = nShapeToSpawn/1000;
	int splitx,splity;
	if ( nreportstep <= 0 )
		nreportstep = 1;
	for ( i = 0; i < sparam->nCount; i++ )
	{
		switch ( sparam->nType )
		{
		case stCircle:
			{
				do
				{
					GetSpawnPos( sparam, &shapes[nShapeCount].vPos );
					shapes[nShapeCount].nType = stCircle;
					shapes[nShapeCount].fParam1 = sparam->fParam1;
					if ( GetKeyState( 'A' ) & 0x80 )
						goto end;
				}
				while ( TestCollisionAll( &shapes[nShapeCount] )== TRUE );
				AddShape( &shapes[i] );
			}
			break;
		case 	stEllipse:
			{
				do
				{
					GetSpawnPos( sparam, &shapes[nShapeCount].vPos );
					shapes[nShapeCount].nType = stEllipse;
					shapes[nShapeCount].fParam1 = sparam->fParam1;
					shapes[nShapeCount].fParam2 = sparam->fParam2;
					if ( shapes[nShapeCount].fParam2 < 0 )
					{
						shapes[nShapeCount].fParam2 = shapes[nShapeCount].fParam1 * MyRandom( 0.9, 0.1 );
					}

					shapes[nShapeCount].fRotate = MyRandom( 0, pi*2 );
					if ( GetKeyState( 'A' ) & 0x80 )
						goto end;
				}
				while ( TestCollisionAll( &shapes[nShapeCount] )== TRUE );
				AddShape( &shapes[i] );
			}
			break;
		case stPentagon:
			{
				do
				{
					GetSpawnPos( sparam, &shapes[nShapeCount].vPos );
					shapes[nShapeCount].nType = stPentagon;
					shapes[nShapeCount].fParam1 = sparam->fParam1;
					shapes[nShapeCount].fRotate = MyRandom( 0, pi*2 );
					if ( GetKeyState( 'A' ) & 0x80 )
						goto end;
				}
				while ( TestCollisionAll( &shapes[nShapeCount] )== TRUE );
				AddShape( &shapes[i] );
			}
			break;
		case stCenteredCircleInRect:
			{

				GetSpawnPos( sparam, &shapes[nShapeCount].vPos );
				shapes[nShapeCount].nType = stCircle;
				shapes[nShapeCount].fParam1 = sparam->fParam1;
				if ( GetKeyState( 'A' ) & 0x80 )
					goto end;
				AddShape( &shapes[i] );
			}
		case stFromLib:
			{
				do
				{
					GetSpawnPos( sparam, &shapes[nShapeCount].vPos );
					shapes[nShapeCount].nType = stFromLib;
					shapes[nShapeCount].fParam1 = sparam->fParam1;
					if ( sparam->nPolyType <= 0 )
						shapes[nShapeCount].iParam1 = (int)(MyRandom( nCustomShapeCount, 0 ));
					else
					{
						shapes[nShapeCount].iParam1 =sparam->nPolyType -1;
					}
					shapes[nShapeCount].fRotate = MyRandom( 0, pi*2 );
					if ( GetKeyState( 'A' ) & 0x80 )
						goto end;
				}
				while ( TestCollisionAll( &shapes[nShapeCount] )== TRUE );
				AddShape( &shapes[i] );
			}
		}


		if ( nShapeCount % nreportstep  == 0 )
			printf( "生成%d个图形中的第%d个图形\n", nShapeToSpawn, nShapeCount+1 );		

		nShapeCount++;
	}
end:
	printf( "生成中断，共生成%d个图形\n", nShapeCount );
}


void DrawCircle( struct Shape_T* circle, HDC dc )
{

	double x,y,rad;
	x = circle->vPos.x * fScale + fOffsetX;
	y = circle->vPos.y * -fScale + fOffsetY;
	rad = circle->fRad * fScale;

	Arc( dc, x - rad, y - rad, x + rad, y + rad, x - rad, y - rad, x + rad, y + rad );
	Arc( dc, x - rad, y - rad, x + rad, y + rad, x + rad, y + rad, x - rad, y - rad );
}

void DrawEllipse( struct Shape_T* ellipse, HDC dc )
{
	XFORM xf;

	double radx, rady;
	double sint,cost;
	radx = ellipse->fRadX * fScale;
	rady = ellipse->fRadY * fScale;
	xf.eDx = ellipse->vPos.x * fScale +fOffsetX;
	xf.eDy = ellipse->vPos.y * -fScale +fOffsetY;
	sint = sin( ellipse->fRotate );
	cost = cos( ellipse->fRotate );
	xf.eM11 = cost;
	xf.eM12 = sint;
	xf.eM21 = -sint;
	xf.eM22 = cost;
	SetWorldTransform( dc,  &xf );

	Arc( dc, - radx, - rady, radx, rady, -radx, - rady, radx, rady );
	Arc( dc, - radx, - rady, radx, rady, radx, rady, - radx, - rady );
	//Arc( dc, x - radx, y - rady, x + radx, y + rady, x - radx, y - rady, x + radx, y + rady );
	//Arc( dc, x - radx, y - rady, x + radx, y + rady, x + radx, y + rady, x - radx, y - rady );
}

void DrawPentagon( struct Shape_T* pentagon, HDC dc )
{
	int j = 0;
	double x,y,rad; 
	XFORM xf;
	double sint,cost;
	double fStep = pi*2.0f/5;
	POINT points[6];

	xf.eDx = pentagon->vPos.x * fScale +fOffsetX;
	xf.eDy = pentagon->vPos.y * -fScale +fOffsetY;
	sint = sin( pentagon->fRotate );
	cost = cos( pentagon->fRotate );
	xf.eM11 = cost;
	xf.eM12 = sint;
	xf.eM21 = -sint;
	xf.eM22 = cost;
	SetWorldTransform( dc,  &xf );

	rad = pentagon->fRad;




	for ( j = 0; j < 5; j++ )
	{
		points[j].x = cos((fStep * j) ) * rad  * fScale;
		points[j].y = sin((fStep * j) ) * rad  * fScale;
	}


	points[5] = points[0];

	Polyline( dc, points, 6 );
}

void DrawPoly( struct Shape_T* poly, HDC dc )
{
	struct Shape_T pptest;
	int j = 0;
	double x,y,rad; 
	XFORM xf;
	double sint,cost;
	POINT points[100];
	struct Vector2_T lastpt;

	double Rotate = 0.0/180.0 * pi ;
	double rot = 0;

	lastpt.x = 0;
	lastpt.y = 0;

	xf.eDx = poly->vPos.x * fScale +fOffsetX;
	xf.eDy = poly->vPos.y * -fScale +fOffsetY;
	sint = sin( poly->fRotate );
	cost = cos( poly->fRotate );
	xf.eM11 = cost;
	xf.eM12 = sint;
	xf.eM21 = -sint;
	xf.eM22 = cost;
	SetWorldTransform( dc,  &xf );


	for ( j = 0; j < CustomShapes[poly->iParam1].nCount; j++ )
	{		
		struct Vector2_T temppt;

		Rotate += (CustomShapes[poly->iParam1].fRotate[j] + 180) ;

		while( Rotate > 360.0 )
		{
			Rotate -= 360.0;
		}
		rot = Rotate /180.0 * pi;
		temppt.x = lastpt.x + cos( rot ) * CustomShapes[poly->iParam1].fLength[j] * poly->fRad  * fScale;
		points[j].y = temppt.y = lastpt.y + sin( rot ) * CustomShapes[poly->iParam1].fLength[j] * poly->fRad * fScale;
		points[j].x = temppt.x + poly->fRad * fScale;
		lastpt.x = temppt.x ;
		lastpt.y = temppt.y;
		//points[j].x += poly->fRad * fScale;
	}


	points[CustomShapes[poly->iParam1].nCount] = points[0];

	Polyline( dc, points, CustomShapes[poly->iParam1].nCount + 1 );	
	
	
}

void DrawGraphic( HDC dc )
{


	int i = 0;
	int k = 0;
	int nCircleCnt = 0;
	XFORM xf;
	SetGraphicsMode( dc, GM_ADVANCED );
	switch ( nBoundType )
	{ 
	case btRect:
	case btRectWithOneCircle:
	case btRect2:
		{
			Rectangle( dc, 0+fOffsetX,0+fOffsetY,fWidth*fScale+fOffsetX, fHeight*-fScale+fOffsetY );//画正方形
		}
		break;
	case btCircleWithOneCircle:
	case btCircle:
	case btEllipse:
		{
			double radx = fWidth * fScale;
			double rady = fHeight * - fScale;
			Arc( dc, - radx+fOffsetX, - rady+fOffsetY, radx+fOffsetX, rady+fOffsetY, -radx+fOffsetX, - rady+fOffsetY, radx+fOffsetX, rady+fOffsetY );
			Arc( dc, - radx+fOffsetX, - rady+fOffsetY, radx+fOffsetX, rady+fOffsetY, radx+fOffsetX, rady+fOffsetY, - radx+fOffsetX, - rady+fOffsetY );

		}
	}


	for ( i = 0; i < nShapeCount; i++ )//画所有的图形
	{
		xf.eDx = xf.eDy = 0;
		xf.eM11 = xf.eM22 = 1.0f;
		xf.eM12 = xf.eM21 = 0.0f;
		SetWorldTransform( dc, &xf );
		switch ( shapes[i].nType )
		{
		case stCircle:
			{
				DrawCircle( &shapes[i], dc );
			}
			break;

		case stEllipse:
			{
				DrawEllipse( &shapes[i], dc );
			}
			break;
		case stPentagon:
			{
				DrawPentagon( &shapes[i], dc );
			}
			break;
		case stFromLib:
			{
				DrawPoly( &shapes[i], dc );
			}
			break;
		}
	}


	xf.eDx = xf.eDy = 0;
	xf.eM11 = xf.eM22 = 1.0f;
	xf.eM12 = xf.eM21 = 0.0f;
	SetWorldTransform( dc, &xf );
	SetGraphicsMode( dc, GM_COMPATIBLE );
	for ( i = 0; i < nPointCount; i++ )
	{
		struct Shape_T sp;
		sp.vPos.x = Points[i].x;
		sp.vPos.y = Points[i].y;
		sp.fRad = 2.0/fScale;
		DrawCircle( &sp, dc );
	}

}

void OutPut()
{

	FILE* pf;
	int i = 0;
	int j = 0;
	int k = 0;
	int nCircleCnt = 0;
	int nPointsPerEdgeX = 0;
	int nPointsPerEdgeY = 0;
	int start = 0;
	int nPointsWrite = 0;

	switch ( nBoundType )
	{
	case btRect2:
		nPointsPerEdgeX = nPointsForWidth / 2;
		nPointsPerEdgeY = nPointsForHeight / 2;
		break;
	case btRectWithOneCircle:
	case btRect:
		nPointsPerEdgeX = nPointsPerEdgeY = nPointsForWidth / 4;
		break;
	}

	nPointCount = 0;

	switch ( nBoundType )
	{
	case btRectWithOneCircle:
	case btRect:
	case btRect2:
		{

			//计算正方形下边的点( X+ )
			for ( i = 0; i < nPointsPerEdgeX; i++ )
			{
				Points[nPointCount].x = i * fWidth/nPointsPerEdgeX;
				Points[nPointCount].y = 0;
				Points[nPointCount].nSelfID = nPointCount + 1;
				Points[nPointCount].nNextID = Points[nPointCount].nSelfID + 1;

				nPointCount++;
			}
			//计算正方形右边的点( Y+ )
			for ( i = 0; i < nPointsPerEdgeY; i++ )
			{
				Points[nPointCount].x = fWidth;
				Points[nPointCount].y = i * fHeight/nPointsPerEdgeY;
				Points[nPointCount].nSelfID = nPointCount + 1;
				Points[nPointCount].nNextID = Points[nPointCount].nSelfID + 1;
				nPointCount++;
			}
			//计算正方形上边的点( X- )
			for ( i = nPointsPerEdgeX; i > 0; i-- )
			{
				Points[nPointCount].x = i * fWidth/nPointsPerEdgeX;
				Points[nPointCount].y = fHeight;
				Points[nPointCount].nSelfID = nPointCount + 1;
				Points[nPointCount].nNextID = Points[nPointCount].nSelfID + 1;
				nPointCount++;
			}
			//计算正方形左边的点
			for ( i = nPointsPerEdgeY; i > 0; i-- )
			{
				Points[nPointCount].x = 0;
				Points[nPointCount].y = i * fHeight/nPointsPerEdgeY;
				Points[nPointCount].nSelfID = nPointCount + 1;
				Points[nPointCount].nNextID = Points[nPointCount].nSelfID + 1;
				nPointCount++;
			}

			//正方形最后一个点,特殊处理
			Points[nPointCount - 1].nNextID = 1;
		}
		break;
	case btCircleWithOneCircle:
	case btCircle:
	case btEllipse:
		{
			double radx,rady;

			double fStep = pi*2.0/nPointsForWidth;
			double fStepFix = fStep/2.0;

			if ( nBoundType == btCircle )
			{
				radx = rady = fWidth;
			}
			else
			{
				radx = fWidth;
				rady = fHeight;
			}


			for ( j = 0; j < nPointsForWidth; j++ )
			{
				Points[nPointCount].x = cos((fStep * j - fStepFix) ) * radx;
				Points[nPointCount].y = sin((fStep * j - fStepFix) ) * rady;
				Points[nPointCount].nSelfID = nPointCount + 1;
				Points[nPointCount].nNextID = nPointCount + 2;
				if  (j == nPointsForWidth-1 )
				{
					//Points[nPointCount].nSelfID = 1;
					Points[nPointCount].nNextID = 1;
				}
				nPointCount++;
			}

		}

	}

	//计算图形上的点(逆时针)
	for ( i = 0; i < nShapeCount; i++ )//画所有的图形
	{
		switch ( shapes[i].nType )
		{
		case stCircle:
			{
				double fStep = pi*2.0/nPointsPerCircle;
				double fStepFix = fStep/2.0;
				int    nFirst = nPointCount;
				for ( j = 0; j < nPointsPerCircle; j++ )
				{
					Points[nPointCount].x = cos((fStep * j - fStepFix) ) * shapes[i].fRad + shapes[i].vPos.x;
					Points[nPointCount].y = sin((fStep * j - fStepFix) ) * shapes[i].fRad + shapes[i].vPos.y;
					Points[nPointCount].nSelfID = nPointCount + 2;
					Points[nPointCount].nNextID = nPointCount + 1;
					if  (j == nPointsPerCircle-1 )
					{
						Points[nPointCount].nSelfID = nFirst + 1;
						Points[nPointCount].nNextID = nPointCount + 1;
					}
					nPointCount++;
				}
			}
			break;
		case stEllipse:
			{
				double fStep = pi*2.0/nPointsPerCircle;
				double fStepFix = fStep/2.0;
				int    nFirst = nPointCount;
				for ( j = 0; j < nPointsPerCircle; j++ )
				{
					double tmpx,tmpy;
					tmpx = cos((fStep * j - fStepFix) ) * shapes[i].fRadX;
					tmpy = sin((fStep * j - fStepFix) ) * shapes[i].fRadY;

					Points[nPointCount].x = shapes[i].vPos.x + cos( -shapes[i].fRotate ) * tmpx - sin( -shapes[i].fRotate )*tmpy;
					Points[nPointCount].y = shapes[i].vPos.y + sin( -shapes[i].fRotate ) * tmpx + cos( -shapes[i].fRotate )*tmpy;
					Points[nPointCount].nSelfID = nPointCount + 2;
					Points[nPointCount].nNextID = nPointCount + 1;
					if  (j == nPointsPerCircle-1 )
					{
						Points[nPointCount].nSelfID = nFirst + 1;
						Points[nPointCount].nNextID = nPointCount + 1;
					}
					nPointCount++;
				}

			}
			break;
		case stPentagon:
			{
				int step = 0;
				int pointsperline = 0;
				double step2 = 0;
				struct Vector2_T pentagonpts[6];
				int    nFirst = nPointCount;

				if ( nPointsPerCircle < 5 )
					pointsperline = 0;
				else
					pointsperline = nPointsPerCircle / 5;

				step2 = pi*2.0f/5;

				for ( j = 0; j < 5; j++ )
				{
					double tmpx,tmpy;
					tmpx = cos((step2 * j ) ) * shapes[i].fRad;
					tmpy = sin((step2 * j ) ) * shapes[i].fRad;
					pentagonpts[j].x = shapes[i].vPos.x + cos( -shapes[i].fRotate ) * tmpx - sin( -shapes[i].fRotate )*tmpy;
					pentagonpts[j].y = shapes[i].vPos.y + sin( -shapes[i].fRotate ) * tmpx + cos( -shapes[i].fRotate )*tmpy;

				}
				pentagonpts[5] = pentagonpts[0];

				for ( j = 0; j < 5; j++ )
				{
					for ( step = 0; step < pointsperline; step++ )
					{
						double factor = (double)(step)/pointsperline;
						Points[nPointCount].x = pentagonpts[j].x + (pentagonpts[j+1].x - pentagonpts[j].x) * factor;
						Points[nPointCount].y = pentagonpts[j].y + (pentagonpts[j+1].y - pentagonpts[j].y) * factor;
						Points[nPointCount].nSelfID = nPointCount + 2;
						Points[nPointCount].nNextID = nPointCount + 1;
						nPointCount++;
					}
				}
				if ( pointsperline > 0 )
				{
					Points[nPointCount - 1].nSelfID = nFirst + 1;
					Points[nPointCount - 1].nNextID = nPointCount ;
				}
			}
			break;
		case stFromLib:
				{

				int step = 0;
				int pointsperline = 0;
				double step2 = 0;
				struct Vector2_T polypoints[1000];
				struct Vector2_T lastpt;
				int    nFirst = nPointCount;
				double  Rotate = 0;
				XFORM xf;
				double sint = sin( shapes[i].fRotate );
				double cost = cos( shapes[i].fRotate );
				xf.eM11 = cost;
				xf.eM12 = sint;
				xf.eM21 = -sint;
				xf.eM22 = cost;

				lastpt.x = lastpt.y = 0;

				if ( nPointsPerCircle < CustomShapes[shapes[i].iParam1].nCount )
					pointsperline = 0;
				else
					pointsperline = nPointsPerCircle / CustomShapes[shapes[i].iParam1].nCount;

				step2 = pi*2.0f/5;

				Rotate = ((shapes[i].fRotate)/pi*180);

				for ( j = 0; j < CustomShapes[shapes[i].iParam1].nCount; j++ )
				{	
					double rot = 0;
					
					Rotate += (CustomShapes[shapes[i].iParam1].fRotate[j] + 180) ;

					while( Rotate > 360.0 )
					{
						Rotate -= 360.0;
					}
					rot = Rotate /180.0 * pi;
					polypoints[j].x = lastpt.x + cos( rot ) * CustomShapes[shapes[i].iParam1].fLength[j] * shapes[i].fRad ;
					polypoints[j].y = lastpt.y + sin( rot ) * CustomShapes[shapes[i].iParam1].fLength[j] * shapes[i].fRad ;
					lastpt = polypoints[j];
					polypoints[j].x  = (polypoints[j].x + shapes[i].vPos.x ) *  fScale;// * (polypoints[j].x * xf.eM11 + polypoints[j].y * xf.eM12 + shapes[i].vPos.x);
					polypoints[j].y  = ((polypoints[j].y * -1) + shapes[i].vPos.y) *  fScale;// * (polypoints[j].x * xf.eM21 + polypoints[j].y * xf.eM22 + shapes[i].vPos.y);

					polypoints[j].x += ( shapes[i].fRad * fScale * xf.eM11 );
					polypoints[j].y += ( shapes[i].fRad * fScale * xf.eM21 );
					
				}

				polypoints[j] = polypoints[0];

				polyrecordmap[shapes[i].iParam1][shapes[i].fRad].startpointid.push_back( nFirst);
				for ( j = 0; j < CustomShapes[shapes[i].iParam1].nCount; j++ )
				{
					for ( step = 0; step < pointsperline; step++ )
					{
						double factor = (double)(step)/pointsperline;
						Points[nPointCount].x = polypoints[j].x + (polypoints[j+1].x - polypoints[j].x) * factor;
						Points[nPointCount].y = polypoints[j].y + (polypoints[j+1].y - polypoints[j].y) * factor;
						Points[nPointCount].nSelfID = nPointCount + 2;
						Points[nPointCount].nNextID = nPointCount + 1;
						nPointCount++;
					}
				}
				polyrecordmap[shapes[i].iParam1][shapes[i].fRad].endpointid.push_back( nPointCount);
				if ( pointsperline > 0 )
				{
					Points[nPointCount - 1].nSelfID = nFirst + 1;
					Points[nPointCount - 1].nNextID = nPointCount ;
				}
			}
			break;
		}
	}
	if ( nPointCount > nPointCountNeeded )
		GetTickCount();

	//打开文件
	pf = fopen( "output.txt", "w" );
	if ( pf == NULL )
		return;

	//写入所有点
	for ( i = 0; i < nPointCount; i++ )
	{
		fprintf( pf, "%d %.16f %.16f\r\n", i+1, Points[i].x, Points[i].y );
	}

	//写入点编号
	for ( i = 0; i < nPointCount; i++ )
	{
		fprintf( pf, "%d %d %d\r\n", i+1, Points[i].nSelfID, Points[i].nNextID );
	}


	fclose(pf);

	pf = fopen( "output_noID.txt", "w" );
	if ( pf == NULL )
		return;

	//写入所有点
	for ( i = 0; i < nPointCount; i++ )
	{
		fprintf( pf, "%.16f %.16f\r\n", Points[i].x, Points[i].y );
	}

	//写入点编号
	for ( i = 0; i < nPointCount; i++ )
	{
		fprintf( pf, "%d %d\r\n", Points[i].nSelfID, Points[i].nNextID );
	}

	

	fclose(pf);

	//写入多边形统计
	pf = fopen( "Poly_output.txt", "w" );
	std::map< int , std::map<float, PolyRecord_T> >::iterator it;
	for ( it = polyrecordmap.begin(); it != polyrecordmap.end() ; ++it )
	{
		fprintf( pf, "类型为：%d  " , it->first+1  );
		std::map<float, PolyRecord_T>::iterator it2;
		for ( it2 = it->second.begin(); it2 != it->second.end(); ++it2 )
		{
			fprintf( pf, "半径为为：%f  \r\n" , it2->first  );
			for ( int i=0; i< it2->second.startpointid.size(); i++ )
			{
				fprintf( pf, "%d %d \r\n", it2->second.startpointid[i], it2->second.endpointid[i] );
			}
		}
	}
	fclose(pf);
}
