unit uLogMgr;

interface
uses
  Windows;
const
  LOGMGR_DLL_NAME = 'logmgr.dll';
  MAX_LOG_NODE_NAME_SIZE = 256;
type
  PPInteger = ^PInteger;
  PPLOG_NODE_ITEM = ^PLOG_NODE_ITEM;
  PLOG_NODE_ITEM = ^LOG_NODE_ITEM;
  LOG_NODE_ITEM = packed record
     nNodeId: integer;
     szNodeName: array[0..MAX_LOG_NODE_NAME_SIZE - 1] of Char;
  end;

  PPWISDOM_ITEM = ^PWISDOM_ITEM;
  PWISDOM_ITEM  = ^WISDOM_ITEM;
  WISDOM_ITEM = packed record
    id: integer;
    nFromId: integer;
    nRow: integer;
    szWisdom: array[0..255] of Char;
  end;

  PPLOG_COMMENT_ITEM = ^PLOG_COMMENT_ITEM;
  PLOG_COMMENT_ITEM = ^LOG_COMMENT_ITEM;
  LOG_COMMENT_ITEM = packed record
    nStart: integer;
    nLength: integer;
    nCommentId: integer;
  end;

  function  LM_InitLogMgr(const szLogFileName: PChar): Boolean; stdcall;
  procedure LM_DestroyLogMgr();stdcall;
  function  LM_DeleteString(pBuf: PChar): Boolean;stdcall;
  function  LM_DeleteInt(pInt: PInteger): Boolean;stdcall;
  function  LM_AddLog(nLogId: PInteger; const nParentLogId: integer;
            const szTitle: PChar): Boolean; stdcall;
  function  LM_UpdateLog(const nLogId: integer; const pBuf: PChar; const nSize: integer;
	            const szAffix: PChar; bSaveLog: Boolean): Boolean; stdcall;
  function  LM_UpdateLogPlainText(const nLogId: integer; const pBuf: PChar; const nSize: integer): Boolean; stdcall;
  function  LM_IncReadLogTime(const nLogId: integer): Boolean; stdcall;
  function  LM_UpdateLogLines(const nLogId: integer; const nLines: integer): Boolean; stdcall;
  function  LM_GetLog(const nLogId: integer; szTitle: PPChar;  pBuf: PPChar;
              nBufSize: PInteger; szCreateDate: PChar; szLastModiDate: PChar;
              nReadTimes: PInteger; nCurrLines: PInteger): Boolean; stdcall;
  function  LM_DeleteLog(const nLogId: integer): Boolean;stdcall;
  function  LM_UpdateTitle(const nLogId: integer; const szTitle: PChar): Boolean;stdcall;
  function  LM_UpdateParentLogId(const nLogId: integer; const nParentLogId: integer): Boolean; stdcall;
  function  LM_DeleteNodeItems(pItems: PLOG_NODE_ITEM): Boolean; stdcall;
  function  LM_DeleteWisdoms(pItems: PWISDOM_ITEM ): Boolean; stdcall;
  function  LM_GetChildNodes(const nParentLog: integer; ppItems: PPLOG_NODE_ITEM;
              nCount: PInteger): Boolean;stdcall;
  function  LM_SearchText(const szSubText: PChar; ppItems: PPLOG_NODE_ITEM; nCount: PInteger): Boolean; stdcall;
  function  LM_DeleteAffix(const nAffixId: integer): Boolean;stdcall;
  function  LM_AddAffix(const nLogId: integer; const szAffixName: PChar): Boolean;stdcall;
  function  LM_GetAffixList(const nLogId: integer; nAffixList: PPInteger;
               nAffixCount: PInteger): Boolean; stdcall;
  function  LM_GetAffixInfo(const nAffixId: integer; szAffixName: PChar): Boolean;stdcall;
  function  LM_SaveAsAffix(const nAffixId: integer; const szFileName: PChar): Boolean; stdcall;
  function  LM_AddWisdom(const nFromLogId: integer; const nRow: integer; const szWisdom: PChar): Boolean; stdcall;
  function  LM_GetAllWisdom(ppItems: PPWISDOM_ITEM; nCount: PInteger): Boolean; stdcall;
  function  LM_AddComment(nCommentId: PInteger; const nLogId: integer; const nStart: integer; const nLength: integer;
		             const szComment: PChar; const nCommentSize: integer): Boolean; stdcall;
  function  LM_GetComments(const nLogId: integer; ppItems: PPLOG_COMMENT_ITEM; nCount: PInteger): Boolean; stdcall;
  function  LM_GetCommentText(const nCommentId: integer; pBuf: PPChar; nBufSize: PInteger): Boolean; stdcall;
  function  LM_DeleteComment(const nCommentId: integer): Boolean; stdcall;
  function  LM_DeleteCommentItems(pItems: PLOG_COMMENT_ITEM): Boolean; stdcall;
  function  LM_DeleteCommentBySel(const nLogId: integer; const nStart: integer; const nLength: integer): Boolean; stdcall;
  function  LM_AddMemoLog(const szMemo: PChar): Boolean; stdcall;
  function  LM_AddPersonAccount(const szIncoming: PChar; const szPayout: PChar; const szAddr: PChar; const szItem: PChar;
		                    const szComment: PChar; const szUserName: PChar; const szUserDate: PChar): Boolean; stdcall;
  function  LM_AddAutoOil(const szPrice: PChar; const szCapacity: PChar; const szTotal: PChar; const szAddDate: PChar;
		             const szKilometre: PChar; const szComment: PChar): Boolean; stdcall;
  function  LM_AddAutoFee(const szPrice: PChar; const szCount: PChar; const szTotal: PChar; const  szFeeDate: PChar;
		             const szComment: PChar): Boolean; stdcall;
  function  LM_CheckPassword(const szPassword: PChar): Boolean; stdcall;
  function  LM_SetPassword(const szOldPassword: PChar; const szNewPassword: PChar): Boolean; stdcall;
  function  LM_AddWorkFlow(const szTimeSect: PChar; const szMemo: PChar): Boolean; stdcall;
