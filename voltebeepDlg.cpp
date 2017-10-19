
// voltebeepDlg.cpp : implementation file
//

#include "stdafx.h"
#include "voltebeep.h"
#include "voltebeepDlg.h"
#include "afxdialogex.h"

#include <vector>
#include "utf8.h"
//using namespace utf8;
//using namespace std;

#define NO_SOUND      L"* no sound *"
#define INI_SECTION   L"general"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CvoltebeepDlg *gDialog;

void Exception(CString msg)
{
   AfxMessageBox(msg);
   AfxThrowUserException();
}
inline CString IntToStr(int num)
{
	CString str;
	str.Format(L"%d", num);
	return str;
}
float VolNorm(float vol)
{
	return (vol < 0.005f)? 0.005f: vol;
}
CString ExePath()
{
	TCHAR szPath[MAX_PATH];
	if(!GetModuleFileName(NULL, szPath, MAX_PATH))
		Exception(L"GetModuleFileName() failed");
	return szPath;
}
CString ExeFolder()
{
	CString path(ExePath());
	path.Truncate(path.ReverseFind(L'\\'));
	return path;
}
CString ExeName()
{
	CString path(ExePath());
	path.Truncate(path.ReverseFind(L'.'));
	return path;
}

#define TIMER_PERIOD 100
void CALLBACK EXPORT CvoltebeepDlg::TimerUpdateProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
	gDialog->m_pSystem->update();

	//debug
#ifdef _DEBUG
	int c;
	gDialog->m_pSystem->getChannelsPlaying(&c);
	gDialog->SetDlgItemText(IDC_STATIC_M1, IntToStr(c));
#endif
}

LRESULT CALLBACK CvoltebeepDlg::HookKeyboardProc(int nCode,	WPARAM wParam, LPARAM lParam)
{
	static bool wink=false; // Win-key down flag
	if(!(nCode < 0)) { // allowed to process message now
		if(nCode == HC_ACTION) {
			KBDLLHOOKSTRUCT *phs = (KBDLLHOOKSTRUCT*)lParam;
			// assign key mode
			if (gDialog->m_AssignMode) {
				gDialog->m_AssignMode = false;

				CWnd *wnd = gDialog->GetFocus();
				gDialog->m_radBT.SetFocus();

				if (wnd == &gDialog->m_edtBT1) {
					gDialog->m_Buttons[0].code = phs->vkCode;
				}
				else if (wnd == &gDialog->m_edtBT2) {
					gDialog->m_Buttons[1].code = phs->vkCode;
				}
				else if (wnd == &gDialog->m_edtBT3) {
					gDialog->m_Buttons[2].code = phs->vkCode;
				}
				else if (wnd == &gDialog->m_edtBT4) {
					gDialog->m_Buttons[3].code = phs->vkCode;
				}
				else if (wnd == &gDialog->m_edtBT5) {
					gDialog->m_Buttons[4].code = phs->vkCode;
				}
				else if (wnd == &gDialog->m_edtBT6) {
					gDialog->m_Buttons[5].code = phs->vkCode;
				}
				gDialog->ReflectButtons();
			} else {
				// normal mode
				if(phs->vkCode == 92) { // right Win key
					wink = (wParam == WM_KEYDOWN);
					return -1; // block message
				} else {
					if(wink && (wParam == WM_KEYDOWN)) { // hot keys (while holding Win)
						switch (phs->vkCode) {
						case 104: // Numpad8 - increase volume
							gDialog->m_VolumeOffset += 0.02f;
							gDialog->ReflectVOffset_n_Mute();
							break;
						case 98:  // Numpad2 - decrease volume
							gDialog->m_VolumeOffset -= 0.02f;
							gDialog->ReflectVOffset_n_Mute();
							break;
						case 101: // Numpad5 - toggle mute
							gDialog->m_Mute = !gDialog->m_Mute;
							gDialog->ReflectVOffset_n_Mute();
							MessageBeep(gDialog->m_Mute? MB_ICONEXCLAMATION: MB_ICONASTERISK);
						}
					}
					else if (!gDialog->m_Mute) {
						// play key sound
						// find key
						int i=0;
						for (; i < BT_COUNT; i++) {
							if (phs->vkCode == gDialog->m_Buttons[i].code)
								break;
						}
						if (i < BT_COUNT) {
							BT *pbt = gDialog->m_Buttons + i;
							if (wParam == WM_KEYDOWN) {
								if (!pbt->isdown) {
									pbt->isdown = true;
									if (*pbt->ppSound) {
										gDialog->m_pSystem->playSound(*pbt->ppSound, 0, true, &gDialog->m_pChannel);
										gDialog->m_pChannel->setVolume(VolNorm(*pbt->pVolume + gDialog->m_VolumeOffset));
										if (gDialog->m_grpFlat == 1) {
											gDialog->m_pChannel->setPan(pbt->pan);
										}
										gDialog->m_pChannel->setPaused(false);
									}
								}
							}
							else if (wParam == WM_KEYUP) {
								pbt->isdown = false;
							}
						}
					}
				}
			}
		}
	}
	// Call next hook
	return CallNextHookEx(gDialog->m_KeybHook, nCode, wParam, lParam);
}

