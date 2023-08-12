
// cell_grabDlg.cpp : ���� ����
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


UINT Thread_SetCellNo(LPVOID lParam)
{
	Ccell_grabDlg *pView = (Ccell_grabDlg *)lParam;
	
	pView->AddCellID();

	return 1;
}


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// Ccell_grabDlg ��ȭ ����



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
	DDX_Control(pDX, IDC_IMAGE_4, _cell_buf_view);
	

}

BEGIN_MESSAGE_MAP(Ccell_grabDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &Ccell_grabDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, &Ccell_grabDlg::OnBnClickedOk2)
END_MESSAGE_MAP()


// Ccell_grabDlg �޽��� ó����

BOOL Ccell_grabDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	_glogger = new gLogger("suslog", std::string("D:\\Log\\") + std::string("syslog.txt"), false, 23, 59);

	AfxBeginThread(ThreadGrab, this, THREAD_PRIORITY_HIGHEST, 0);

	_grab_full_buf.gCreate(WIDTH, GRAB_BUF_SIZE, 8);
	_grab_tiny.gCreate(WIDTH, HEIGHT_GRAB, 8);
	_inspect.gCreate(WIDTH, HEIGHT, 8);
	_cell_buf_view.gCreate(200, 800, 24);

	_grab_buf = new unsigned char[WIDTH * GRAB_BUF_SIZE];
	memset(_grab_buf, 0, sizeof(unsigned char) * WIDTH * GRAB_BUF_SIZE);

	memset(_cell_id_map, 0, sizeof(int) * GRAB_BUF_CNT);

	//DrawCellBuf();

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void Ccell_grabDlg::DrawCellBuf()
{
	_cell_buf_view.gSetImage(NULL, 0, 0, 3);

	int width = _cell_buf_view.gGetWidth();
	int height = _cell_buf_view.gGetHeight();
	int bpp = _cell_buf_view.gGetBPP() / 8;
	

	memset(_cell_buf_view.gGetImgPtr(), 0, sizeof(unsigned char) * width * height * bpp);

	int length_h = height / 40;

	for (int i = 0; i < 40; i++) {
		CPoint ptLine1;
		CPoint ptLine2;
		ptLine1.x = 0;
		ptLine1.y = (i + 1) * length_h;

		ptLine2.x = width;
		ptLine2.y = (i + 1) * length_h;

		_cell_buf_view.gDrawLine(ptLine1, ptLine2, RGB(230, 230, 230));
	}

	for (int i = 0; i < 40; i++) {
		CPoint ptText;
		ptText.x = width * 0.1;
		ptText.y = i * length_h;
		CString strText;
		strText.Format("%2d|  %2d", i, _cell_id_map[i]);
		string strText_ = strText;
		int font_size = 100;
		_cell_buf_view.gDrawText(ptText, strText_, RGB(255, 234, 0), font_size);
	}

	_cell_buf_view.UpdateDisplay();
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

int Ccell_grabDlg::GetCellId()
{
	return _cell_id;
}

void Ccell_grabDlg::SetCellId(int cell_id)
{
	_cell_id = cell_id;
}

void Ccell_grabDlg::AddCellID()
{
	int current_idx = GetCurrentGrabIdx();
	int cell_id = GetCellId();

	PutCellId(current_idx, cell_id);

	cell_id++;
	SetCellId(cell_id);
}

void Ccell_grabDlg::PutCellId(int grab_idx, int cell_id)
{
	_cell_id_map[grab_idx] = cell_id;
}

int Ccell_grabDlg::GetInsCellID(int current_grab_idx, int before_range)
{
	int cell_id = 0;

	int stt_y = current_grab_idx - before_range;
	int end_y = current_grab_idx;
	
	vector<int> vt_cell_id;
	vt_cell_id.clear();

	if (stt_y < 0) {
		//�� ���۷� �Ѿ�� - 2�� ī��
		int stt_y1 = GRAB_BUF_CNT + stt_y;
		int end_y1 = GRAB_BUF_CNT;
		
		for (int i = stt_y1; i < end_y1; i++) {
			if (_cell_id_map[i] != 0) {
				vt_cell_id.emplace_back(_cell_id_map[i]);
				//�����°� ���ۿ��� �����.
				_cell_id_map[i] = 0;
			}
		}

		int stt_y2 = 0;
		int end_y2 = end_y;

		for (int i = stt_y1; i < end_y1; i++) {
			if (_cell_id_map[i] != 0) {
				vt_cell_id.emplace_back(_cell_id_map[i]);
				//�����°� ���ۿ��� �����.
				_cell_id_map[i] = 0;
			}
		}

	}
	else {
		int stt_y1 = stt_y;
		int end_y1 = end_y;

		for (int i = stt_y1; i < end_y1; i++) {
			if (_cell_id_map[i] != 0) {
				vt_cell_id.emplace_back(_cell_id_map[i]);
				//�����°� ���ۿ��� �����.
				_cell_id_map[i] = 0;
			}
		}
	}

	int cell_list_size = vt_cell_id.size();

	//���� ��ġ���� �� �����Ѱ� ��������
	if (cell_list_size > 0) {
		cell_id = vt_cell_id[cell_list_size - 1];
	}
	
	if (cell_id == 0) {
		int a = 9;
	}

	return cell_id;
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

		//���̹���+���̹������� ���� ã�´�
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

		//���� �ִٸ� grab end idx�� �����Ѵ�
		if (bSearchTab == true) {

			//�� ������ �ù�
			AfxBeginThread(Thread_SetCellNo, this, THREAD_PRIORITY_HIGHEST, 0);

			//�Ÿ��� ��������
			SetRemainTabIdx(5);
			SetSearchTab(true);
			strLog.Format("TabOK[%d] set_trg_idx:%d", i, current_idx + 4);
			_glogger->info(strLog);
		}

		remain_tab_idx = GetRemainTabIdx();
		remain_tab_idx--;

		SetRemainTabIdx(remain_tab_idx);

		//grab end idx�� ���� idx�� ��ġ�ϸ�
		if (remain_tab_idx <= 0 && GetSearchTab() == true) {
			SetSearchTab(false);
			//inspect ���۷� ī�� �� �˻� - thread �� ó������(�ش� ���� ��ó�� cell no �Ѱܼ� ó��)
			int cellid = GetInsCellID(current_idx, 6);
			_inspect.gSetImage(NULL, 0, 0, 8);
			CopyToInspectBuf(_grab_buf, _inspect.gGetImgPtr(), WIDTH, GRAB_BUF_SIZE, _inspect.gGetWidth(), _inspect.gGetHeight(), GetSearchTabEdgePos() - SPEC_TAB_LENGTH * 0.5);
			//image�� cell�׷�����(test)
			CString str_cell_id;
			str_cell_id.Format("Cell ID : %d", cellid);
			string str_cell_id_ = str_cell_id;
			CPoint ptText;
			ptText.SetPoint(100, 200);
			int font_size = 100;
			_inspect.gDrawText(ptText, str_cell_id_, RGB(0, 255, 0), font_size);
			_inspect.UpdateDisplay();

			strLog.Format("CopyWithInspect[%d] current_idx:%d, remain_tab_idx:%d", i, current_idx, remain_tab_idx);
			_glogger->info(strLog);
		}

		DrawCellBuf();
		//���� ������ �ʴ�60
		Sleep(200);
	}
}

