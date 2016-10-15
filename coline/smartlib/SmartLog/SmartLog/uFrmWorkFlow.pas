unit uFrmWorkFlow;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmWorkFlow = class(TForm)
    Label1: TLabel;
    Label2: TLabel;
    edtTimeSect: TEdit;
    mmFlow: TMemo;
    btnAdd: TButton;
    btnClose: TButton;
    procedure FormCanResize(Sender: TObject; var NewWidth,
      NewHeight: Integer; var Resize: Boolean);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure btnCloseClick(Sender: TObject);
    procedure btnAddClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

procedure AddWorkFlow;

implementation
uses
  uLogMgr;

procedure AddWorkFlow;
var
  Frm: TfrmWorkFlow;
begin
  Frm := TfrmWorkFlow.Create(Application);
  Frm.ShowModal;
  Frm.Free;
end;

{$R *.dfm}

procedure TfrmWorkFlow.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
begin
  Resize := False;
end;

procedure TfrmWorkFlow.FormClose(Sender: TObject;
  var Action: TCloseAction);
begin
  Action := caFree;
end;

procedure TfrmWorkFlow.btnCloseClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

procedure TfrmWorkFlow.btnAddClick(Sender: TObject);
begin
  if (edtTimeSect.Text <> '') and (mmFlow.Lines.Text <> '') then
  begin
    if LM_AddWorkFlow(PChar(edtTimeSect.Text), PChar(mmFlow.Lines.Text)) then
    begin
      edtTimeSect.Clear;
      mmFlow.Lines.Clear;
    end else
      ShowMessage('加入工作流水失败');
  end;
end;

end.