// CvoltebeepDlg dialog constructor

CvoltebeepDlg::CvoltebeepDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CvoltebeepDlg::IDD, pParent)
	, m_grpFlat(-1)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	for (int i=0; i < BT_COUNT; i++) {
		m_Buttons[i].isdown = false;
		m_Buttons[i].code = 0;
		m_Buttons[i].pan = 0;
		m_Buttons[i].pVolume = NULL;
		m_Buttons[i].ppSound = NULL;
	}
	for (int i=0; i < SND_COUNT; i++) {
		m_Sounds[i].volume = 1.0f;
		m_Sounds[i].pSound = NULL;
	}
	m_pExtradriverdata = NULL;
	m_pChannel = NULL;
	m_ini.setINIFileName(ExeName() + L".ini");
	m_AssignMode = false;
	m_MaxVolume = 300;
	m_VolumeOffset = 0.0f;
	m_Mute = false;
}

void CvoltebeepDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, m_sldVolume);
	DDX_Control(pDX, IDC_LIST_SOUNDS, m_lstboxSounds);
	DDX_Control(pDX, IDC_RADIO_BT, m_radBT);
	DDX_Control(pDX, IDC_EDIT1, m_edtBT1);
	DDX_Control(pDX, IDC_EDIT2, m_edtBT2);
	DDX_Control(pDX, IDC_EDIT3, m_edtBT3);
	DDX_Control(pDX, IDC_EDIT4, m_edtBT4);
	DDX_Control(pDX, IDC_EDIT5, m_edtBT5);
	DDX_Control(pDX, IDC_EDIT6, m_edtBT6);
	DDX_Radio(pDX, IDC_RADIO_FLAT, m_grpFlat);
}

BEGIN_MESSAGE_MAP(CvoltebeepDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HSCROLL()
	ON_MESSAGE(WM_DESTROY, OnExit)
	ON_LBN_SELCHANGE(IDC_LIST_SOUNDS, OnItemchangedListSounds)
	ON_BN_CLICKED(IDC_RADIO_BT, &CvoltebeepDlg::OnBnClickedRadioBt)
	ON_BN_CLICKED(IDC_RADIO_FX, &CvoltebeepDlg::OnBnClickedRadioBt)
	ON_EN_SETFOCUS(IDC_EDIT1, &CvoltebeepDlg::OnEnSetfocusEdit1)
	ON_EN_SETFOCUS(IDC_EDIT2, &CvoltebeepDlg::OnEnSetfocusEdit2)
	ON_EN_SETFOCUS(IDC_EDIT3, &CvoltebeepDlg::OnEnSetfocusEdit3)
	ON_EN_SETFOCUS(IDC_EDIT4, &CvoltebeepDlg::OnEnSetfocusEdit4)
	ON_EN_SETFOCUS(IDC_EDIT5, &CvoltebeepDlg::OnEnSetfocusEdit5)
	ON_EN_SETFOCUS(IDC_EDIT6, &CvoltebeepDlg::OnEnSetfocusEdit6)
	ON_BN_CLICKED(IDC_RADIO_FLAT, &CvoltebeepDlg::OnBnClickedRadioFlat)
	ON_BN_CLICKED(IDC_RADIO_POS, &CvoltebeepDlg::OnBnClickedRadioFlat)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CvoltebeepDlg::OnBnClickedButtonHelp)
