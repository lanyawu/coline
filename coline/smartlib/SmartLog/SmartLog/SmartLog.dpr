program SmartLog;

uses
  Forms,
  uSmartLog in 'uSmartLog.pas' {FrmSmartLog},
  uLogMgr in 'uLogMgr.pas',
  uFrmComment in 'uFrmComment.pas' {frmComment},
  uAddMemo in 'uAddMemo.pas' {frmAddMemo},
  uFrmAccount in 'uFrmAccount.pas' {frmAccount},
  uFrmAuto in 'uFrmAuto.pas' {FrmAuto},
  uSetPwd in 'uSetPwd.pas' {FrmSetPwd},
  uFrmInputPwd in 'uFrmInputPwd.pas' {FrmInputPwd},
  uFrmWorkFlow in 'uFrmWorkFlow.pas' {frmWorkFlow},
  SyncPath in 'SyncPath.pas' {frmSyncPath};

{$R *.res}

begin
  Application.Initialize;
  Application.Title := 'Smart¼ÇÊÂ±¾';
  Application.CreateForm(TFrmSmartLog, FrmSmartLog);
  Application.Run;
end.
