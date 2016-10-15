unit uSetPwd;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TFrmSetPwd = class(TForm)
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    btnOk: TButton;
    btnClose: TButton;
    edtOldPwd: TEdit;
    edtNewPwd: TEdit;
    edtChkPwd: TEdit;
    procedure FormCanResize(Sender: TObject; var NewWidth,
      NewHeight: Integer; var Resize: Boolean);
    procedure btnOkClick(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;


procedure SetPassword;
implementation
uses
  uLogMgr;

procedure SetPassword;
var
  Frm: TFrmSetPwd;
begin
  Frm := TFrmSetPwd.Create(Application);
  Frm.ShowModal;
  Frm.Free;
end;

{$R *.dfm}

procedure TFrmSetPwd.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
begin
  Resize := False;
end;

procedure TFrmSetPwd.btnOkClick(Sender: TObject);
begin
  if edtNewPwd.Text = edtChkPwd.Text then
  begin
    if not LM_SetPassword(PChar(edtOldPwd.Text), PChar(edtNewPwd.Text)) then
       ShowMessage('…Ë÷√√‹¬Î ß∞‹')
    else
       ModalResult := mrOk;
  end else begin
    ShowMessage('»∑»œ√‹¬Î≤ª∆•≈‰');
  end;
end;

procedure TFrmSetPwd.btnCloseClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

end.