END_MESSAGE_MAP()


// CvoltebeepDlg message handler

BOOL CvoltebeepDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// set big icon
	SetIcon(m_hIcon, FALSE);		// set small icon

	// TODO: insert your initialization here
	gDialog = this;
	m_edbtnExStyle = GetWindowLong(m_edtBT1.m_hWnd, GWL_EXSTYLE);

	// set process priority to high
	if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
		Exception(L"SetPriorityClass() failed");

	// init GUI controls
	SetDlgItemText(IDC_RADIO_BT, L"BT");
	SetDlgItemText(IDC_RADIO_FX, L"FX");
	SetDlgItemText(IDC_STATIC_KEYS, L"Choose Keys");
	SetDlgItemText(IDC_STATIC_SOUND, L"Select Sound");
	SetDlgItemText(IDC_STATIC_BT, L"BT Buttons");
	SetDlgItemText(IDC_STATIC_FX, L"FX Buttons");
	SetDlgItemText(IDC_STATIC_SOUNDSTYLE, L"Sound Style");
	SetDlgItemText(IDC_RADIO_FLAT, L"Flat");
	SetDlgItemText(IDC_RADIO_POS, L"Positional");
	SetDlgItemText(IDC_STATIC_M1, L"");
	m_lstboxSounds.ModifyStyle(0, LVS_SINGLESEL|LVS_SHOWSELALWAYS);
	m_MaxVolume = m_ini.getKeyValueInt(L"maxvolume", INI_SECTION);
	m_sldVolume.SetRange(0, m_MaxVolume);
	m_sldVolume.SetTicFreq(10);
	m_grpFlat = m_ini.getKeyValueInt(L"soundstyle", INI_SECTION);
	UpdateData(FALSE);

	// set keyboard hook
	m_KeybHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)HookKeyboardProc, GetModuleHandle(NULL), NULL/*GetCurrentThreadId()*/);
	if(m_KeybHook == NULL)
		Exception(L"SetWindowsHookEx() failed");

	// init sound system
	if (FMOD::System_Create(&m_pSystem) != FMOD_OK)
		Exception(L"FMOD::System_Create() failed");
	if (m_pSystem->getVersion(&m_Version) != FMOD_OK)
		Exception(L"FMOD::system->getVersion() failed");
	if (m_Version < FMOD_VERSION)
		Exception(L"FMOD lib version doesn't match header version");
	if (m_pSystem->init(32, FMOD_INIT_NORMAL, m_pExtradriverdata) != FMOD_OK)
		Exception(L"FMOD::system->init() failed");

	// init buttons
	for (int i=0; i < BT_COUNT; i++) {
		CString key;
		key.Format(L"button%d", i+1);
		m_Buttons[i].code = m_ini.getKeyValueInt(key, INI_SECTION);
		key.Format(L"button%dpan", i+1);
		m_Buttons[i].pan = m_ini.getKeyValueFloat(key, INI_SECTION);
	}
	ReflectButtons();
	m_Buttons[0].ppSound = &m_Sounds[0].pSound;
	m_Buttons[0].pVolume = &m_Sounds[0].volume;
	m_Buttons[1].ppSound = &m_Sounds[0].pSound;
	m_Buttons[1].pVolume = &m_Sounds[0].volume;
	m_Buttons[2].ppSound = &m_Sounds[0].pSound;
	m_Buttons[2].pVolume = &m_Sounds[0].volume;
	m_Buttons[3].ppSound = &m_Sounds[0].pSound;
	m_Buttons[3].pVolume = &m_Sounds[0].volume;
	m_Buttons[4].ppSound = &m_Sounds[1].pSound;
	m_Buttons[4].pVolume = &m_Sounds[1].volume;
	m_Buttons[5].ppSound = &m_Sounds[1].pSound;
	m_Buttons[5].pVolume = &m_Sounds[1].volume;

	// load sounds
	m_Sounds[0].file = m_ini.getKeyValue(L"soundbt", INI_SECTION);
	m_Sounds[1].file = m_ini.getKeyValue(L"soundfx", INI_SECTION);
	LoadSounds();
	m_Sounds[0].volume = (float)m_ini.getKeyValueInt(L"volumebt", INI_SECTION) / 100.0f;
	m_Sounds[1].volume = (float)m_ini.getKeyValueInt(L"volumefx", INI_SECTION) / 100.0f;

	m_lstboxSounds.AddString(NO_SOUND);
	m_lstboxSounds.Dir(0, L"*.wav");
	m_lstboxSounds.Dir(0, L"*.mp3");
	m_lstboxSounds.Dir(0, L"*.ogg");
	m_radBT.SetCheck(BST_CHECKED);
	OnBnClickedRadioBt();
	OnHScroll(TB_THUMBPOSITION, m_sldVolume.GetPos(), (CScrollBar*)&m_sldVolume);

	m_VolumeOffset = m_ini.getKeyValueFloat(L"volumeoffset", INI_SECTION);
	ReflectVOffset_n_Mute();

	SetTimer(66, TIMER_PERIOD, TimerUpdateProc);
	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

