unit SyncPath;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Buttons;

const
  SMARTSYNC_TYPE_ESTABLISH  = 1;
  SMARTSYNC_TYPE_ANALYSE    = 2;
  SMARTSYNC_TYPE_SYNC       = 3;
  SMARTSYNC_TYPE_COMPLETE   = 99;
type
  TSyncCallBack = procedure(nType: integer; const szSubFile: PChar; const szNewFileSize: PChar;
	                          const szLastModiTime: PChar; const szOldFileSize: PChar; const szOldModiTime: PChar;
			                      nDirCount: integer; nFileCount: integer);stdcall;
  TfrmSyncPath = class(TForm)
    edtSrcPath: TEdit;
    Label1: TLabel;
    edtDestPath: TEdit;
    sbSrcPath: TSpeedButton;
    sbDestPath: TSpeedButton;
    btnIndex: TBitBtn;
    btnAnalyse: TBitBtn;
    btnSyc: TBitBtn;
    mmInfo: TMemo;
    procedure FormCanResize(Sender: TObject; var NewWidth,
      NewHeight: Integer; var Resize: Boolean);
    procedure sbSrcPathClick(Sender: TObject);
    procedure sbDestPathClick(Sender: TObject);
    procedure btnIndexClick(Sender: TObject);
    procedure btnAnalyseClick(Sender: TObject);
    procedure btnSycClick(Sender: TObject);
  private
    function BrowserDirectory(const strCaption, S: string): string;
    { Private declarations }
  public
    { Public declarations }
  end;

  TSyncThread  = class(TThread)
  private
    FFrm: TfrmSyncPath;
    FSyncType: integer;
    FCompleted: Boolean;
  protected
    procedure Execute;override;
    procedure DisplayInfo;
    procedure ThreadDisplayInfo(bComplete: Boolean);
  public
    FSubFileName: string;
    FNewFileSize: string;
    FLastModiTime: string;
    FOldFileSize: string;
    FOldModiTime: string;
    FDirCount: integer;
    FFileCount: integer;

    constructor Create(Frm: TfrmSyncPath; nType: integer);
  end;

procedure ShowSyncPath;



implementation
uses
  ShellApi, ShlObj;

var
  FCurrThread: TSyncThread;

function SmartSyncPath(const szSrcPath: PChar; const szDestPath: PChar;
	                     pCallBack: TSyncCallBack): BOOL; stdcall; external 'smartsync.dll' name 'SmartSyncPath';
function EstablishSyncPath(const szPath: PChar; pCallBack: TSyncCallBack): BOOL; stdcall; external 'smartsync.dll' name 'EstablishSyncPath';
function AnalyseSyncPath(const szSrcPath: PChar; const szDestPath: PChar;
                         pCallBack: TSyncCallBack): BOOL; stdcall; external 'smartsync.dll' name 'AnalyseSyncPath';

procedure  SyncCallback(nType: integer; const szSubFile: PChar; const szNewFileSize: PChar;
	                      const szLastModiTime: PChar; const szOldFileSize: PChar; const szOldModiTime: PChar;
                        nDirCount: integer; nFileCount: integer);stdcall;
begin
  if nType = SMARTSYNC_TYPE_COMPLETE then
  begin
    if FCurrThread <> nil then
    begin
      FCurrThread.FDirCount := nDirCount;
      FCurrThread.FFileCount := nFileCount;
      FCurrThread.ThreadDisplayInfo(True);
      FCurrThread := nil;
    end;
  end else
  begin
    if FCurrThread <> nil then
    begin
      if szSubFile <> nil then
         FCurrThread.FSubFileName := szSubFile
      else
         FCurrThread.FSubFileName := '';
      if szNewFileSize <> nil then
         FCurrThread.FNewFileSize := szNewFileSize
      else
         FCurrThread.FNewFileSize := '0';
      if szLastModiTime <> nil then
         FCurrThread.FLastModiTime := szLastModiTime
      else
         FCurrThread.FLastModiTime := '';
      if szOldFileSize <> nil then
         FCurrThread.FOldFileSize := szOldFileSize
      else
         FCurrThread.FOldFileSize := '0';
      if szOldModiTime <> nil then
         FCurrThread.FOldModiTime := szOldModiTime
      else
         FCurrThread.FOldModiTime := '';

      FCurrThread.FDirCount := nDirCount;
      FCurrThread.FFileCount := nFileCount;
      FCurrThread.ThreadDisplayInfo(False);
    end;
  end;
end;
{$R *.dfm}
procedure ShowSyncPath;
var
  Frm: TfrmSyncPath;
begin
  FCurrThread := nil;
  Frm := TfrmSyncPath.Create(Application);
  frm.ShowModal;
  frm.Free;
end;


{ TSyncThread }

