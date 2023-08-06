
// cell_grabDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "cell_grab.h"
#include "cell_grabDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT ThreadGrab(LPVOID lParam)
{
	Ccell_grabDlg *pView = (Ccell_grabDlg *)lParam;

	DWORD dwRet = 0;

	while (1) {
		dwRet = WaitForSingleObject((HANDLE)pView->EV_grab, INFINITE);
		if (dwRet == WAIT_FAILED) {
			return 0;
		}
		else if (dwRet == WAIT_ABANDONED) {
			ResetEvent(pView->EV_grab);
		}
		else if (dwRet == WAIT_TIMEOUT) {
			continue;
		}
		else {
			pView->DoGrab();
		}
		Sleep(1);
	}
	return 0;
}



// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Ccell_grabDlg 대화 상자



Ccell_grabDlg::Ccell_grabDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CELL_GRAB_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Ccell_grabDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_IMAGE_1, _grab_full_buf);
	DDX_Control(pDX, IDC_IMAGE_2, _grab_tiny);
	DDX_Control(pDX, IDC_IMAGE_3, _inspect);

}

BEGIN_MESSAGE_MAP(Ccell_grabDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &Ccell_grabDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, &Ccell_grabDlg::OnBnClickedOk2)
END_MESSAGE_MAP()


// Ccell_grabDlg 메시지 처리기

BOOL Ccell_grabDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	_glogger = new gLogger("suslog", std::string("D:\\Log\\") + std::string("syslog.txt"), false, 23, 59);

	AfxBeginThread(ThreadGrab, this, THREAD_PRIORITY_HIGHEST, 0);

	_grab_full_buf.gCreate(WIDTH, GRAB_BUF_SIZE, 8);
	_grab_tiny.gCreate(WIDTH, HEIGHT_GRAB, 8);
	_inspect.gCreate(WIDTH, HEIGHT, 8);

	_grab_buf = new unsigned char[WIDTH * GRAB_BUF_SIZE];
	memset(_grab_buf, 0, sizeof(unsigned char) * WIDTH * GRAB_BUF_SIZE);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

int Ccell_grabDlg::GetCurrentGrabIdx()
{
	return _grab_idx;
}

void Ccell_grabDlg::SetCurrentGrabIdx(int current_grab_idx)
{
	_grab_idx = current_grab_idx;
}

int Ccell_grabDlg::GetRemainTabIdx()
{
	return _remain_tab;
}
void Ccell_grabDlg::SetRemainTabIdx(int remain_tab_idx)
{
	_remain_tab = remain_tab_idx;
}

bool Ccell_grabDlg::GetSearchTab()
{
	return _search_tab;
}

void Ccell_grabDlg::SetSearchTab(bool search_tab)
{
	_search_tab = search_tab;
}

int Ccell_grabDlg::GetSearchTabEdgePos()
{
	return _search_tab_edge_pos;
}

void Ccell_grabDlg::SetSearchTabEdgePos(int search_tab_edge_pos)
{
	_search_tab_edge_pos = search_tab_edge_pos;
}

