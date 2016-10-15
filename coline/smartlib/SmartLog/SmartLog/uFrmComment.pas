unit uFrmComment;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ComCtrls, ActnList, ImgList, ToolWin, ExtCtrls;

type
  TfrmComment = class(TForm)
    btnOk: TButton;
    btnCancel: TButton;
    Editor: TRichEdit;
    StandardToolBar: TToolBar;
    CutButton: TToolButton;
    CopyButton: TToolButton;
    PasteButton: TToolButton;
    UndoButton: TToolButton;
    ToolButton10: TToolButton;
    FontName: TComboBox;
    ToolButton11: TToolButton;
    FontSize: TEdit;
    UpDown1: TUpDown;
    ToolButton2: TToolButton;
    BoldButton: TToolButton;
    ItalicButton: TToolButton;
    UnderlineButton: TToolButton;
    ToolButton16: TToolButton;
    LeftAlign: TToolButton;
    CenterAlign: TToolButton;
    RightAlign: TToolButton;
    ToolButton20: TToolButton;
    BulletsButton: TToolButton;
    ToolbarImages: TImageList;
    ActionList2: TActionList;
    EditUndoCmd: TAction;
    EditCutCmd: TAction;
    EditCopyCmd: TAction;
    EditPasteCmd: TAction;
    Panel1: TPanel;
    procedure btnOkClick(Sender: TObject);
    procedure btnCancelClick(Sender: TObject);
    procedure ActionList2Update(Action: TBasicAction;
      var Handled: Boolean);
    procedure AlignButtonClick(Sender: TObject);
    procedure BoldButtonClick(Sender: TObject);
    procedure BulletsButtonClick(Sender: TObject);
    function CurrText: TTextAttributes;
    procedure EditCopy(Sender: TObject);
    procedure EditCut(Sender: TObject);
    procedure EditPaste(Sender: TObject);
    procedure EditUndo(Sender: TObject);
    procedure FontNameChange(Sender: TObject);
    procedure FontSizeChange(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure GetFontNames;
    procedure ItalicButtonClick(Sender: TObject);
    procedure SelectFont(Sender: TObject);
    procedure SelectionChange(Sender: TObject);
    procedure SetEditRect;
    procedure UnderlineButtonClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
  private 
    { Private declarations }
  public
    { Public declarations }
  end;


function GetComment(const NodeName: string; var pBuf: PChar; var nBufSize: integer): Boolean;

implementation
uses
  RichEdit;
{$R *.dfm}
function GetComment(const NodeName: string; var pBuf: PChar; var nBufSize: integer): Boolean;
var
  Frm: TfrmComment;
  Stream: TMemoryStream;
begin
  Result := False;
  Frm := TfrmComment.Create(Application);
  try
    Frm.Caption := NodeName + '--����';
    if Frm.ShowModal = mrOk then
    begin
      //
      Stream := TMemoryStream.Create;
      try
        Frm.Editor.PlainText := False;
        Frm.Editor.Lines.SaveToStream(Stream);
        nBufSize := Stream.Size;
        GetMem(pBuf, nBufSize);
        Stream.Position := 0;
        Stream.Read(pBuf^, nBufSize);
      finally
        Stream.Free;
      end;
      Result := True;
    end;
  finally
    Frm.Free;
  end;
end;

procedure TfrmComment.btnOkClick(Sender: TObject);
begin
  ModalResult := mrOk;
end;

procedure TfrmComment.SelectionChange(Sender: TObject);
begin
  with Editor.Paragraph do
  try
    BoldButton.Down := fsBold in Editor.SelAttributes.Style;
    ItalicButton.Down := fsItalic in Editor.SelAttributes.Style;
    UnderlineButton.Down := fsUnderline in Editor.SelAttributes.Style;
    BulletsButton.Down := Boolean(Numbering);
    FontSize.Text := IntToStr(Editor.SelAttributes.Size);
    FontName.Text := Editor.SelAttributes.Name;
    case Ord(Alignment) of
      0: LeftAlign.Down := True;
      1: RightAlign.Down := True;
      2: CenterAlign.Down := True;
    end;
  finally
    
  end;
end;

function TfrmComment.CurrText: TTextAttributes;
begin
  if Editor.SelLength > 0 then Result := Editor.SelAttributes
  else Result := Editor.DefAttributes;
end;

function EnumFontsProc(var LogFont: TLogFont; var TextMetric: TTextMetric;
  FontType: Integer; Data: Pointer): Integer; stdcall;
begin
  TStrings(Data).Add(LogFont.lfFaceName);
  Result := 1;
end;

procedure TfrmComment.GetFontNames;
var
  DC: HDC;
begin
  DC := GetDC(0);
  EnumFonts(DC, nil, @EnumFontsProc, Pointer(FontName.Items));
  ReleaseDC(0, DC);
  FontName.Sorted := True;
end;

procedure TfrmComment.SetEditRect;
begin

end;

{ Event Handlers }

procedure TfrmComment.FormCreate(Sender: TObject);
begin
  GetFontNames;

  CurrText.Name := DefFontData.Name;
  CurrText.Size := -MulDiv(DefFontData.Height, 72, Screen.PixelsPerInch);
  SelectionChange(Self);
end;


procedure TfrmComment.EditUndo(Sender: TObject);
begin
  with Editor do
    if HandleAllocated then SendMessage(Handle, EM_UNDO, 0, 0);
end;

procedure TfrmComment.EditCut(Sender: TObject);
begin
  Editor.CutToClipboard;
end;

procedure TfrmComment.EditCopy(Sender: TObject);
begin
  Editor.CopyToClipboard;
end;

procedure TfrmComment.EditPaste(Sender: TObject);
begin
  Editor.PasteFromClipboard;
end;

procedure TfrmComment.SelectFont(Sender: TObject);
begin

  SelectionChange(Self);
  Editor.SetFocus;
end;

procedure TfrmComment.FormResize(Sender: TObject);
begin
  SetEditRect;
  SelectionChange(Sender);
end;


procedure TfrmComment.BoldButtonClick(Sender: TObject);
begin
  if BoldButton.Down then
    CurrText.Style := CurrText.Style + [fsBold]
  else
    CurrText.Style := CurrText.Style - [fsBold];
end;

procedure TfrmComment.ItalicButtonClick(Sender: TObject);
begin
  if ItalicButton.Down then
    CurrText.Style := CurrText.Style + [fsItalic]
  else
    CurrText.Style := CurrText.Style - [fsItalic];
end;

procedure TfrmComment.FontSizeChange(Sender: TObject);
begin
  CurrText.Size := StrToInt(FontSize.Text);
end;

procedure TfrmComment.AlignButtonClick(Sender: TObject);
begin
  Editor.Paragraph.Alignment := TAlignment(TControl(Sender).Tag);
end;

procedure TfrmComment.FontNameChange(Sender: TObject);
begin
  CurrText.Name := FontName.Items[FontName.ItemIndex];
end;

procedure TfrmComment.UnderlineButtonClick(Sender: TObject);
begin
  if UnderlineButton.Down then
    CurrText.Style := CurrText.Style + [fsUnderline]
  else
    CurrText.Style := CurrText.Style - [fsUnderline];
end;

procedure TfrmComment.BulletsButtonClick(Sender: TObject);
begin
  Editor.Paragraph.Numbering := TNumberingStyle(BulletsButton.Down);
end;


procedure TfrmComment.FormShow(Sender: TObject);
begin
  Editor.SetFocus;
end;

procedure TfrmComment.ActionList2Update(Action: TBasicAction;
  var Handled: Boolean);
begin
 { Update the status of the edit commands }
  EditCutCmd.Enabled := Editor.SelLength > 0;
  EditCopyCmd.Enabled := EditCutCmd.Enabled;
  if Editor.HandleAllocated then
  begin
    EditUndoCmd.Enabled := Editor.Perform(EM_CANUNDO, 0, 0) <> 0;
    EditPasteCmd.Enabled := Editor.Perform(EM_CANPASTE, 0, 0) <> 0;
  end;
end;

procedure TfrmComment.btnCancelClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

procedure TfrmComment.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;
end;

end.