LRESULT CvoltebeepDlg::OnExit(WPARAM wParam, LPARAM lParam)
{
	// save settings
	m_ini.setKeyInt((int)(m_Sounds[0].volume * 100.0f), L"volumebt", INI_SECTION);
	m_ini.setKeyInt((int)(m_Sounds[1].volume * 100.0f), L"volumefx", INI_SECTION);
	m_ini.setKey(m_Sounds[0].file, L"soundbt", INI_SECTION);
	m_ini.setKey(m_Sounds[1].file, L"soundfx", INI_SECTION);

	for (int i=0; i < BT_COUNT; i++) {
		CString key;
		key.Format(L"button%d", i+1);
		m_ini.setKeyInt((int)m_Buttons[i].code, key, INI_SECTION);
		key.Format(L"button%dpan", i+1);
		m_ini.setKeyFloat(m_Buttons[i].pan, key, INI_SECTION);
	}
	m_ini.setKeyInt(m_MaxVolume, L"maxvolume", INI_SECTION);
	m_ini.setKeyInt(m_grpFlat, L"soundstyle", INI_SECTION);
	m_ini.setKeyFloat(m_VolumeOffset, L"volumeoffset", INI_SECTION);

	// release keyb hook
	if (!UnhookWindowsHookEx(m_KeybHook))
		Exception(L"UnhookWindowsHookEx() failed");

	// shut down sound system
	for (int i=0; i < SND_COUNT; i++) {
		if (m_Sounds[i].pSound) {
			if (m_Sounds[i].pSound->release() != FMOD_OK)
				Exception(L"FMOD::sound->release() failed");
		}
	}
	if (m_pSystem->close() != FMOD_OK)
		Exception(L"FMOD::system->close() failed");
	if (m_pSystem->release() != FMOD_OK)
		Exception(L"FMOD::system->release() failed");

	return DefWindowProc(WM_DESTROY, wParam, lParam);
}

void CvoltebeepDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CvoltebeepDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CvoltebeepDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);

	// Check which slider sent the notification
	if (pSlider == &m_sldVolume)
	{
		int pos=0;
		switch(nSBCode) {
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
			pos = nPos;
			break;
		default:
			pos = m_sldVolume.GetPos();
			break;
		}
		SetVolume(pos);
	}
}