void Ccell_grabDlg::CopyToInspectBuf(unsigned char* pimg_src, unsigned char* pimg_dst, 
	int width_src, int height_src, int width_dst, int height_dst, int edge_tab_center)
{
	//tab center pos�� �������� �˻���ۿ� ī���Ѵ�
	int stt_y = edge_tab_center - (HEIGHT * 0.5);
	int end_y = edge_tab_center + (HEIGHT * 0.5);

	if (stt_y < 0) {
		//�� ���۷� �Ѿ�� - 2�� ī��
		int stt_y1 = GRAB_BUF_SIZE + stt_y;
		int copy_length_y1 = GRAB_BUF_SIZE - stt_y1;
		memcpy(pimg_dst, pimg_src + stt_y1 * width_src, sizeof(unsigned char) * width_src * copy_length_y1);

		int copy_length_y2 = end_y;
		memcpy(pimg_dst + copy_length_y1 * width_dst, pimg_src, sizeof(unsigned char) * width_src * copy_length_y2);
	}
	else if (end_y > GRAB_BUF_SIZE) {
		//���� ���۷� �Ѿ�� - 2�� ī��
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

	//��������
	int* pProjY = new int[height * 2];
	memset(pProjY, 0, sizeof(int) * height * 2);
	//������ y pos ������ �ִ� �迭
	int* pProjPos = new int[height * 2];
	memset(pProjPos, 0, sizeof(int) * height * 2);

	int cnt_w = rtroi.Width();
	int idx_y = 0;

	//2���� tiny grab ���۸� ����
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

	//�� th
	for (int y = 0; y < height * 2 - offset; y++) {
		int val_top = pProjY[y];
		int val_bot = pProjY[y + offset];

		//����
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

	//�׽�Ʈ �� �̹��� ������ �Ǽ� ī��
	unsigned char* pImg = _grab_full_buf.gGetImgPtr();
	memcpy(pImg + stt_y * width, pimgDst, sizeof(unsigned char) * width * height);
	_grab_full_buf.UpdateDisplay();

	//�ش���ġ�� ���� ī��
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

	// �˻� Ŭ����
	CFileFind finder;

	// CFileFind�� ����, ���͸��� �����ϸ� TRUE �� ��ȯ��
	BOOL bWorking = finder.FindFile(strPathAllFile);

	CString strFileName;
	CString strDirName;

	while (bWorking)
	{
		//���� ���� or ���� �� �����ϸ�ٸ� TRUE ��ȯ
		bWorking = finder.FindNextFile();

		// folder �� ���� continue
		if (finder.IsDirectory() || finder.IsDots())
			continue;

		//������ �̸�
		CString _fileName = finder.GetFileName();

		// �������� �������� ����������� ����
		if (_fileName == _T("Thumbs.db")) continue;

		strFileName = finder.GetFilePath();// GetFileTitle();

										   //�о�� ���� �̸��� ����Ʈ�ڽ��� ����
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void Ccell_grabDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR Ccell_grabDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Ccell_grabDlg::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	//CDialogEx::OnOK();

}


void Ccell_grabDlg::OnBnClickedOk2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	SetEvent(EV_grab);
}

