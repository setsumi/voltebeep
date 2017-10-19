
// voltebeepDlg.h : header file
//

#pragma once
#include "afxcmn.h"

#include "INI.h"
#include "afxwin.h"

#define BT_COUNT 6
struct BT {
	bool isdown;
	DWORD code;
	float pan;
	// referencing
	float *pVolume;
	FMOD::Sound **ppSound;
};

#define SND_COUNT 2
struct SND {
	CString file;
	// shared
	float volume;
	FMOD::Sound *pSound;
};

// CvoltebeepDlg dialog
class CvoltebeepDlg : public CDialogEx
{
// construction
public:
	CvoltebeepDlg(CWnd* pParent = NULL);	// standard constructor

// dialog data
	enum { IDD = IDD_VOLTEBEEP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// implementation
protected:
	HICON m_hIcon;

	// my stuff
	HHOOK m_KeybHook;
	FMOD::System     *m_pSystem;
	FMOD::Channel    *m_pChannel;
	unsigned int      m_Version;
	void             *m_pExtradriverdata;
	BT                m_Buttons[BT_COUNT];
	SND               m_Sounds[SND_COUNT];
	CIniReader        m_ini;
	bool              m_AssignMode;
	LONG              m_edbtnExStyle;
	int               m_MaxVolume;
	float             m_VolumeOffset;
	bool              m_Mute;

	void ReflectSound();
	void SetVolume(int volume = -1);
	void ReflectButtons();
	void LoadSounds();
	void ReflectVOffset_n_Mute();

	static void CALLBACK EXPORT TimerUpdateProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime);
	static LRESULT CALLBACK HookKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	BOOL PreTranslateMessage(MSG* pMsg);

	// message handlers
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl m_sldVolume;
	CListBox m_lstboxSounds;
	CButton m_radBT;

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	LRESULT OnExit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnItemchangedListSounds();
	afx_msg void OnBnClickedRadioBt();

	afx_msg void OnEnSetfocusEdit1();
	afx_msg void OnEnSetfocusEdit2();
	afx_msg void OnEnSetfocusEdit3();
	afx_msg void OnEnSetfocusEdit4();
	afx_msg void OnEnSetfocusEdit5();
	afx_msg void OnEnSetfocusEdit6();
	CEdit m_edtBT1;
	CEdit m_edtBT2;
	CEdit m_edtBT3;
	CEdit m_edtBT4;
	CEdit m_edtBT5;
	CEdit m_edtBT6;

	int m_grpFlat;
	afx_msg void OnBnClickedRadioFlat();

	afx_msg void OnBnClickedButtonHelp();
};
