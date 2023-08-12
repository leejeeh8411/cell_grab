
// cell_grabDlg.h : 헤더 파일
//

#pragma once

static const int WIDTH = 15552;
static const int HEIGHT_GRAB = 256;
static const int HEIGHT = 3072;

static const int GRAB_BUF_CNT = 40;
static const int GRAB_BUF_SIZE = HEIGHT_GRAB * GRAB_BUF_CNT;

static const int TAB_EDGE_TH = 80;
static const int TAB_EDGE_OFFSET = 6;

static const int SPEC_TAB_LENGTH = 1000;


#include "gLogger.h"
#include "gImage.h"
#include <cmath>

// Ccell_grabDlg 대화 상자
class Ccell_grabDlg : public CDialogEx
{
// 생성입니다.
public:
	Ccell_grabDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

	CEvent	EV_grab;

	gLogger* _glogger;

	gImage _grab_full_buf;
	gImage _grab_tiny;
	gImage _inspect;
	gImage _cell_buf_view;

	void Ccell_grabDlg::DrawCellBuf();

	int Ccell_grabDlg::GetCurrentGrabIdx();
	void Ccell_grabDlg::SetCurrentGrabIdx(int current_grab_idx);
	int Ccell_grabDlg::GetRemainTabIdx();
	void Ccell_grabDlg::SetRemainTabIdx(int remain_tab_idx);
	bool Ccell_grabDlg::GetSearchTab();
	void Ccell_grabDlg::SetSearchTab(bool searchTab);
	int Ccell_grabDlg::GetSearchTabEdgePos();
	void Ccell_grabDlg::SetSearchTabEdgePos(int searchTab);
	
	
	//현재 cell id를 저장한다(테스트용)
	void Ccell_grabDlg::AddCellID();
	//현재 범위에서 n번째 전까지 확인하여 cell no 리턴
	int Ccell_grabDlg::GetInsCellID(int current_grab_idx, int before_range);


	void Ccell_grabDlg::DoGrab();
	void Ccell_grabDlg::GrabCopyToMainBuf(unsigned char* pimgSrc, unsigned char* pimgDst, int width, int height, int current_grab_idx);
	void Ccell_grabDlg::CopyToInspectBuf(unsigned char* pimg_src, unsigned char* pimg_dst,
		int width_src, int height_src, int width_dst, int height_dst, int edge_tab_center);
	bool Ccell_grabDlg::SearchTab(unsigned char* pimg, int idx, CRect rtroi, int width, int height, int th, int offset, int* edge_pos);

	std::vector<CString> Ccell_grabDlg::GetFileListInFolder(CString strPath);

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CELL_GRAB_DIALOG };
#endif

	protected:

		
	unsigned char* _grab_buf;
	int _grab_idx = 0;
	int _remain_tab = 0;
	bool _search_tab = false;
	bool _first_run = true;
	int _search_tab_edge_pos = 0;
	int _cell_id_map[GRAB_BUF_CNT];
	int _cell_id = 1;

	//cell no를 cell buf에 넣는다
	void Ccell_grabDlg::PutCellId(int grab_idx, int cell_id);
	//현재 cell id를 가져온다(테스트용)
	int Ccell_grabDlg::GetCellId();
	//현재 cell id를 저장한다(테스트용)
	void Ccell_grabDlg::SetCellId(int cell_id);

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedOk2();
};