void Ccell_grabDlg::DoGrab()
{
	vector<CString> vt_file_list = GetFileListInFolder("D:\\SplitImg\\");

	CString strLog;

	for (int i = 0; i < vt_file_list.size(); i++) {
		_grab_tiny.gLoad(vt_file_list[i]);

		strLog.Format("imgLoad[%d] %s", i, vt_file_list[i]);
		_glogger->info(strLog);

		int current_idx = GetCurrentGrabIdx();
		int remain_tab_idx = GetRemainTabIdx();

		strLog.Format("DoGrab[%d] current_idx:%d, remain_tab_idx:%d",i ,current_idx, remain_tab_idx);
		_glogger->info(strLog);

		GrabCopyToMainBuf(_grab_buf, _grab_tiny.gGetImgPtr(), WIDTH, HEIGHT_GRAB, current_idx);

		//전이미지+현이미지에서 탭을 찾는다
		int tab_edge_pos = 0;
		bool bSearchTab = false;
		if (remain_tab_idx <= 0) {
			CRect rtroi;
			rtroi.SetRect(14200, 0, 14400, HEIGHT_GRAB);
			bSearchTab = SearchTab(_grab_buf, current_idx, rtroi, WIDTH, HEIGHT_GRAB, TAB_EDGE_TH, TAB_EDGE_OFFSET, &tab_edge_pos);
			SetSearchTabEdgePos(tab_edge_pos);

			strLog.Format("SearchTab[%d] current_idx:%d, remain_tab_idx:%d",i, current_idx, remain_tab_idx);
			_glogger->info(strLog);
		}

		//탭이 있다면 grab end idx를 설정한다
		if (bSearchTab == true) {
			//거리로 설정하자
			SetRemainTabIdx(5);
			SetSearchTab(true);
			strLog.Format("TabOK[%d] set_trg_idx:%d", i, current_idx + 4);
			_glogger->info(strLog);
		}

		remain_tab_idx = GetRemainTabIdx();
		remain_tab_idx--;

		SetRemainTabIdx(remain_tab_idx);

		//grab end idx에 현재 idx가 위치하면
		if (remain_tab_idx <= 0 && GetSearchTab() == true) {
			SetSearchTab(false);
			//inspect 버퍼로 카피 후 검사 - thread 로 처리하자(해당 버퍼 근처에 cell no 넘겨서 처리)
			CopyToInspectBuf(_grab_buf, _inspect.gGetImgPtr(), WIDTH, GRAB_BUF_SIZE, _inspect.gGetWidth(), _inspect.gGetHeight(), GetSearchTabEdgePos() - SPEC_TAB_LENGTH * 0.5);
			_inspect.UpdateDisplay();

			strLog.Format("CopyWithInspect[%d] current_idx:%d, remain_tab_idx:%d", i, current_idx, remain_tab_idx);
			_glogger->info(strLog);
		}

		//실제 정속은 초당60
		Sleep(200);
	}
}

void Ccell_grabDlg::CopyToInspectBuf(unsigned char* pimg_src, unsigned char* pimg_dst, 
	int width_src, int height_src, int width_dst, int height_dst, int edge_tab_center)
{
	//tab center pos을 기점으로 검사버퍼에 카피한다
	int stt_y = edge_tab_center - (HEIGHT * 0.5);
	int end_y = edge_tab_center + (HEIGHT * 0.5);

	if (stt_y < 0) {
		//전 버퍼로 넘어가면 - 2번 카피
		int stt_y1 = GRAB_BUF_SIZE + stt_y;
		int copy_length_y1 = GRAB_BUF_SIZE - stt_y1;
		memcpy(pimg_dst, pimg_src + stt_y1 * width_src, sizeof(unsigned char) * width_src * copy_length_y1);

		int copy_length_y2 = end_y;
		memcpy(pimg_dst + copy_length_y1 * width_dst, pimg_src, sizeof(unsigned char) * width_src * copy_length_y2);
	}
	else if (end_y > GRAB_BUF_SIZE) {
		//다음 버퍼로 넘어가면 - 2번 카피
		int copy_length_y1 = GRAB_BUF_SIZE - stt_y;
		memcpy(pimg_dst, pimg_src + stt_y * width_src, sizeof(unsigned char) * width_src * copy_length_y1);

		int copy_length_y2 = end_y - GRAB_BUF_SIZE;
		memcpy(pimg_dst + copy_length_y1 * width_dst, pimg_src, sizeof(unsigned char) * width_src * copy_length_y2);
	}
	else {
		memcpy(pimg_dst, pimg_src + stt_y * width_src, sizeof(unsigned char) * width_src * HEIGHT);
	}
}

