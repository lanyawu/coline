unit uFrmAuto;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ComCtrls;

type
  TFrmAuto = class(TForm)
    gbOil: TGroupBox;
    Label1: TLabel;
    edtMetre: TEdit;
    Label2: TLabel;
    edtPrice: TEdit;
    Label3: TLabel;
    edtCount: TEdit;
    Label4: TLabel;
    edtTotal: TEdit;
    Label5: TLabel;
    Label6: TLabel;
    cbQua: TComboBox;
    dtDate: TDateTimePicker;
    Label7: TLabel;
    mmComment: TMemo;
    btnAdd: TButton;
    GroupBox1: TGroupBox;
    Label8: TLabel;
    edtFeePrice: TEdit;
    Label9: TLabel;
    edtFeeCount: TEdit;
    Label10: TLabel;
    edtFeeTotal: TEdit;
    Label11: TLabel;
    dtFeeDate: TDateTimePicker;
    Label12: TLabel;
    mmFeeComment: TMemo;
    btnFeeAdd: TButton;
    btnClose: TButton;
    procedure btnAddClick(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure btnFeeAddClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormCanResize(Sender: TObject; var NewWidth,
      NewHeight: Integer; var Resize: Boolean);
  private
    { Private declarations }
  public
    { Public declarations }
  end;


procedure ShowAutoForm;
implementation
uses
  uLogMgr;
{$R *.dfm}
procedure ShowAutoForm;
var
  Frm: TFrmAuto;
begin
  Frm := TFrmAuto.Create(Application);
  Frm.ShowModal;
  Frm.Free;
end;

procedure TFrmAuto.btnAddClick(Sender: TObject);
var
  strDate: string;
begin
  strDate := DateToStr(dtDate.Date);
  if not LM_AddAutoOil(PChar(edtPrice.Text), PChar(edtCount.Text), PChar(edtTotal.Text),
              PChar(strDate), PChar(edtMetre.Text), PChar(mmComment.Lines.Text)) then
     ShowMessage('����ʧ��');
  edtCount.Clear;
  edtTotal.Clear;
  edtMetre.Clear;
  mmComment.Lines.Clear;
end;

procedure TFrmAuto.btnCloseClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

procedure TFrmAuto.btnFeeAddClick(Sender: TObject);
var
  strDate: string;
begin
  strDate := DateToStr(dtFeeDate.Date);
  if not LM_AddAutoFee(PChar(edtFeePrice.Text), PChar(edtFeeCount.Text),
          PChar(edtFeeTotal.Text), PChar(strDate), PChar(mmFeeComment.Lines.Text)) then
          ShowMessage('����ʧ��');
  edtFeePrice.Clear;
  edtFeeCount.Clear;
  edtFeeTotal.Clear;
  mmFeeComment.Lines.Clear;
end;

procedure TFrmAuto.FormCreate(Sender: TObject);
begin
  dtFeeDate.DateTime := Now;
  dtDate.DateTime := Now;
end;

procedure TFrmAuto.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
begin
  Resize := False;
end;

end.
