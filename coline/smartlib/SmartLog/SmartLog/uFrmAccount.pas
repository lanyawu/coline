unit uFrmAccount;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls, ComCtrls;

type
  TfrmAccount = class(TForm)
    Panel1: TPanel;
    Label1: TLabel;
    edtIncoming: TEdit;
    Label2: TLabel;
    edtPayout: TEdit;
    Label3: TLabel;
    cbItem: TComboBox;
    Label4: TLabel;
    cbUserName: TComboBox;
    Label5: TLabel;
    dtDate: TDateTimePicker;
    Label6: TLabel;
    mmComment: TMemo;
    btnAdd: TButton;
    btnClose: TButton;
    edtAddr: TEdit;
    procedure FormCanResize(Sender: TObject; var NewWidth,
      NewHeight: Integer; var Resize: Boolean);
    procedure btnAddClick(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

procedure ShowAccountForm;
implementation
uses
  uLogMgr;

procedure ShowAccountForm;
var
  Frm: TfrmAccount;
begin
  Frm := TfrmAccount.Create(Application);
  Frm.ShowModal;
  Frm.Free;
end;

{$R *.dfm}

procedure TfrmAccount.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
begin
  Resize := False;
end;

procedure TfrmAccount.btnAddClick(Sender: TObject);
var
  strAddr, strItem, strComment: string;
  strName, strDate: string;
begin
  if (edtIncoming.Text = '') and (edtPayout.Text = '') then
  begin
    ShowMessage('收入与支出为空');
    Exit;
  end;
  strAddr := edtAddr.Text;
  strItem := cbItem.Items[cbItem.ItemIndex];
  strComment := mmComment.Lines.Text;
  strName := cbUserName.Items[cbUserName.ItemIndex];
  strDate := DateToStr(dtDate.Date);
  if not (LM_AddPersonAccount(PChar(edtIncoming.Text), PChar(edtPayout.Text),
                      PChar(strAddr), PChar(strItem), PChar(strComment),
                      PChar(strName), PChar(strDate))) then
     ShowMessage('加入失败');
  edtIncoming.Clear;
  edtPayout.Clear;
end;

procedure TfrmAccount.btnCloseClick(Sender: TObject);
begin
  Close;
end;

procedure TfrmAccount.FormCreate(Sender: TObject);
begin
  dtDate.DateTime := Now;
end;

end.
