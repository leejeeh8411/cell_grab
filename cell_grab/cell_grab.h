
// cell_grab.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// Ccell_grabApp:
// �� Ŭ������ ������ ���ؼ��� cell_grab.cpp�� �����Ͻʽÿ�.
//

class Ccell_grabApp : public CWinApp
{
public:
	Ccell_grabApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern Ccell_grabApp theApp;