bool Ccell_grabDlg::SearchTab(unsigned char* pimg, int idx, CRect rtroi, int width, int height, int th, int offset, int* edge_pos)
{
	/*	buf_cnt		height
		10			250

		idx	before	current
		0	2250	0
		1	0		250
		2	250		500
		3	500		750
		4	750		1000
		5	1000	1250
		6	1250	1500
		7	1500	1750
		8	1750	2000
		9	2000	2250*/

	bool search = false;

	//프로젝션
	int* pProjY = new int[height * 2];
	memset(pProjY, 0, sizeof(int) * height * 2);
	//원버퍼 y pos 가지고 있는 배열
	int* pProjPos = new int[height * 2];
	memset(pProjPos, 0, sizeof(int) * height * 2);

	int cnt_w = rtroi.Width();
	int idx_y = 0;

	//2개의 tiny grab 버퍼를 본다
	for (int idx = 0; idx < 2; idx++) {
		int stt_y = 0;
		int end_y = 0;

		if (_grab_idx - 2 + idx < 0) {
			stt_y = GRAB_BUF_SIZE + (_grab_idx - 2 + idx) % GRAB_BUF_CNT * height;
			end_y = stt_y + height;
		}
		else {
			stt_y = (_grab_idx - 2 + idx) % GRAB_BUF_CNT * height;
			end_y = (_grab_idx - 1 + idx) % GRAB_BUF_CNT * height;
		}


		for (int y = stt_y; y < end_y; y++) {
			int sum = 0;
			for (int x = rtroi.left; x < rtroi.right; x++) {
				sum += *(pimg + y * width + x);
			}
			if (cnt_w < 1) {
				cnt_w = 1;
			}
			sum /= cnt_w;
			pProjY[idx_y] = sum;
			pProjPos[idx_y] = y;
			idx_y++;
		}
	}

	//탭 th
	for (int y = 0; y < height * 2 - offset; y++) {
		int val_top = pProjY[y];
		int val_bot = pProjY[y + offset];

		//방향
		//up->dn
		//w->b
		int diff = val_top - val_bot;

		if (diff > th) {
			int pos_y = y + (offset * 0.5);
			int full_buf_y_pos = pProjPos[pos_y];
			(*edge_pos) = full_buf_y_pos;
			search = true;
			break;
		}
	}

	delete[] pProjY;
	delete[] pProjPos;

	return search;
}


void Ccell_grabDlg::GrabCopyToMainBuf(unsigned char* pimgSrc, unsigned char* pimgDst, int width, int height, int current_grab_idx)
{
	int stt_y = current_grab_idx * height;

	//테스트 시 이미지 보여야 되서 카피
	unsigned char* pImg = _grab_full_buf.gGetImgPtr();
	memcpy(pImg + stt_y * width, pimgDst, sizeof(unsigned char) * width * height);
	_grab_full_buf.UpdateDisplay();

	//해당위치에 버퍼 카피
	memcpy(pimgSrc + stt_y * width, pimgDst, sizeof(unsigned char) * width * height);
	
	current_grab_idx++;

	if (current_grab_idx >= GRAB_BUF_CNT) {
		current_grab_idx = 0;
	}

	SetCurrentGrabIdx(current_grab_idx);

	CString strLog;
	strLog.Format("GrabCopyToMainBuf stt_y:%d, current_grab_idx:%d", stt_y, current_grab_idx);
	_glogger->info(strLog);
}

vector<CString> Ccell_grabDlg::GetFileListInFolder(CString strPath)
{
	std::vector<CString> vt_file_list;
	vt_file_list.clear();

	CString strPathAllFile;
	strPathAllFile.Format("%s\\*.*", strPath);

	// 검색 클래스
	CFileFind finder;

	// CFileFind는 파일, 디렉터리가 존재하면 TRUE 를 반환함
	BOOL bWorking = finder.FindFile(strPathAllFile);

	CString strFileName;
	CString strDirName;

	while (bWorking)
	{
		//다음 파일 or 폴더 가 존재하면다면 TRUE 반환
		bWorking = finder.FindNextFile();

		// folder 일 경우는 continue
		if (finder.IsDirectory() || finder.IsDots())
			continue;

		//파일의 이름
		CString _fileName = finder.GetFileName();

		// 현재폴더 상위폴더 썸네일파일은 제외
		if (_fileName == _T("Thumbs.db")) continue;

		strFileName = finder.GetFilePath();// GetFileTitle();

										   //읽어온 파일 이름을 리스트박스에 넣음
		vt_file_list.emplace_back(strFileName);
	}

	return vt_file_list;

}


void Ccell_grabDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void Ccell_grabDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR Ccell_grabDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Ccell_grabDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//CDialogEx::OnOK();

}


void Ccell_grabDlg::OnBnClickedOk2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	SetEvent(EV_grab);
}

