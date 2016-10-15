unit uAddMemo;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmAddMemo = class(TForm)
    mmLog: TMemo;
    btnAdd: TButton;
    btnCancel: TButton;
    Label1: TLabel;
    procedure FormCanResize(Sender: TObject; var NewWidth,
      NewHeight: Integer; var Resize: Boolean);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure btnAddClick(Sender: TObject);
    procedure btnCancelClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  AddMemoFrm: TfrmAddMemo;
procedure AddMemoLog;
implementation
uses
  uLogMgr;
{$R *.dfm}

procedure AddMemoLog;
begin
  AddMemoFrm.Show;
  AddMemoFrm.FormStyle := fsStayOnTop;
  AddMemoFrm.FormStyle := fsNormal;
  AddMemoFrm.BringToFront;
end;

procedure TfrmAddMemo.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
begin
  Resize := False;
end;

procedure TfrmAddMemo.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caHide;
end;

procedure TfrmAddMemo.btnAddClick(Sender: TObject);
var
  Str: string;
begin
  Str := mmLog.Lines.Text;
  if (Str <> '') then
  begin
    if LM_AddMemoLog(PChar(mmLog.Lines.Text)) then
    begin
      Close;
    end else
      ShowMessage('加入事项失败');
  end else
      ShowMessage('事项内容不能为空');
end;

procedure TfrmAddMemo.btnCancelClick(Sender: TObject);
begin
  Close;
end;

end.
