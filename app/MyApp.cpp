/*
 * CiderPress
 * Copyright (C) 2007 by faddenSoft, LLC.  All Rights Reserved.
 * See the file LICENSE for distribution terms.
 */
/*
 * The application object.
 */
#include "stdafx.h"
#include "../util/UtilLib.h"
#include "MyApp.h"
#include "Registry.h"
#include "Main.h"
#include "DiskArchive.h"
#include "Help/PopUpIds.h"
#include <process.h>

/* magic global that MFC finds (or that finds MFC) */
MyApp gMyApp;

/* used for debug logging */
DebugLog* gDebugLog;


/*
 * This is the closest thing to "main" that we have, but we
 * should wait for InitInstance for most things.
 */
MyApp::MyApp(LPCTSTR lpszAppName) : CWinApp(lpszAppName)
{
    gDebugLog = new DebugLog(L"C:\\src\\cplog.txt");

    time_t now = time(NULL);

    LOGI("CiderPress v%d.%d.%d%ls started at %.24hs",
        kAppMajorVersion, kAppMinorVersion, kAppBugVersion,
        kAppDevString, ctime(&now));

    int tmpDbgFlag;
    // enable memory leak detection
    tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
    LOGI("Leak detection enabled");

    EnableHtmlHelp();
}

/*
 * This is the last point of control we have.
 */
MyApp::~MyApp(void)
{
    DiskArchive::AppCleanup();
    NiftyList::AppCleanup();

    LOGI("SHUTTING DOWN\n");
    delete gDebugLog;
}

BOOL MyApp::InitInstance(void)
{
    // Create the main window.
    m_pMainWnd = new MainWindow;
    m_pMainWnd->ShowWindow(m_nCmdShow);
    m_pMainWnd->UpdateWindow();

    LOGD("Happily in InitInstance!");

    /* find our .EXE file */
    //HMODULE hModule = ::GetModuleHandle(NULL);
    WCHAR buf[MAX_PATH];
    if (::GetModuleFileName(NULL /*hModule*/, buf, NELEM(buf)) != 0) {
        LOGD("Module name is '%ls'", buf);
        fExeFileName = buf;

        WCHAR* cp = wcsrchr(buf, '\\');
        if (cp == NULL)
            fExeBaseName = L"";
        else
            fExeBaseName = fExeFileName.Left(cp - buf +1);
    } else {
        LOGW("GLITCH: GetModuleFileName failed (err=%ld)", ::GetLastError());
    }

    LogModuleLocation(L"riched.dll");
    LogModuleLocation(L"riched20.dll");
    LogModuleLocation(L"riched32.dll");

#if 0
    /* find our .INI file by tweaking the EXE path */
    char* cp = strrchr(buf, '\\');
    if (cp == NULL)
        cp = buf;
    else
        cp++;
    if (cp + ::lstrlen(_T("CiderPress.INI")) >= buf+sizeof(buf))
        return FALSE;
    ::lstrcpy(cp, _T("CiderPress.INI"));

    free((void*)m_pszProfileName);
    m_pszProfileName = strdup(buf);
    LOGI("Profile name is '%ls'", m_pszProfileName);

    if (!WriteProfileString("SectionOne", "MyEntry", "test"))
        LOGI("WriteProfileString failed");
#endif

    // This causes functions like SetProfileInt to use the registry rather
    // than a .INI file.  The registry key is "usually the name of a company".
    SetRegistryKey(L"faddenSoft");

    //LOGI("Registry key is '%ls'", m_pszRegistryKey);
    //LOGI("Profile name is '%ls'", m_pszProfileName);
    LOGI("Short command line is '%ls'", m_lpCmdLine);
    //LOGI("CP app name is '%ls'", m_pszAppName);
    //LOGI("CP exe name is '%ls'", m_pszExeName);
    LOGI("CP help file is '%ls'", m_pszHelpFilePath);
    LOGI("Command line is '%ls'", ::GetCommandLine());

    //if (!WriteProfileString("SectionOne", "MyEntry", "test"))
    //  LOGI("WriteProfileString failed");

#ifdef CAN_UPDATE_FILE_ASSOC
    /*
     * If we're installing or uninstalling, do what we need to and then
     * bail immediately.  This will hemorrhage memory, but I'm sure the
     * incredibly robust Windows environment will take it in stride.
     */
    if (wcscmp(m_lpCmdLine, L"-install") == 0) {
        LOGI("Invoked with INSTALL flag");
        fRegistry.OneTimeInstall();
        exit(0);
    } else if (wcscmp(m_lpCmdLine, L"-uninstall") == 0) {
        LOGI("Invoked with UNINSTALL flag");
        fRegistry.OneTimeUninstall();
        exit(1);    // tell DeployMaster to continue with uninstall
    }

    fRegistry.FixBasicSettings();
#endif

    return TRUE;
}