implementation

function  LM_InitLogMgr; external LOGMGR_DLL_NAME name 'LM_InitLogMgr';
procedure LM_DestroyLogMgr; external LOGMGR_DLL_NAME name 'LM_DestroyLogMgr';
function  LM_DeleteString; external LOGMGR_DLL_NAME name 'LM_DeleteString';
function  LM_DeleteInt; external LOGMGR_DLL_NAME name 'LM_DeleteInt';
function  LM_AddLog; external LOGMGR_DLL_NAME name 'LM_AddLog';
function  LM_UpdateLog; external LOGMGR_DLL_NAME name 'LM_UpdateLog';
function  LM_UpdateLogPlainText; external LOGMGR_DLL_NAME name 'LM_UpdateLogPlainText';
function  LM_IncReadLogTime; external LOGMGR_DLL_NAME name 'LM_IncReadLogTime';
function  LM_UpdateLogLines; external LOGMGR_DLL_NAME name 'LM_UpdateLogLines';
function  LM_UpdateTitle;external LOGMGR_DLL_NAME name 'LM_UpdateTitle';
function  LM_UpdateParentLogId; external LOGMGR_DLL_NAME name 'LM_UpdateParentLogId';
function  LM_GetLog; external LOGMGR_DLL_NAME name 'LM_GetLog';
function  LM_SearchText; external LOGMGR_DLL_NAME name 'LM_SearchText';
function  LM_GetChildNodes; external LOGMGR_DLL_NAME name 'LM_GetChildNodes';
function  LM_DeleteLog; external LOGMGR_DLL_NAME name 'LM_DeleteLog';
function  LM_DeleteNodeItems; external LOGMGR_DLL_NAME name 'LM_DeleteNodeItems';
function  LM_DeleteAffix; external LOGMGR_DLL_NAME name 'LM_DeleteAffix';
function  LM_AddAffix; external LOGMGR_DLL_NAME name 'LM_AddAffix';
function  LM_GetAffixList; external LOGMGR_DLL_NAME name 'LM_GetAffixList';
function  LM_GetAffixInfo; external LOGMGR_DLL_NAME name 'LM_GetAffixInfo';
function  LM_SaveAsAffix; external LOGMGR_DLL_NAME name 'LM_SaveAsAffix';
function  LM_AddWisdom; external LOGMGR_DLL_NAME name 'LM_AddWisdom';
function  LM_GetAllWisdom; external LOGMGR_DLL_NAME name 'LM_GetAllWisdom';
function  LM_DeleteWisdoms; external LOGMGR_DLL_NAME name 'LM_DeleteWisdoms';
function  LM_AddComment; external LOGMGR_DLL_NAME name 'LM_AddComment';
function  LM_GetComments; external LOGMGR_DLL_NAME name 'LM_GetComments';
function  LM_GetCommentText; external LOGMGR_DLL_NAME name 'LM_GetCommentText';
function  LM_DeleteComment; external LOGMGR_DLL_NAME name 'LM_DeleteComment';
function  LM_DeleteCommentItems; external LOGMGR_DLL_NAME name 'LM_DeleteCommentItems';
function  LM_DeleteCommentBySel; external LOGMGR_DLL_NAME name 'LM_DeleteCommentBySel';
function  LM_AddMemoLog; external LOGMGR_DLL_NAME name 'LM_AddMemoLog';
function  LM_AddPersonAccount; external LOGMGR_DLL_NAME name 'LM_AddPersonAccount';
function  LM_AddAutoOil; external LOGMGR_DLL_NAME name 'LM_AddAutoOil';
function  LM_AddAutoFee; external LOGMGR_DLL_NAME name 'LM_AddAutoFee';
function  LM_CheckPassword; external LOGMGR_DLL_NAME name 'LM_CheckPassword';
function  LM_SetPassword; external LOGMGR_DLL_NAME name 'LM_SetPassword';
function  LM_AddWorkFlow; external LOGMGR_DLL_NAME name 'LM_AddWorkFlow';
end.
