unit FormMain;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, 
  tlHelp32,
  ShellAPI, ExtCtrls, StdCtrls;

type
  TForm1 = class(TForm)
    Timer1: TTimer;
    Label1: TLabel;
    procedure Timer1Timer(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

uses Unit2;

{$R *.dfm}

function CommFuns_FindHandle(exename: String;isKill:integer): DWORD;
type
  TProcessInfo   =   Record
  ExeFile   :   String;
  ProcessID   :   DWORD;
  end;
  pProcessInfo   =   ^TProcessInfo;
var
  //p   :   pProcessInfo;
  ContinueLoop:BOOL;
  FSnapshotHandle:THandle;
  FProcessEntry32:TProcessEntry32;
begin
  result:=0;
  FSnapshotHandle:=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
  FProcessEntry32.dwSize:=Sizeof(FProcessEntry32);
  ContinueLoop:=Process32First(FSnapshotHandle,FProcessEntry32);
  while   integer(ContinueLoop)<>0   do
  begin
   // New(p);
   // p.ExeFile   :=   FProcessEntry32.szExeFile;
   // p.ProcessID   :=   FProcessEntry32.th32ProcessID;
    if lowercase(FProcessEntry32.szExeFile)=lowercase(exename) then begin
      if isKill=1 then
        TerminateProcess(OpenProcess(PROCESS_TERMINATE, BOOL(0),FProcessEntry32.th32ProcessID), 0);
      result:=FProcessEntry32.th32ProcessID;
      //break;
    end;
    ContinueLoop:=Process32Next(FSnapshotHandle,FProcessEntry32);
  end;

end;


{*******************************************************************************
  删除整个目录
}
procedure DelFolderTree(const   Directory:   TFileName);
var
    DrivesPathsBuff:   array[0..1024]   of   char;
    DrivesPaths:   string;
    len:   longword;
    ShortPath:   array[0..MAX_PATH]   of   char;
    dir:   TFileName;
procedure   rDelTree(const   Directory:   TFileName);
var
    SearchRec:   TSearchRec;
    Attributes:   LongWord;
    ShortName,   FullName:   TFileName;
    pname:   pchar;
begin
    if   FindFirst(Directory   +   '*',   faAnyFile   and   not   faVolumeID,
        SearchRec)   =   0   then   begin
        try
            repeat   //   检测所有的文件和目录
                if   SearchRec.FindData.cAlternateFileName[0]   =   #0   then
                    ShortName   :=   SearchRec.Name
                else
                    ShortName   :=   SearchRec.FindData.cAlternateFileName;
                FullName   :=   Directory   +   ShortName;
                if   (SearchRec.Attr   and   faDirectory)   <>   0   then   begin
                    //   是一个目录
                    if   (ShortName   <>   '.')   and   (ShortName   <>   '..')   then
                        rDelTree(FullName   +   '\');
                end   else   begin
                    //   是一个文件
                    pname   :=   PChar(FullName);
                    Attributes   :=   GetFileAttributes(pname);
                    if   Attributes   =   $FFFFFFFF   then
                        raise   EInOutError.Create(SysErrorMessage(GetLastError));
                    if   (Attributes   and   FILE_ATTRIBUTE_READONLY)   <>   0   then
                        SetFileAttributes(pname,   Attributes   and   not
                            FILE_ATTRIBUTE_READONLY);
                    //**modified by cxd 2008.09.12
                    //**description(修改Bug过程中新发现的Bug，当一用户登陆后退出再立即登陆回导致文件共享冲突)
                    //**begin
                    {
                    if   Windows.DeleteFile(pname)   =   False   then
                      raise   EInOutError.Create(SysErrorMessage(GetLastError));
                    }
                    //**sign
                    try
                      if   Windows.DeleteFile(pname)   =   False   then
                        raise   EInOutError.Create(SysErrorMessage(GetLastError));
                    except

                    end;
                 //**end
                end;
            until   FindNext(SearchRec)   <>   0;
        except
            FindClose(SearchRec);
            raise;
        end;
        FindClose(SearchRec);
    end;
    if   Pos(#0   +   Directory   +   #0,   DrivesPaths)   =   0   then   begin
        //   如果不是根目录，就删除
        pname   :=   PChar(Directory);
        Attributes   :=   GetFileAttributes(pname);
        if   Attributes   =   $FFFFFFFF   then
            raise   EInOutError.Create(SysErrorMessage(GetLastError));
        if   (Attributes   and   FILE_ATTRIBUTE_READONLY)   <>   0   then
            SetFileAttributes(pname,   Attributes   and   not
                FILE_ATTRIBUTE_READONLY);
        if   Windows.RemoveDirectory(pname)   =   False   then   begin
            //raise   EInOutError.Create(SysErrorMessage(GetLastError));
        end;
    end;
end;
//   ----------------
begin
    DrivesPathsBuff[0]   :=   #0;
    len   :=   GetLogicalDriveStrings(1022,   @DrivesPathsBuff[1]);
    if   len   =   0   then
        raise   EInOutError.Create(SysErrorMessage(GetLastError));
    SetString(DrivesPaths,   DrivesPathsBuff,   len   +   1);
    DrivesPaths   :=   Uppercase(DrivesPaths);
    len   :=   GetShortPathName(PChar(Directory),   ShortPath,   MAX_PATH);
    if   len   =   0   then
        //raise   EInOutError.Create(SysErrorMessage(GetLastError));
        exit;
    SetString(dir,   ShortPath,   len);
    dir   :=   Uppercase(dir);
    rDelTree(IncludeTrailingBackslash(dir));
end;





  function   CopyDirectory(SourcePath,TargetPath:   string):   Boolean;   //copy整个目录
  var   
      search:   TSearchRec;   
      ret:   integer;   
      key:   string;   
  begin   
      if   TargetPath[Length(TargetPath)]   <>   '\'   then   
          TargetPath   :=   TargetPath   +   '\';   
      if   SourcePath[Length(SourcePath)]   <>   '\'   then   
          SourcePath   :=   SourcePath   +   '\';   
    
      key   :=   SourcePath   +   '*.*';   
      ret   :=   findFirst(key,   faanyfile,   search);   
      while   ret   =   0   do   begin   
          if   ((search.Attr   and   fadirectory)   =   faDirectory)   
              then   begin   
              if   (Search.Name   <>   '.')   and   (Search.name   <>   '..')   then   
                  CopyDirectory(SourcePath   +   Search.name,TargetPath+search.Name);   
          end   else   begin   
              if   ((search.attr   and   fadirectory)   <>   fadirectory)   then   
              begin   
                  if   not   DirectoryExists(TargetPath)   then   
                  ForceDirectories(targetPath);   
                  copyfile(pchar(SourcePath+search.name),pchar(targetPath+search.Name),False);   
                  end;   
          end;   
          ret   :=   FindNext(search);   
      end;   
      findClose(search);   
      result   :=   True;   
  end;


function   WinExecAndWait(FileName:String;   Visibility   :   integer):Thandle;
  var   
    zAppName:array[0..512]   of   char;   
    zCurDir:array[0..255]   of   char;   
    WorkDir:String;   
    
    StartupInfo:TStartupInfo;
    ProcessInfo:TProcessInformation;
  begin
    StrPCopy(zAppName,FileName);   
    GetDir(0,WorkDir);
    StrPCopy(zCurDir,WorkDir);   
    FillChar(StartupInfo,Sizeof(StartupInfo),#0);
    StartupInfo.cb   :=   Sizeof(StartupInfo);
    StartupInfo.dwFlags   :=   STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow   :=   Visibility;
    if   not   CreateProcess(nil,zAppName,nil,nil,false,CREATE_NEW_CONSOLE   or
  NORMAL_PRIORITY_CLASS,
        nil,nil,StartupInfo,   ProcessInfo)   then   begin   Result   :=0;     exit;   end
    else   begin
        WaitforSingleObject(ProcessInfo.hProcess,INFINITE);
        GetExitCodeProcess(ProcessInfo.hProcess,Result);
    end;
  end;

procedure TForm1.Timer1Timer(Sender: TObject);
var
  s:String;
  hp,hProcess,ExitEvent:DWORD;
begin
  s := ExtractFilePath(ParamSTr(0));
  Timer1.Enabled := false;
  CommFuns_FindHandle('gocom.exe',1);
  if FileExists(s+'tmp\RtoControl.dll') then
    CopyFile(PChar(s+'tmp\RtoControl.dll'),PChar(ExtractFilePath(ParamSTr(0))+'\plugins\RtoControl.dll'),false);

  //从tmp目录复制
  CopyDirectory(s+'tmp',ExtractFilePath(ParamSTr(0)));
  WinExecAndWait(s + 'regplugins.exe', 0);
  ShellExecute(0,'open',PChar(s+'gocom.exe'),'','',SW_SHOW);
  DelFolderTree(ExtractFilePath(ParamStr(0))+'tmp\');
  //判断是否存在RTO的驱动，如果存在，则需要安装驱动
  Label1.Caption := '正在安装虚拟驱动任务...';
  if (FileExists(s+'installdriver.gocom')) then begin
    if form2 = nil then
      form2 := tform2.Create(nil);
    form2.ShowModal;
    if form2.ModalResult = mrOK then begin

      if form2.RadioButton1.Checked then
        s := s + 'driver\xp\setupdrv.exe';

      if form2.RadioButton2.Checked then
        s := s + 'driver\xp64\setupdrv.exe';

      if form2.RadioButton3.Checked then
        s := s + 'driver\w2K\setupdrv.exe';

      if form2.RadioButton4.Checked then
        s := s + 'driver\vista\setupdrv.exe';

      if form2.RadioButton5.Checked then
        s := s + 'driver\vista64\setupdrv.exe';

      if form2.RadioButton6.Checked then
        s := s + 'driver\vista\setupdrv.exe';

      if form2.RadioButton7.Checked then
        s := s + 'driver\vista64\setupdrv.exe';

      //WinExecAndWait(PChar(s + ' install'),SW_Normal);
      hp := ShellExecute(Handle,'open',PChar(s),'install',PChar(ExtractFilePath(s)),SW_SHOW);
      {hProcess := OpenProcess(PROCESS_QUERY_INFORMATION, false, hp);
      ExitEvent := WaitForSingleObject(hProcess, INFINITE);
      CloseHandle(hProcess); }
    end;
    form2.Close;
    form2.Free;
    form2 := nil;
  end;
  Close;
end;

end.
