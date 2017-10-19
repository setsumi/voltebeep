
// voltebeep.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CvoltebeepApp:
// このクラスの実装については、voltebeep.cpp を参照してください。
//

class CvoltebeepApp : public CWinApp
{
public:
	CvoltebeepApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CvoltebeepApp theApp;