constructor TSyncThread.Create(Frm: TfrmSyncPath; nType: integer);
begin
  inherited Create(TRUE);
  FFrm := Frm;
  FCompleted := False;
  FSyncType := nType;
  FreeOnTerminate := True;
  Resume;
end;

procedure TSyncThread.DisplayInfo;
var
  Str: string;
begin
  case FSyncType of
       SMARTSYNC_TYPE_ESTABLISH:
                              if FCompleted then
                                 Str := '创建索引完毕, 目录数:' + IntToStr(FDirCount) +
                                         ' 文件数:' + IntToStr(FFileCount)
                              else
                                 Str := '';
       SMARTSYNC_TYPE_ANALYSE:
                              if FCompleted then
                                 Str := '比较目录完毕, 目录数:' + IntToStr(FDirCount) +
                                         ' 文件数:' + IntToStr(FFileCount)
                              else
                                 Str := '文件: ' + FSubFileName  + ' 大小:' + FNewFileSize
                                     + ' 最后修改时间:' + FLastModiTime + ' 原有版本时间:' + FOldModiTime;
       SMARTSYNC_TYPE_SYNC:
                              if FCompleted then
                                 Str := '同步完毕, 目录数:' + IntToStr(FDirCount) +
                                         ' 文件数:' + IntToStr(FFileCount)
                              else
                                 Str := '文件: ' + FSubFileName  + ' 大小:' + FNewFileSize
                                     + ' 最后修改时间:' + FLastModiTime + ' 原有版本时间:' + FOldModiTime;
  end;
  if (FFrm <> nil) and (Str <> '') then
     FFrm.mmInfo.Lines.Add(Str);
end;

procedure TSyncThread.Execute;
begin
  case FSyncType of
       SMARTSYNC_TYPE_ESTABLISH:
                               EstablishSyncPath(PChar(FFrm.edtDestPath.Text),  SyncCallback);
       SMARTSYNC_TYPE_ANALYSE:
                               AnalyseSyncPath(PChar(FFrm.edtSrcPath.Text), PChar(FFrm.edtDestPath.Text),
                                  SyncCallBack);
       SMARTSYNC_TYPE_SYNC:
                               SmartSyncPath(PChar(FFrm.edtSrcPath.Text), PChar(FFrm.edtDestPath.Text),
                                  SyncCallBack);
  end;

end;

procedure TfrmSyncPath.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
begin
  Resize := False;
end;

procedure TfrmSyncPath.sbSrcPathClick(Sender: TObject);
var
  Str: string;
begin
   Str := BrowserDirectory('选择源目录', edtSrcPath.Text);
   if Str <> '' then
      edtSrcPath.Text := Str;
end;

function TfrmSyncPath.BrowserDirectory(const strCaption: string; const S: string): string;
var
  lpItemID : PItemIDList;
  BrowseInfo : TBrowseInfo;
  DisplayName : array[0..MAX_PATH] of char;
  TempPath : array[0..MAX_PATH] of char;
begin
  Result := '';
  FillChar(BrowseInfo, sizeof(TBrowseInfo), #0);
  BrowseInfo.hwndOwner := Handle;
  BrowseInfo.pszDisplayName := @DisplayName;
  BrowseInfo.lpszTitle := PChar(strCaption);
  BrowseInfo.ulFlags := BIF_RETURNONLYFSDIRS;
  lpItemID := SHBrowseForFolder(BrowseInfo);
  if lpItemId <> nil then
  begin
     SHGetPathFromIDList(lpItemID, TempPath);
     Result := TempPath;
     GlobalFreePtr(lpItemID);
  end;
end;

procedure TfrmSyncPath.sbDestPathClick(Sender: TObject);
var
  Str: string;
begin
  Str := BrowserDirectory('选择目标路径', edtDestPath.Text);
  if Str <> '' then
     edtDestPath.Text := Str;
end;

procedure TfrmSyncPath.btnIndexClick(Sender: TObject);
begin
  if (FCurrThread = nil) then
      FCurrThread := TSyncThread.Create(Self, SMARTSYNC_TYPE_ESTABLISH);
end;

procedure TfrmSyncPath.btnAnalyseClick(Sender: TObject);
begin
  mmInfo.Lines.Clear;
  mmInfo.Lines.Add('分析结果:');
  if (FCurrThread = nil) then
      FCurrThread := TSyncThread.Create(Self, SMARTSYNC_TYPE_ANALYSE);
end;

procedure TfrmSyncPath.btnSycClick(Sender: TObject);
begin
  mmInfo.Lines.Clear;
  mmInfo.Lines.Add('同步结果:');
  if (FCurrThread = nil) then
      FCurrThread := TSyncThread.Create(Self, SMARTSYNC_TYPE_SYNC);
end;

procedure TSyncThread.ThreadDisplayInfo(bComplete: Boolean);
begin
  FCompleted := bComplete;
  Synchronize(DisplayInfo);
end;

end.