BOOL CvoltebeepDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		switch (pMsg->wParam) {
		case (VK_RETURN): // disable dialog closing on OK (Enter) and Cancel (Esc)
		case (VK_ESCAPE):
			return TRUE;
		case (VK_SPACE):
		case (VK_TAB):
		case (VK_LEFT):
		case (VK_RIGHT):
		case (VK_UP):
		case (VK_DOWN):
			break;
		default:
			return TRUE;
		}
	}
	else if (pMsg->message == WM_SYSKEYDOWN || pMsg->message == WM_SYSKEYUP)
	{
		switch (pMsg->wParam) {
		case (VK_F4): // allow Alt+F4
			break;
		default:
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CvoltebeepDlg::OnBnClickedRadioBt()
{
	SetVolume();
	ReflectSound();
	if (m_radBT.GetCheck()) {
		SetWindowLongPtr(m_edtBT1.m_hWnd, GWL_EXSTYLE, WS_EX_STATICEDGE);
		SetWindowLongPtr(m_edtBT2.m_hWnd, GWL_EXSTYLE, WS_EX_STATICEDGE);
		SetWindowLongPtr(m_edtBT3.m_hWnd, GWL_EXSTYLE, WS_EX_STATICEDGE);
		SetWindowLongPtr(m_edtBT4.m_hWnd, GWL_EXSTYLE, WS_EX_STATICEDGE);
		SetWindowLongPtr(m_edtBT5.m_hWnd, GWL_EXSTYLE, m_edbtnExStyle);
		SetWindowLongPtr(m_edtBT6.m_hWnd, GWL_EXSTYLE, m_edbtnExStyle);
	} else {
		SetWindowLongPtr(m_edtBT1.m_hWnd, GWL_EXSTYLE, m_edbtnExStyle);
		SetWindowLongPtr(m_edtBT2.m_hWnd, GWL_EXSTYLE, m_edbtnExStyle);
		SetWindowLongPtr(m_edtBT3.m_hWnd, GWL_EXSTYLE, m_edbtnExStyle);
		SetWindowLongPtr(m_edtBT4.m_hWnd, GWL_EXSTYLE, m_edbtnExStyle);
		SetWindowLongPtr(m_edtBT5.m_hWnd, GWL_EXSTYLE, WS_EX_STATICEDGE);
		SetWindowLongPtr(m_edtBT6.m_hWnd, GWL_EXSTYLE, WS_EX_STATICEDGE);
	}
	Invalidate(FALSE);
	UpdateWindow();
}

void CvoltebeepDlg::SetVolume(int volume)
{
	int i = m_radBT.GetCheck()? 0: 1;
	if (volume == -1)
		volume = (int)(m_Sounds[i].volume * 100.0f);
	else
		m_Sounds[i].volume = (float)volume / 100.0f;

	m_sldVolume.SetPos(volume);
	CString str;
	str.Format(L"Volume: %d%%", volume);
	SetDlgItemText(IDC_STATIC_VOL, str);
}

void CvoltebeepDlg::ReflectSound()
{
	int ii = m_radBT.GetCheck()? 0: 1;
	CString file(m_Sounds[ii].file);

	int count = m_lstboxSounds.GetCount();
	int i=0;
	for (; i < count; i++)
	{
		CString item;
		m_lstboxSounds.GetText(i, item);
		if (file.CompareNoCase(item) == 0)
			break;
	}
	if (i < count)
		m_lstboxSounds.SetCurSel(i);
	else
		m_lstboxSounds.SetCurSel(0);
}

void CvoltebeepDlg::OnEnSetfocusEdit1()
{
	ReflectButtons();
	m_edtBT1.SetWindowText(L"Press");
	m_AssignMode = true;
}
void CvoltebeepDlg::OnEnSetfocusEdit2()
{
	ReflectButtons();
	m_edtBT2.SetWindowText(L"Press");
	m_AssignMode = true;
}
void CvoltebeepDlg::OnEnSetfocusEdit3()
{
	ReflectButtons();
	m_edtBT3.SetWindowText(L"Press");
	m_AssignMode = true;
}
void CvoltebeepDlg::OnEnSetfocusEdit4()
{
	ReflectButtons();
	m_edtBT4.SetWindowText(L"Press");
	m_AssignMode = true;
}
void CvoltebeepDlg::OnEnSetfocusEdit5()
{
	ReflectButtons();
	m_edtBT5.SetWindowText(L"Press");
	m_AssignMode = true;
}
void CvoltebeepDlg::OnEnSetfocusEdit6()
{
	ReflectButtons();
	m_edtBT6.SetWindowText(L"Press");
	m_AssignMode = true;
}
void CvoltebeepDlg::ReflectButtons()
{
	m_edtBT1.SetWindowText(IntToStr(m_Buttons[0].code));
	m_edtBT2.SetWindowText(IntToStr(m_Buttons[1].code));
	m_edtBT3.SetWindowText(IntToStr(m_Buttons[2].code));
	m_edtBT4.SetWindowText(IntToStr(m_Buttons[3].code));
	m_edtBT5.SetWindowText(IntToStr(m_Buttons[4].code));
	m_edtBT6.SetWindowText(IntToStr(m_Buttons[5].code));
}
void CvoltebeepDlg::OnBnClickedRadioFlat()
{
	UpdateData(TRUE);
}

void CvoltebeepDlg::OnItemchangedListSounds()
{
	// set sound file name
	int sel = m_lstboxSounds.GetCurSel();
	CString file;
	m_lstboxSounds.GetText(sel, file);
	int i = m_radBT.GetCheck()? 0: 1;
	m_Sounds[i].file = file;

	LoadSounds();

	//preview
	if (m_Sounds[i].pSound) {
		m_pSystem->playSound(m_Sounds[i].pSound, 0, true, &m_pChannel);
		m_pChannel->setVolume(VolNorm(m_Sounds[i].volume + m_VolumeOffset));
		m_pChannel->setPaused(false);
	}
}

void CvoltebeepDlg::LoadSounds()
{
	for (int i=0; i < SND_COUNT; i++) {
		SND *ps = m_Sounds + i;
		if (ps->pSound) {
			if (ps->pSound->release() != FMOD_OK)
				Exception(L"FMOD::sound->release() failed");
		}
		if (*ps->file.GetBuffer() == L'*' || ps->file.IsEmpty()) {
			ps->pSound = NULL;
		} else {
			std::vector<char> utf8str;
			const wchar_t *wstr = ps->file.GetBuffer();
			utf8::utf16to8(wstr, wstr + ps->file.GetLength(), back_inserter(utf8str));
			utf8str.push_back(0);
			if (m_pSystem->createSound(utf8str.data(), FMOD_DEFAULT, 0, &ps->pSound) != FMOD_OK) {
				CString err;
				err.Format(L"FMOD::system->createSound(\"%s\") failed", ps->file);
				Exception(err);
			}
		}
	}
}

void CvoltebeepDlg::ReflectVOffset_n_Mute()
{
	CString str;
	str.Format(L"VOffset: %.2f", m_VolumeOffset);
	SetDlgItemText(IDC_STATIC_VOLUMEOFFSET, str);
	str.Format(L"Mute: %s", m_Mute? L"ON": L"OFF");
	SetDlgItemText(IDC_STATIC_MUTE, str);
}


void CvoltebeepDlg::OnBnClickedButtonHelp()
{
	// TODO: Add your control notification handler code here
	MessageBox(L"Keyboard shortcuts (for use in game)\n\n"
		L"right Win + Numpad 8\tincrease overall volume\n"
		L"right Win + Numpad 2\tdecrease overall volume\n"
		L"right Win + Numpad 5\ttoggle mute\n\n", L"Information", MB_ICONINFORMATION|MB_OK);
}
