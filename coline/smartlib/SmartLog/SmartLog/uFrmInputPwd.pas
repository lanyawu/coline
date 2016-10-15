unit uFrmInputPwd;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TFrmInputPwd = class(TForm)
    Label1: TLabel;
    edtPwd: TEdit;
    btnOk: TButton;
    btnClose: TButton;
    procedure FormCanResize(Sender: TObject; var NewWidth,
      NewHeight: Integer; var Resize: Boolean);
    procedure btnOkClick(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure edtPwdKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
  private
    { Private declarations }
  public
    { Public declarations }
  end;


function InputPwd: string;

implementation

function InputPwd: string;
var
  Frm: TFrmInputPwd;
begin
  Result := '';
  Frm := TFrmInputPwd.Create(Application);
  if Frm.ShowModal = mrOk then
     Result := Frm.edtPwd.Text;
  Frm.Free;
end;

{$R *.dfm}

procedure TFrmInputPwd.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
begin
  Resize := False;
end;

procedure TFrmInputPwd.btnOkClick(Sender: TObject);
begin
  ModalResult := mrOk;
end;

procedure TFrmInputPwd.btnCloseClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

procedure TFrmInputPwd.edtPwdKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if Key = VK_RETURN then
     ModalResult := mrOk;
end;

end.