void MyApp::LogModuleLocation(const WCHAR* name)
{
    HMODULE hModule;
    WCHAR fileNameBuf[256];
    hModule = ::GetModuleHandle(name);
    if (hModule != NULL &&
        ::GetModuleFileName(hModule, fileNameBuf, NELEM(fileNameBuf)) != 0)
    {
        // GetModuleHandle does not increase ref count, so no need to release
        LOGI("Module '%ls' loaded from '%ls'", name, fileNameBuf);
    } else {
        LOGI("Module '%ls' not loaded", name);
    }
}

BOOL MyApp::OnIdle(LONG lCount)
{
    BOOL bMore = CWinApp::OnIdle(lCount);

    //if (lCount == 0) {
    //  LOGI("IDLE lcount=%d", lCount);
    //}

    /*
     * If MFC is done, we take a swing.
     */
    if (bMore == false) {
        /* downcast */
        ((MainWindow*)m_pMainWnd)->DoIdle();
    }

    return bMore;
}

// TODO: figure out why we have help topics without matching control ID constants
/*static*/ const DWORD MyApp::PopUpHelpIds[] = {
    IDOK, IDH_IDOK,
    IDCANCEL, IDH_IDCANCEL,
    IDHELP, IDH_IDHELP,
    IDC_NUFXLIB_VERS_TEXT, IDH_NUFXLIB_VERS_TEXT,
    IDC_CONTENT_LIST, IDH_CONTENT_LIST,
    IDC_COL_PATHNAME, IDH_COL_PATHNAME,
    IDC_COL_TYPE, IDH_COL_TYPE,
    IDC_COL_AUXTYPE, IDH_COL_AUXTYPE,
    IDC_COL_MODDATE, IDH_COL_MODDATE,
    IDC_COL_FORMAT, IDH_COL_FORMAT,
    IDC_COL_SIZE, IDH_COL_SIZE,
    IDC_COL_RATIO, IDH_COL_RATIO,
    IDC_COL_PACKED, IDH_COL_PACKED,
    IDC_COL_ACCESS, IDH_COL_ACCESS,
    IDC_COL_DEFAULTS, IDH_COL_DEFAULTS,
    IDC_DEFC_UNCOMPRESSED, IDH_DEFC_UNCOMPRESSED,
    IDC_DEFC_SQUEEZE, IDH_DEFC_SQUEEZE,
    IDC_DEFC_LZW1, IDH_DEFC_LZW1,
    IDC_DEFC_LZW2, IDH_DEFC_LZW2,
    IDC_DEFC_LZC12, IDH_DEFC_LZC12,
    IDC_DEFC_LZC16, IDH_DEFC_LZC16,
    IDC_DEFC_DEFLATE, IDH_DEFC_DEFLATE,
    IDC_DEFC_BZIP2, IDH_DEFC_BZIP2,
    1024, IDH_TOPIC1024,
    IDC_PVIEW_NOWRAP_TEXT, IDH_PVIEW_NOWRAP_TEXT,
    IDC_PVIEW_BOLD_HEXDUMP, IDH_PVIEW_BOLD_HEXDUMP,
    IDC_PVIEW_BOLD_BASIC, IDH_PVIEW_BOLD_BASIC,
    IDC_PVIEW_DISASM_ONEBYTEBRKCOP, IDH_PVIEW_DISASM_ONEBYTEBRKCOP,
    IDC_PVIEW_HIRES_BW, IDH_PVIEW_HIRES_BW,
    IDC_PVIEW_DHR_CONV_COMBO, IDH_PVIEW_DHR_CONV_COMBO,
    IDC_PVIEW_HITEXT, IDH_PVIEW_HITEXT,
    IDC_PVIEW_PASCALTEXT, IDH_PVIEW_PASCALTEXT,
    IDC_PVIEW_APPLESOFT, IDH_PVIEW_APPLESOFT,
    IDC_PVIEW_INTEGER, IDH_PVIEW_INTEGER,
    IDC_PVIEW_HIRES, IDH_PVIEW_HIRES,
    IDC_PVIEW_DHR, IDH_PVIEW_DHR,
    IDC_PVIEW_SHR, IDH_PVIEW_SHR,
    IDC_PVIEW_AWP, IDH_PVIEW_AWP,
    IDC_PVIEW_PRODOSFOLDER, IDH_PVIEW_PRODOSFOLDER,
    IDC_PVIEW_RESOURCES, IDH_PVIEW_RESOURCES,
    IDC_PVIEW_RELAX_GFX, IDH_PVIEW_RELAX_GFX,
    IDC_PVIEW_ADB, IDH_PVIEW_ADB,
    IDC_PVIEW_SCASSEM, IDH_PVIEW_SCASSEM,
    IDC_PVIEW_ASP, IDH_PVIEW_ASP,
    IDC_PVIEW_MACPAINT, IDH_PVIEW_MACPAINT,
    IDC_PVIEW_PASCALCODE, IDH_PVIEW_PASCALCODE,
    IDC_PVIEW_CPMTEXT, IDH_PVIEW_CPMTEXT,
    IDC_PVIEW_GWP, IDH_PVIEW_GWP,
    IDC_PVIEW_DISASM, IDH_PVIEW_DISASM,
    IDC_PVIEW_PRINTSHOP, IDH_PVIEW_PRINTSHOP,
    IDC_PVIEW_TEXT8, IDH_PVIEW_TEXT8,
    IDC_PVIEW_SIZE_EDIT, IDH_PVIEW_SIZE_EDIT,
    IDC_PVIEW_SIZE_SPIN, IDH_PVIEW_SIZE_SPIN,
    IDC_DISKEDIT_DOREAD, IDH_DISKEDIT_DOREAD,
    IDC_DISKEDIT_DOWRITE, IDH_DISKEDIT_DOWRITE,
    IDC_DISKEDIT_TRACK, IDH_DISKEDIT_TRACK,
    IDC_DISKEDIT_TRACKSPIN, IDH_DISKEDIT_TRACK,         // remapped
    IDC_DISKEDIT_SECTOR, IDH_DISKEDIT_TRACK,            // remapped
    IDC_DISKEDIT_SECTORSPIN, IDH_DISKEDIT_TRACK,        // remapped
    IDC_DISKEDIT_OPENFILE, IDH_DISKEDIT_OPENFILE,
    IDC_DISKEDIT_EDIT, IDH_DISKEDIT_EDIT,
    IDC_DISKEDIT_PREV, IDH_DISKEDIT_PREV,
    IDC_DISKEDIT_NEXT, IDH_DISKEDIT_NEXT,
    IDC_STEXT_SECTOR, IDH_DISKEDIT_TRACK,               // remapped
    IDC_STEXT_TRACK, IDH_DISKEDIT_TRACK,                // remapped
    IDC_DISKEDIT_DONE, IDH_DISKEDIT_DONE,
    IDC_DISKEDIT_HEX, IDH_DISKEDIT_HEX,
    IDC_DISKEDIT_SUBVOLUME, IDH_DISKEDIT_SUBVOLUME,
    1082, IDH_TOPIC1082,
    1089, IDH_TOPIC1089,
    IDC_DECONF_FSFORMAT, IDH_DECONF_FSFORMAT,
    IDC_DECONF_SECTORORDER, IDH_DECONF_SECTORORDER,
    IDC_DECONF_PHYSICAL, IDH_DECONF_PHYSICAL,
    IDC_DECONF_FILEFORMAT, IDH_DECONF_FILEFORMAT,
    IDC_DECONF_SOURCE, IDH_DECONF_SOURCE,
    IDC_DISKIMG_VERS_TEXT, IDH_DISKIMG_VERS_TEXT,
    IDC_FVIEW_EDITBOX, IDH_FVIEW_EDITBOX,
    IDC_SELECTED_COUNT, IDH_SELECTED_COUNT,
    1103, IDH_TOPIC1103,
    1105, IDH_TOPIC1105,
    IDC_DECONF_HELP, IDH_DECONF_HELP,
    IDC_SUBV_LIST, IDH_SUBV_LIST,
    IDC_DEFILE_FILENAME, IDH_DEFILE_FILENAME,
    IDC_DEFILE_RSRC, IDH_DEFILE_RSRC,
    IDC_CIDERPRESS_VERS_TEXT, IDH_CIDERPRESS_VERS_TEXT,
    IDC_PREF_TEMP_FOLDER, IDH_PREF_TEMP_FOLDER,
    IDC_CHOOSEDIR_TREE, IDH_CHOOSEDIR_TREE,
    IDC_CHOOSEDIR_PATHEDIT, IDH_CHOOSEDIR_PATHEDIT,
    IDC_CHOOSEDIR_EXPAND_TREE, IDH_CHOOSEDIR_EXPAND_TREE,
    IDC_CHOOSEDIR_PATH, IDH_CHOOSEDIR_PATH,
    IDC_CHOOSEDIR_NEW_FOLDER, IDH_CHOOSEDIR_NEW_FOLDER,
    IDC_PREF_CHOOSE_TEMP_FOLDER, IDH_PREF_CHOOSE_TEMP_FOLDER,
    IDC_FVIEW_FONT, IDH_FVIEW_FONT,
    IDC_FVIEW_NEXT, IDH_FVIEW_NEXT,
    IDC_FVIEW_PREV, IDH_FVIEW_PREV,
    IDC_NEWFOLDER_CURDIR, IDH_NEWFOLDER_CURDIR,
    IDC_NEWFOLDER_NAME, IDH_NEWFOLDER_NAME,
    IDC_EXT_PATH, IDH_EXT_PATH,
    IDC_EXT_CONVEOLTEXT, IDH_EXT_CONVEOLTEXT,
    IDC_EXT_CONVEOLALL, IDH_EXT_CONVEOLALL,
    IDC_EXT_STRIP_FOLDER, IDH_EXT_STRIP_FOLDER,
    IDC_EXT_OVERWRITE_EXIST, IDH_EXT_OVERWRITE_EXIST,
    IDC_EXT_SELECTED, IDH_EXT_SELECTED,
    IDC_EXT_ALL, IDH_EXT_ALL,
    IDC_EXT_REFORMAT, IDH_EXT_REFORMAT,
    IDC_EXT_DATAFORK, IDH_EXT_DATAFORK,
    IDC_EXT_RSRCFORK, IDH_EXT_RSRCFORK,
    IDC_EXT_CONVEOLNONE, IDH_EXT_CONVEOLNONE,
    IDC_EXT_CHOOSE_FOLDER, IDH_EXT_CHOOSE_FOLDER,
    IDC_PROG_ARC_NAME, IDH_PROG_ARC_NAME,
    IDC_PROG_FILE_NAME, IDH_PROG_FILE_NAME,
    IDC_PROG_VERB, IDH_PROG_VERB,
    IDC_PROG_TOFROM, IDH_PROG_TOFROM,
    IDC_PROG_PROGRESS, IDH_PROG_PROGRESS,
    IDC_OVWR_YES, IDH_OVWR_YES,
    IDC_OVWR_YESALL, IDH_OVWR_YESALL,
    IDC_OVWR_NO, IDH_OVWR_NO,
    IDC_OVWR_NOALL, IDH_OVWR_NOALL,
    IDC_OVWR_NEW_INFO, IDH_OVWR_NEW_INFO,
    IDC_OVWR_RENAME, IDH_OVWR_RENAME,
    IDC_OVWR_EXIST_NAME, IDH_OVWR_EXIST_NAME,
    IDC_OVWR_EXIST_INFO, IDH_OVWR_EXIST_INFO,
    IDC_OVWR_NEW_NAME, IDH_OVWR_NEW_NAME,
    IDC_RENOVWR_SOURCE_NAME, IDH_RENOVWR_SOURCE_NAME,
    IDC_RENOVWR_ORIG_NAME, IDH_RENOVWR_ORIG_NAME,
    IDC_RENOVWR_NEW_NAME, IDH_RENOVWR_NEW_NAME,
    IDC_SELECT_ACCEPT, IDH_SELECT_ACCEPT,
    IDC_ADDFILES_PREFIX, IDH_ADDFILES_PREFIX,
    IDC_ADDFILES_INCLUDE_SUBFOLDERS, IDH_ADDFILES_INCLUDE_SUBFOLDERS,
    IDC_ADDFILES_STRIP_FOLDER, IDH_ADDFILES_STRIP_FOLDER,
    IDC_ADDFILES_NOPRESERVE, IDH_ADDFILES_NOPRESERVE,
    IDC_ADDFILES_PRESERVE, IDH_ADDFILES_PRESERVE,
    IDC_ADDFILES_PRESERVEPLUS, IDH_ADDFILES_PRESERVEPLUS,
    IDC_ADDFILES_STATIC1, IDH_ADDFILES_STATIC1,
    IDC_ADDFILES_STATIC2, IDH_ADDFILES_STATIC2,
    IDC_ADDFILES_STATIC3, IDH_ADDFILES_STATIC3,
    IDC_ADDFILES_OVERWRITE, IDH_ADDFILES_OVERWRITE,
    IDC_PREF_SHRINKIT_COMPAT, IDH_PREF_SHRINKIT_COMPAT,
    IDC_USE_SELECTED, IDH_USE_SELECTED,
    IDC_USE_ALL, IDH_USE_ALL,
    IDC_RENAME_OLD, IDH_RENAME_OLD,
    IDC_RENAME_NEW, IDH_RENAME_NEW,
    IDC_RENAME_PATHSEP, IDH_RENAME_PATHSEP,
    IDC_COMMENT_EDIT, IDH_COMMENT_EDIT,
    IDC_COMMENT_DELETE, IDH_COMMENT_DELETE,
    IDC_RECOMP_COMP, IDH_RECOMP_COMP,
    IDC_PREF_ASSOCIATIONS, IDH_PREF_ASSOCIATIONS,
    IDC_ASSOCIATION_LIST, IDH_ASSOCIATION_LIST,
    IDC_REG_COMPANY_NAME, IDH_REG_COMPANY_NAME,
    IDC_REG_EXPIRES, IDH_REG_EXPIRES,
    IDC_ABOUT_ENTER_REG, IDH_ABOUT_ENTER_REG,
    IDC_REGENTER_USER, IDH_REGENTER_USER,
    IDC_REGENTER_COMPANY, IDH_REGENTER_COMPANY,
    IDC_REGENTER_REG, IDH_REGENTER_REG,
    IDC_REG_USER_NAME, IDH_REG_USER_NAME,
    IDC_ZLIB_VERS_TEXT, IDH_ZLIB_VERS_TEXT,
    IDC_EXT_CONVHIGHASCII, IDH_EXT_CONVHIGHASCII,
    IDC_EXT_DISKIMAGE, IDH_EXT_DISKIMAGE,
    IDC_EXT_DISK_2MG, IDH_EXT_DISK_2MG,
    IDC_EXT_ADD_PRESERVE, IDH_EXT_ADD_PRESERVE,
    IDC_EXT_ADD_EXTEN, IDH_EXT_ADD_EXTEN,
    IDC_EXT_CONFIG_PRESERVE, IDH_EXT_CONFIG_PRESERVE,
    IDC_EXT_CONFIG_CONVERT, IDH_EXT_CONFIG_CONVERT,
    IDC_PREF_COERCE_DOS, IDH_PREF_COERCE_DOS,
    IDC_PREF_SPACES_TO_UNDER, IDH_PREF_SPACES_TO_UNDER,
    IDC_REGENTER_USERCRC, IDH_REGENTER_USERCRC,
    IDC_REGENTER_COMPCRC, IDH_REGENTER_COMPCRC,
    IDC_REGENTER_REGCRC, IDH_REGENTER_REGCRC,
    IDC_RENAME_SKIP, IDH_RENAME_SKIP,
    IDC_DECONF_VIEWASBLOCKS, IDH_DECONF_VIEWASBLOCKS,
    IDC_DECONF_VIEWASSECTORS, IDH_DECONF_VIEWASSECTORS,
    IDC_DECONF_VIEWASNIBBLES, IDH_DECONF_VIEWASNIBBLES,
    IDC_DECONF_OUTERFORMAT, IDH_DECONF_OUTERFORMAT,
    IDC_DECONF_VIEWAS, IDH_DECONF_VIEWAS,
    IDC_IMAGE_TYPE, IDH_IMAGE_TYPE,
    IDC_DISKCONV_DOS, IDH_DISKCONV_DOS,
    IDC_DISKCONV_DOS2MG, IDH_DISKCONV_DOS2MG,
    IDC_DISKCONV_PRODOS, IDH_DISKCONV_PRODOS,
    IDC_DISKCONV_PRODOS2MG, IDH_DISKCONV_PRODOS2MG,
    IDC_DISKCONV_NIB, IDH_DISKCONV_NIB,
    IDC_DISKCONV_NIB2MG, IDH_DISKCONV_NIB2MG,
    IDC_DISKCONV_D13, IDH_DISKCONV_D13,
    IDC_DISKCONV_DC42, IDH_DISKCONV_DC42,
    IDC_DISKCONV_SDK, IDH_DISKCONV_SDK,
    IDC_DISKCONV_TRACKSTAR, IDH_DISKCONV_TRACKSTAR,
    IDC_DISKCONV_HDV, IDH_DISKCONV_HDV,
    IDC_DISKCONV_DDD, IDH_DISKCONV_DDD,
    IDC_DISKCONV_GZIP, IDH_DISKCONV_GZIP,
    IDC_DISKEDIT_NIBBLE_PARMS, IDH_DISKEDIT_NIBBLE_PARMS,
    IDC_PROPS_PATHNAME, IDH_PROPS_PATHNAME,
    IDC_PROPS_FILETYPE, IDH_PROPS_FILETYPE,
    IDC_PROPS_AUXTYPE, IDH_PROPS_AUXTYPE,
    IDC_PROPS_ACCESS_R, IDH_PROPS_ACCESS_R,
    IDC_PROPS_ACCESS_W, IDH_PROPS_ACCESS_W,
    IDC_PROPS_ACCESS_N, IDH_PROPS_ACCESS_N,
    IDC_PROPS_ACCESS_D, IDH_PROPS_ACCESS_D,
    IDC_PROPS_ACCESS_I, IDH_PROPS_ACCESS_I,
    IDC_PROPS_ACCESS_B, IDH_PROPS_ACCESS_B,
    IDC_PROPS_MODWHEN, IDH_PROPS_MODWHEN,
    IDC_PROPS_TYPEDESCR, IDH_PROPS_TYPEDESCR,
    1269, IDH_TOPIC1269,
    IDC_CONVFILE_PRESERVEDIR, IDH_CONVFILE_PRESERVEDIR,
    IDC_CONVDISK_140K, IDH_CONVDISK_140K,
    IDC_CONVDISK_800K, IDH_CONVDISK_800K,
    IDC_CONVDISK_1440K, IDH_CONVDISK_1440K,
    IDC_CONVDISK_5MB, IDH_CONVDISK_5MB,
    IDC_CONVDISK_16MB, IDH_CONVDISK_16MB,
    IDC_CONVDISK_20MB, IDH_CONVDISK_20MB,
    IDC_CONVDISK_32MB, IDH_CONVDISK_32MB,
    IDC_CONVDISK_SPECIFY, IDH_CONVDISK_SPECIFY,
    IDC_IMAGE_SIZE_TEXT, IDH_IMAGE_SIZE_TEXT,
    IDC_BULKCONV_PATHNAME, IDH_BULKCONV_PATHNAME,
    IDC_PREF_EXTVIEWER_EXTS, IDH_PREF_EXTVIEWER_EXTS,
    IDC_VOLUME_LIST, IDH_VOLUME_LIST,
    IDC_OPENVOL_READONLY, IDH_OPENVOL_READONLY,
    IDC_VOLUMECOPYPROG_FROM, IDH_VOLUMECOPYPROG_FROM,
    IDC_VOLUMECOPYPROG_TO, IDH_VOLUMECOPYPROG_TO,
    IDC_VOLUMECOPYPROG_PROGRESS, IDH_VOLUMECOPYPROG_PROGRESS,
    IDC_CONVDISK_SPECIFY_EDIT, IDH_CONVDISK_SPECIFY_EDIT,
    IDC_CONVDISK_COMPUTE, IDH_CONVDISK_COMPUTE,
    IDC_DEOW_FILE, IDH_DEOW_FILE,
    IDC_CONVDISK_SPACEREQ, IDH_CONVDISK_SPACEREQ,
    IDC_DEOW_VOLUME, IDH_DEOW_VOLUME,
    IDC_DEOW_CURRENT, IDH_DEOW_CURRENT,
    1306, IDH_TOPIC1306,
    IDC_CONVDISK_VOLNAME, IDH_CONVDISK_VOLNAME,
    IDC_VOLUME_FILTER, IDH_VOLUME_FILTER,
    IDC_VOLUMECOPYSEL_LIST, IDH_VOLUMECOPYSEL_LIST,
    IDC_VOLUEMCOPYSEL_TOFILE, IDH_VOLUEMCOPYSEL_TOFILE,
    IDC_VOLUEMCOPYSEL_FROMFILE, IDH_VOLUEMCOPYSEL_FROMFILE,
    IDC_CREATEFS_DOS32, IDH_CREATEFS_DOS32,
    IDC_CREATEFS_DOS33, IDH_CREATEFS_DOS33,
    IDC_CREATEFS_PRODOS, IDH_CREATEFS_PRODOS,
    IDC_CREATEFS_PASCAL, IDH_CREATEFS_PASCAL,
    IDC_CREATEFS_HFS, IDH_CREATEFS_HFS,
    IDC_CREATEFS_BLANK, IDH_CREATEFS_BLANK,
    1320, IDH_TOPIC1320,
    IDC_CREATEFSDOS_ALLOCDOS, IDH_CREATEFSDOS_ALLOCDOS,
    IDC_CREATEFSDOS_VOLNUM, IDH_CREATEFSDOS_VOLNUM,
    IDC_CREATEFSPRODOS_VOLNAME, IDH_CREATEFSPRODOS_VOLNAME,
    IDC_CREATEFSPASCAL_VOLNAME, IDH_CREATEFSPASCAL_VOLNAME,
    IDC_ASPI_VERS_TEXT, IDH_ASPI_VERS_TEXT,
    IDC_PREF_SUCCESS_BEEP, IDH_PREF_SUCCESS_BEEP,
    IDC_ADD_TARGET_TREE, IDH_ADD_TARGET_TREE,
    IDC_AIDISK_SUBVOLSEL, IDH_AIDISK_SUBVOLSEL,
    IDC_AIDISK_NOTES, IDH_AIDISK_NOTES,
    IDC_AI_FILENAME, IDH_AI_FILENAME,
    IDC_AIBNY_RECORDS, IDH_AIBNY_RECORDS,
    IDC_AINUFX_FORMAT, IDH_AINUFX_FORMAT,
    IDC_AINUFX_RECORDS, IDH_AINUFX_RECORDS,
    IDC_AINUFX_MASTERVERSION, IDH_AINUFX_MASTERVERSION,
    IDC_AINUFX_CREATEWHEN, IDH_AINUFX_CREATEWHEN,
    IDC_AINUFX_MODIFYWHEN, IDH_AINUFX_MODIFYWHEN,
    IDC_AINUFX_JUNKSKIPPED, IDH_AINUFX_JUNKSKIPPED,
    IDC_AIDISK_OUTERFORMAT, IDH_AIDISK_OUTERFORMAT,
    IDC_AIDISK_FILEFORMAT, IDH_AIDISK_FILEFORMAT,
    IDC_AIDISK_PHYSICALFORMAT, IDH_AIDISK_PHYSICALFORMAT,
    IDC_AIDISK_SECTORORDER, IDH_AIDISK_SECTORORDER,
    IDC_AIDISK_FSFORMAT, IDH_AIDISK_FSFORMAT,
    IDC_AIDISK_FILECOUNT, IDH_AIDISK_FILECOUNT,
    IDC_AIDISK_CAPACITY, IDH_AIDISK_CAPACITY,
    IDC_AIDISK_FREESPACE, IDH_AIDISK_FREESPACE,
    IDC_AIDISK_DAMAGED, IDH_AIDISK_DAMAGED,
    IDC_AIDISK_WRITEABLE, IDH_AIDISK_WRITEABLE,
    IDC_PDISK_CONFIRM_FORMAT, IDH_PDISK_CONFIRM_FORMAT,
    IDC_PDISK_PRODOS_ALLOWLOWER, IDH_PDISK_PRODOS_ALLOWLOWER,
    IDC_PDISK_PRODOS_USESPARSE, IDH_PDISK_PRODOS_USESPARSE,
    IDC_FVIEW_PRINT, IDH_FVIEW_PRINT,
    IDC_CREATESUBDIR_BASE, IDH_CREATESUBDIR_BASE,
    IDC_CREATESUBDIR_NEW, IDH_CREATESUBDIR_NEW,
    IDC_RENAMEVOL_TREE, IDH_RENAMEVOL_TREE,
    IDC_RENAMEVOL_NEW, IDH_RENAMEVOL_NEW,
    IDC_ADDFILES_CONVEOLNONE, IDH_ADDFILES_CONVEOLNONE,
    IDC_ADDFILES_CONVEOLTEXT, IDH_ADDFILES_CONVEOLTEXT,
    IDC_ADDFILES_CONVEOLALL, IDH_ADDFILES_CONVEOLALL,
    IDC_ADDFILES_STATIC4, IDH_ADDFILES_STATIC4,
    IDC_PROPS_CREATEWHEN, IDH_PROPS_CREATEWHEN,
    IDC_EOLSCAN_CR, IDH_EOLSCAN_CR,
    IDC_EOLSCAN_LF, IDH_EOLSCAN_LF,
    IDC_EOLSCAN_CRLF, IDH_EOLSCAN_CRLF,
    IDC_EOLSCAN_CHARS, IDH_EOLSCAN_CHARS,
    IDC_PREF_PASTE_JUNKPATHS, IDH_PREF_PASTE_JUNKPATHS,
    IDC_EXT_CONVEOLTYPE, IDH_EXT_CONVEOLTYPE,
    IDC_ADDFILES_CONVEOLTYPE, IDH_ADDFILES_CONVEOLTYPE,
    IDC_TWOIMG_LOCKED, IDH_TWOIMG_LOCKED,
    IDC_TWOIMG_DOSVOLSET, IDH_TWOIMG_DOSVOLSET,
    IDC_TWOIMG_DOSVOLNUM, IDH_TWOIMG_DOSVOLNUM,
    IDC_TWOIMG_COMMENT, IDH_TWOIMG_COMMENT,
    IDC_TWOIMG_CREATOR, IDH_TWOIMG_CREATOR,
    IDC_TWOIMG_VERSION, IDH_TWOIMG_VERSION,
    IDC_TWOIMG_FORMAT, IDH_TWOIMG_FORMAT,
    IDC_TWOIMG_BLOCKS, IDH_TWOIMG_BLOCKS,
    IDC_FVIEW_DATA, IDH_FVIEW_DATA,
    IDC_FVIEW_RSRC, IDH_FVIEW_RSRC,
    IDC_FVIEW_CMMT, IDH_FVIEW_CMMT,
    IDC_FVIEW_FORMATSEL, IDH_FVIEW_FORMATSEL,
    IDC_FVIEW_FMT_HEX, IDH_FVIEW_FMT_HEX,
    IDC_FVIEW_FMT_RAW, IDH_FVIEW_FMT_RAW,
    IDC_FVIEW_FMT_BEST, IDH_FVIEW_FMT_BEST,
    IDC_PDISK_OPENVOL_RO, IDH_PDISK_OPENVOL_RO,
    IDC_EOLSCAN_HIGHASCII, IDH_EOLSCAN_HIGHASCII,
    IDC_CASSETTE_LIST, IDH_CASSETTE_LIST,
    IDC_IMPORT_CHUNK, IDH_IMPORT_CHUNK,
    IDC_CASSETTE_ALG, IDH_CASSETTE_ALG,
    IDC_CASSETTE_INPUT, IDH_CASSETTE_INPUT,
    IDC_CASSIMPTARG_FILENAME, IDH_CASSIMPTARG_FILENAME,
    IDC_CASSIMPTARG_BAS, IDH_CASSIMPTARG_BAS,
    IDC_CASSIMPTARG_INT, IDH_CASSIMPTARG_INT,
    IDC_CASSIMPTARG_BIN, IDH_CASSIMPTARG_BIN,
    IDC_CASSIMPTARG_BINADDR, IDH_CASSIMPTARG_BINADDR,
    IDC_CASSIMPTARG_RANGE, IDH_CASSIMPTARG_RANGE,
    IDC_CLASH_RENAME, IDH_CLASH_RENAME,
    IDC_CLASH_SKIP, IDH_CLASH_SKIP,
    IDC_CLASH_WINNAME, IDH_CLASH_WINNAME,
    IDC_CLASH_STORAGENAME, IDH_CLASH_STORAGENAME,
    IDC_PREF_REDUCE_SHK_ERROR_CHECKS, IDH_PREF_REDUCE_SHK_ERROR_CHECKS,
    IDC_IMPORT_BAS_RESULTS, IDH_IMPORT_BAS_RESULTS,
    IDC_IMPORT_BAS_SAVEAS, IDH_IMPORT_BAS_SAVEAS,
    IDC_FVIEW_FIND, IDH_FVIEW_FIND,
    IDC_CREATEFSHFS_VOLNAME, IDH_CREATEFSHFS_VOLNAME,
    IDC_PROPS_HFS_FILETYPE, IDH_PROPS_HFS_FILETYPE,
    IDC_PROPS_HFS_AUXTYPE, IDH_PROPS_HFS_AUXTYPE,
    IDC_PROPS_HFS_MODE, IDH_PROPS_HFS_MODE,
    IDC_PROPS_HFS_LABEL, IDH_PROPS_HFS_LABEL,
    IDC_PASTE_SPECIAL_COUNT, IDH_PASTE_SPECIAL_COUNT,
    IDC_PASTE_SPECIAL_PATHS, IDH_PASTE_SPECIAL_PATHS,
    IDC_PASTE_SPECIAL_NOPATHS, IDH_PASTE_SPECIAL_NOPATHS,
    IDC_PROGRESS_COUNTER_COUNT, IDH_PROGRESS_COUNTER_COUNT,
    IDC_PROGRESS_COUNTER_DESC, IDH_PROGRESS_COUNTER_DESC,
    IDC_PDISK_OPENVOL_PHYS0, IDH_PDISK_OPENVOL_PHYS0,
    IDC_PREF_SHK_BAD_MAC, IDH_PREF_SHK_BAD_MAC,
    0
};

/*static*/ BOOL MyApp::HandleHelpInfo(HELPINFO* lpHelpInfo)
{
    CString path(gMyApp.m_pszHelpFilePath);
    path += "::/PopUp.txt";

    LOGD("HandleHelpInfo ID=%d", lpHelpInfo->iCtrlId);
    ::HtmlHelp((HWND) lpHelpInfo->hItemHandle, path,
        HH_TP_HELP_WM_HELP, (DWORD) PopUpHelpIds);

    return TRUE;
}

/*static*/ void MyApp::HandleHelp(CWnd* pWnd, DWORD topicId)
{
    // The CWnd#HtmlHelp() function is insisting on using the top-level
    // parent, but if we do that with the Add Files custom file dialog
    // then the help window pops up behind the app rather than in front.
    CWnd* pParent = pWnd->GetTopLevelParent();
    LOGD("HandleHelp ID=%lu pWnd=%p parent=%p", topicId, pWnd, pParent);
    ::HtmlHelp(pWnd->m_hWnd, gMyApp.m_pszHelpFilePath,
        HH_HELP_CONTEXT, topicId);
}
