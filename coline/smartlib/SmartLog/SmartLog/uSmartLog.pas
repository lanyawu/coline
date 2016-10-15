unit uSmartLog;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ActnList, ImgList, Menus, ComCtrls, uLogMgr, StdCtrls, ToolWin, ExtCtrls,
  Buttons;

const
  WISDOM_TREE_NODE_ID  = $FFFFFFFF;
  SEARCH_TREE_NODE_ID  = $FFFFFFFE;
  WISDOM_TREE_TITLE    = '名言警句';
  COMMENT_BACK_COLOR   = $F0CAA6;
type
  TMyHintWindow = class(THintWindow)
  private

    FWndBmp: TBitmap; //窗口位图
    FHintBmp: TBitmap; //提示信息位图
  protected
    procedure CreateParams(var Params: TCreateParams); override;
    procedure Paint; override;
    procedure NCPaint(DC: HDC); override;
    {画提示的图象}
    procedure DrawHintImg(Bmp:TBitmap; AHint: string);
    {取得提示窗口对应的桌面区域的图象}
    procedure GetDesktopImg(Bmp: TBitmap; R: TRect);
    {对桌面区域图象作处理,使其看起来像一块玻璃且带有一点阴影}
    procedure EffectHandle(WndBmp, HintBmp: TBitmap);
  public
    constructor Create(Aowner: TComponent); override;
    destructor Destroy; override;
    procedure ActivateHint(Rect: TRect; const AHint: string); override;
  end;

  TRichEdit20 = class(TRichEdit)
  public
    procedure CreateParams(var Params: TCreateParams); override;
  end;
  TFrmSmartLog = class(TForm)
    Ruler: TPanel;
    FirstInd: TLabel;
    LeftInd: TLabel;
    RulerLine: TBevel;
    RightInd: TLabel;
    Bevel1: TBevel;
    StatusBar: TStatusBar;
    StandardToolBar: TToolBar;
    ToolButton1: TToolButton;
    OpenButton: TToolButton;
    SaveButton: TToolButton;
    PrintButton: TToolButton;
    ToolButton5: TToolButton;
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
    MainMenu: TMainMenu;
    FileMenu: TMenuItem;
    FileNewItem: TMenuItem;
    FileOpenItem: TMenuItem;
    FileSaveItem: TMenuItem;
    N1: TMenuItem;
    FilePrintItem: TMenuItem;
    N4: TMenuItem;
    FileExitItem: TMenuItem;
    EditMenu: TMenuItem;
    EditUndoItem: TMenuItem;
    N2: TMenuItem;
    EditCutItem: TMenuItem;
    EditCopyItem: TMenuItem;
    EditPasteItem: TMenuItem;
    N5: TMenuItem;
    miEditFont: TMenuItem;
    HelpMenu: TMenuItem;
    HelpAboutItem: TMenuItem;
    OpenDialog: TOpenDialog;
    SaveDialog: TSaveDialog;
    PrintDialog: TPrintDialog;
    FontDialog1: TFontDialog;
    ToolbarImages: TImageList;
    ActionList1: TActionList;
    FileNewCmd: TAction;
    FileOpenCmd: TAction;
    FileSaveCmd: TAction;
    FilePrintCmd: TAction;
    FileExitCmd: TAction;
    FileSaveAsCmd: TAction;
    ActionList2: TActionList;
    EditUndoCmd: TAction;
    EditCutCmd: TAction;
    EditCopyCmd: TAction;
    EditPasteCmd: TAction;
    EditFontCmd: TAction;
    pnlClient: TPanel;
    pnlEditor: TPanel;
    pnlList: TPanel;
    Splitter1: TSplitter;
    tvList: TTreeView;
    pmTree: TPopupMenu;
    miCreateChildNode: TMenuItem;
    acTree: TActionList;
    acCreateChildNode: TAction;
    Panel1: TPanel;
    lvAffix: TListView;
    Splitter2: TSplitter;
    ListViewImageList: TImageList;
    pmListView: TPopupMenu;
    miSaveAs: TMenuItem;
    miAddAffix: TMenuItem;
    TreeImageList: TImageList;
    miDelAffix: TMenuItem;
    miDelLog: TMenuItem;
    EditInsertImage: TAction;
    N3: TMenuItem;
    miCreateTopNode: TMenuItem;
    fdText: TFindDialog;
    ImageList1: TImageList;
    pmEditor: TPopupMenu;
    miCopy: TMenuItem;
    miCut: TMenuItem;
    Paste1: TMenuItem;
    miUndo: TMenuItem;
    N6: TMenuItem;
    miWisdom: TMenuItem;
    miComment: TMenuItem;
    pnlAffix: TPanel;
    Label1: TLabel;
    pnlInfo: TPanel;
    reComment: TRichEdit;
    Label2: TLabel;
    Label3: TLabel;
    lblCreateDate: TLabel;
    lblModiDate: TLabel;
    miDeleteComment: TMenuItem;
    ToolButton3: TToolButton;
    edtSearchText: TEdit;
    sbSearch: TSpeedButton;
    miFind: TMenuItem;
    miTool: TMenuItem;
    miAccount: TMenuItem;
    miAuto: TMenuItem;
    miSetPwd: TMenuItem;
    ToolButton4: TToolButton;
    N7: TMenuItem;
    miRefresh: TMenuItem;
    miWorkFlow: TMenuItem;
    miSync: TMenuItem;
    procedure EditCopy(Sender: TObject);
    procedure EditCut(Sender: TObject);
    procedure EditPaste(Sender: TObject);
    procedure EditUndo(Sender: TObject);
    procedure FileExit(Sender: TObject);
    procedure FileNew(Sender: TObject);
    procedure FileOpen(Sender: TObject);
    procedure FilePrint(Sender: TObject);
    procedure FileSave(Sender: TObject);
    procedure FileSaveAs(Sender: TObject);
    procedure FontNameChange(Sender: TObject);
    procedure FontSizeChange(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure FormCreate(Sender: TObject);
    procedure FormPaint(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure AlignButtonClick(Sender: TObject);
    procedure BoldButtonClick(Sender: TObject);
    procedure BulletsButtonClick(Sender: TObject);
    procedure FirstIndMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure GetFontNames;
    procedure HelpAbout(Sender: TObject);
    procedure ItalicButtonClick(Sender: TObject);
    procedure LeftIndMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure PerformFileOpen(const AFileName: string);
    procedure RichEditChange(Sender: TObject);
    procedure RightIndMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure RulerItemMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure RulerItemMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure RulerResize(Sender: TObject);
    procedure SelectFont(Sender: TObject);
    procedure SelectionChange(Sender: TObject);
    procedure ActionList2Update(Action: TBasicAction;
      var Handled: Boolean);
    procedure CheckFileSave;
    function CurrText: TTextAttributes;
    procedure SetFileName(const FileName: String);
    procedure SetModified(Value: Boolean);
    procedure SetupRuler;
    procedure ShowHint(Sender: TObject);
    procedure UnderlineButtonClick(Sender: TObject);
    procedure UpdateCursorPos;
    procedure WMDropFiles(var Msg: TWMDropFiles); message WM_DROPFILES;
    procedure SetEditRect;
    procedure acCreateChildNodeExecute(Sender: TObject);
    procedure lvAffixGetImageIndex(Sender: TObject; Item: TListItem);
    procedure FormDestroy(Sender: TObject);
    procedure miSaveAsClick(Sender: TObject);
    procedure pmListViewPopup(Sender: TObject);
    procedure miAddAffixClick(Sender: TObject);
    procedure tvListGetImageIndex(Sender: TObject; Node: TTreeNode);
    procedure tvListGetSelectedIndex(Sender: TObject; Node: TTreeNode);
    procedure tvListClick(Sender: TObject);
    procedure tvListEdited(Sender: TObject; Node: TTreeNode;
      var S: String);
    procedure miCreateChildNodeClick(Sender: TObject);
    procedure miDelAffixClick(Sender: TObject);
    procedure miDelLogClick(Sender: TObject);
    procedure tvListChange(Sender: TObject; Node: TTreeNode);
    procedure EditInsertImageExecute(Sender: TObject);
    procedure miCreateTopNodeClick(Sender: TObject);
    procedure fdTextFind(Sender: TObject);
    procedure fdTextShow(Sender: TObject);
    procedure pmEditorPopup(Sender: TObject);
    procedure miWisdomClick(Sender: TObject);
    procedure tvListDragOver(Sender, Source: TObject; X, Y: Integer;
      State: TDragState; var Accept: Boolean);
    procedure tvListEndDrag(Sender, Target: TObject; X, Y: Integer);
    procedure miCommentClick(Sender: TObject);
    procedure EditorKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure miDeleteCommentClick(Sender: TObject);
    procedure edtSearchTextEnter(Sender: TObject);
    procedure sbSearchClick(Sender: TObject);
    procedure miFindClick(Sender: TObject);
    procedure miAccountClick(Sender: TObject);
    procedure miAutoClick(Sender: TObject);
    procedure miSetPwdClick(Sender: TObject);
    procedure ToolButton4Click(Sender: TObject);
    procedure miRefreshClick(Sender: TObject);
    procedure miWorkFlowClick(Sender: TObject);
    procedure miSyncClick(Sender: TObject);
  private
    FFileName: string;
    FSearchText: string;
    FSearchType: TSearchTypes;
    FIconList: TStringList;
    FUpdating: Boolean;
    FDragOfs: Integer;
    FModified: Boolean;
    FDragging: Boolean;
    FLogComments: PLOG_COMMENT_ITEM;
    FLogCommentCount: integer;
    FPreSelNode: TTreeNode;
    FMsgHotKeyId: Cardinal;
    Editor: TRichEdit20;
    function InitLogFile(FileName: string): Boolean;
    procedure LoadNodeChild(Node: TTreeNode);
    procedure LoadNodeLog(Node: TTreeNode);
    procedure TreeViewSelChg;
    procedure GetCommentById(const nLogId: integer);
    procedure SaveNodeLog(Node: TTreeNode);
    function  GetSearchNode: TTreeNode;
    procedure InsertImage(FileName: string);
    procedure RefreshFormCaption(Node: TTreeNode);
    function  GetCommentIdByCurrSel(const nSelStart: integer): integer;
    function  CheckIsSpecLogId(const nLogId: DWORD): Boolean;
    function  RegisterHotKey(const HotKey: string): Boolean;
    procedure StringHotKeyToWord(const Str: string;
      var fsModifier: Cardinal; var nKey: Word);
    procedure WMHOTKEY(var Msg: TMessage); message WM_HOTKEY;
    { Private declarations }
  public
    { Public declarations }
  end;

var
  FrmSmartLog: TFrmSmartLog;

implementation
uses
  RichEdit, ComObj, uAddMemo, uFrmComment, uSetPwd, SyncPath,
  uFrmAccount, uFrmAuto, uFrmWorkFlow, uFrmInputPwd, ActiveX, ShellApi;

var
  FRichEditModule: Cardinal = 0;
type
  _ReObject = record
    cbStruct: DWORD;           { Size of structure                }
    cp: ULONG;                 { Character position of object     }
    clsid: TCLSID;             { Class ID of object               }
    poleobj: IOleObject;       { OLE object interface             }
    pstg: IStorage;            { Associated storage interface     }
    polesite: IOleClientSite;  { Associated client site interface }
    sizel: TSize;              { Size of object (may be 0,0)      }
    dvAspect: Longint;         { Display aspect to use            }
    dwFlags: DWORD;            { Object status flags              }
    dwUser: DWORD;             { Dword for user's use             }
  end;

  TReObject = _ReObject;
  
  IRichEditOle = interface(IUnknown)
    ['{00020d00-0000-0000-c000-000000000046}']
    function GetClientSite(out clientSite: IOleClientSite): HResult; stdcall;
    function GetObjectCount: HResult; stdcall;
    function GetLinkCount: HResult; stdcall;
    function GetObject(iob: Longint; out reobject: TReObject;
      dwFlags: DWORD): HResult; stdcall;
    function InsertObject(var reobject: TReObject): HResult; stdcall;
    function ConvertObject(iob: Longint; rclsidNew: TIID;
      lpstrUserTypeNew: LPCSTR): HResult; stdcall;
    function ActivateAs(rclsid: TIID; rclsidAs: TIID): HResult; stdcall;
    function SetHostNames(lpstrContainerApp: LPCSTR;
      lpstrContainerObj: LPCSTR): HResult; stdcall;
    function SetLinkAvailable(iob: Longint; fAvailable: BOOL): HResult; stdcall;
    function SetDvaspect(iob: Longint; dvaspect: DWORD): HResult; stdcall;
    function HandsOffStorage(iob: Longint): HResult; stdcall;
    function SaveCompleted(iob: Longint; const stg: IStorage): HResult; stdcall;
    function InPlaceDeactivate: HResult; stdcall;
    function ContextSensitiveHelp(fEnterMode: BOOL): HResult; stdcall;
    function GetClipboardData(var chrg: TCharRange; reco: DWORD;
      out dataobj: IDataObject): HResult; stdcall;
    function ImportDataObject(dataobj: IDataObject; cf: TClipFormat;
      hMetaPict: HGLOBAL): HResult; stdcall;
  end;

resourcestring
  sSaveChanges = 'Save changes to %s?';
  sOverWrite = 'OK to overwrite %s';
  sUntitled = 'Untitled';
  sModified = 'Modified';
  sColRowInfo = 'Line: %3d   Col: %3d';

const
  RulerAdj = 4/3;
  GutterWid = 6;
  REO_CP_SELECTION = $FFFFFFFF;
  REO_BELOWBASELINE = $00000002;
  REO_STATIC = $40000000;
  REO_RESIZABLE = $00000001;
  REO_GETOBJ_ALL_INTERFACES = $00000007;  
  IID_IOleObject: TGUID = (
    D1:$00000112;D2:$0000;D3:$0000;D4:($C0,$00,$00,$00,$00,$00,$00,$46));
{$R *.dfm}
procedure TFrmSmartLog.SelectionChange(Sender: TObject);
var
  nCommentId: integer;
  pBuf: PChar;
  nBufSize: integer;
  Stream: TMemoryStream;
begin
  with Editor.Paragraph do
  try
    FUpdating := True;
    FirstInd.Left := Trunc(FirstIndent*RulerAdj)-4+GutterWid;
    LeftInd.Left := Trunc((LeftIndent+FirstIndent)*RulerAdj)-4+GutterWid;
    RightInd.Left := Ruler.ClientWidth-6-Trunc((RightIndent+GutterWid)*RulerAdj);
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
    UpdateCursorPos;
  finally
    FUpdating := False;
  end;
  nCommentId := GetCommentIdByCurrSel(Editor.SelStart);
  reComment.Clear;
  if nCommentId > 0 then
  begin
    nBufSize := 0;
    pBuf := nil;
    if LM_GetCommentText(nCommentId, @pBuf, @nBufSize) then
    begin
      Stream := TMemoryStream.Create;
      try
        Stream.Write(pBuf^, nBufSize);
        Stream.Position := 0;
        reComment.PlainText := False;
        reComment.Lines.LoadFromStream(Stream);
      finally
        Stream.Free;
      end;
      LM_DeleteString(pBuf);
      nBufSize := 0;
    end;
  end;
end;

function TFrmSmartLog.CurrText: TTextAttributes;
begin
  if Editor.SelLength > 0 then
     Result := Editor.SelAttributes
  else
     Result := Editor.DefAttributes;
end;

function EnumFontsProc(var LogFont: TLogFont; var TextMetric: TTextMetric;
  FontType: Integer; Data: Pointer): Integer; stdcall;
begin
  TStrings(Data).Add(LogFont.lfFaceName);
  Result := 1;
end;

procedure TFrmSmartLog.GetFontNames;
var
  DC: HDC;
begin
  DC := GetDC(0);
  EnumFonts(DC, nil, @EnumFontsProc, Pointer(FontName.Items));
  ReleaseDC(0, DC);
  FontName.Sorted := True;
end;

procedure TFrmSmartLog.SetFileName(const FileName: String);
begin
  FFileName := FileName;
  Caption := Format('%s - %s', [ExtractFileName(FileName), Application.Title]);
end;

procedure TFrmSmartLog.CheckFileSave;
begin
  SaveNodeLog(tvList.Selected);
end;

procedure TFrmSmartLog.SetupRuler;
var
  I: Integer;
  S: String;
begin
  SetLength(S, 201);
  I := 1;
  while I < 200 do
  begin
    S[I] := #9;
    S[I+1] := '|';
    Inc(I, 2);
  end;
  Ruler.Caption := S;
end;

procedure TFrmSmartLog.SetEditRect;
var
  R: TRect;
begin
  with Editor do
  begin
    R := Rect(GutterWid, 0, ClientWidth-GutterWid, ClientHeight);
    SendMessage(Handle, EM_SETRECT, 0, Longint(@R));
  end;
end;

{ Event Handlers }

procedure TFrmSmartLog.FormCreate(Sender: TObject);
begin
  Editor := TRichEdit20.Create(Self);
  Editor.Parent := pnlEditor;
  Editor.PopupMenu := pmEditor;
  Editor.ScrollBars := ssBoth;
  Editor.Align := alClient;
  Editor.OnKeyDown := EditorKeyDown;
  Editor.OnChange := RichEditChange;
  FLogComments := nil;
  FLogCommentCount := 0;
  Application.OnHint := ShowHint;
  OpenDialog.InitialDir := ExtractFilePath(ParamStr(0));
  SaveDialog.InitialDir := OpenDialog.InitialDir;
  SetFileName(sUntitled);
  GetFontNames;
  SetupRuler;
  SelectionChange(Self);

  CurrText.Name := DefFontData.Name;
  CurrText.Size := -MulDiv(DefFontData.Height, 72, Screen.PixelsPerInch);
  FIconList := TStringList.Create;
  FPreSelNode := nil;
  RegisterHotKey('CTRL+ALT+M');
  AddMemoFrm := TfrmAddMemo.Create(Self);
  AddMemoFrm.Hide;
end;

procedure TFrmSmartLog.ShowHint(Sender: TObject);
begin
  if Length(Application.Hint) > 0 then
  begin
    StatusBar.SimplePanel := True;
    StatusBar.SimpleText := Application.Hint;
  end else
    StatusBar.SimplePanel := False;
end;

procedure TFrmSmartLog.FileNew(Sender: TObject);
var
  Dlg: TSaveDialog;
begin
  Dlg := TSaveDialog.Create(Self);
  try
    if Dlg.Execute then
    begin
      InitLogFile(Dlg.FileName);
      Editor.Lines.Clear;
      Editor.Modified := False;
      SetModified(False);
    end;
  finally
    Dlg.Free;
  end;
end;

procedure TFrmSmartLog.PerformFileOpen(const AFileName: string);
begin
  InitLogFile(AFileName);
  SetFileName(AFileName);
  Editor.SetFocus;
  Editor.Modified := False;
  SetModified(False);
end;

procedure TFrmSmartLog.FileOpen(Sender: TObject);
begin
  CheckFileSave;
  if OpenDialog.Execute then
  begin
    PerformFileOpen(OpenDialog.FileName);
    Editor.ReadOnly := True;
  end;
end;

procedure TFrmSmartLog.FileSave(Sender: TObject);
begin
  fdText.Execute;
end;

procedure TFrmSmartLog.FileSaveAs(Sender: TObject);
begin
  //
end;

procedure TFrmSmartLog.FilePrint(Sender: TObject);
begin
  Editor.ReadOnly := False;
end;

procedure TFrmSmartLog.FileExit(Sender: TObject);
begin
  Application.Terminate;;
end;

procedure TFrmSmartLog.EditUndo(Sender: TObject);
begin
  with Editor do
    if HandleAllocated then SendMessage(Handle, EM_UNDO, 0, 0);
end;

procedure TFrmSmartLog.EditCut(Sender: TObject);
begin
  Editor.CutToClipboard;
end;

procedure TFrmSmartLog.EditCopy(Sender: TObject);
begin
  Editor.CopyToClipboard;
end;

procedure TFrmSmartLog.EditPaste(Sender: TObject);
begin
  Editor.PasteFromClipboard;
end;

procedure TFrmSmartLog.HelpAbout(Sender: TObject);
begin
  //
end;

procedure TFrmSmartLog.SelectFont(Sender: TObject);
begin
  FontDialog1.Font.Assign(Editor.SelAttributes);
  if FontDialog1.Execute then
    CurrText.Assign(FontDialog1.Font);
  SelectionChange(Self);
  Editor.SetFocus;
end;

procedure TFrmSmartLog.RulerResize(Sender: TObject);
begin
  RulerLine.Width := Ruler.ClientWidth - (RulerLine.Left*2);
end;

procedure TFrmSmartLog.FormResize(Sender: TObject);
begin
  SetEditRect;
  SelectionChange(Sender);
end;

procedure TFrmSmartLog.FormPaint(Sender: TObject);
begin
  SetEditRect;
end;

procedure TFrmSmartLog.BoldButtonClick(Sender: TObject);
begin
  if FUpdating then Exit;
  if BoldButton.Down then
    CurrText.Style := CurrText.Style + [fsBold]
  else
    CurrText.Style := CurrText.Style - [fsBold];
end;

procedure TFrmSmartLog.ItalicButtonClick(Sender: TObject);
begin
  if FUpdating then Exit;
  if ItalicButton.Down then
    CurrText.Style := CurrText.Style + [fsItalic]
  else
    CurrText.Style := CurrText.Style - [fsItalic];
end;

procedure TFrmSmartLog.FontSizeChange(Sender: TObject);
begin
  if FUpdating then Exit;
  CurrText.Size := StrToInt(FontSize.Text);
end;

procedure TFrmSmartLog.AlignButtonClick(Sender: TObject);
begin
  if FUpdating then Exit;
  Editor.Paragraph.Alignment := TAlignment(TControl(Sender).Tag);
end;

procedure TFrmSmartLog.FontNameChange(Sender: TObject);
begin
  if FUpdating then Exit;
  CurrText.Name := FontName.Items[FontName.ItemIndex];
end;

procedure TFrmSmartLog.UnderlineButtonClick(Sender: TObject);
begin
  if FUpdating then Exit;
  if UnderlineButton.Down then
    CurrText.Style := CurrText.Style + [fsUnderline]
  else
    CurrText.Style := CurrText.Style - [fsUnderline];
end;

procedure TFrmSmartLog.BulletsButtonClick(Sender: TObject);
begin
  if FUpdating then Exit;
  Editor.Paragraph.Numbering := TNumberingStyle(BulletsButton.Down);
end;

procedure TFrmSmartLog.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
  try
    CheckFileSave;
  except
    CanClose := False;
  end;
  CanClose := False;
  Application.Minimize;
end;

{ Ruler Indent Dragging }

procedure TFrmSmartLog.RulerItemMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  FDragOfs := (TLabel(Sender).Width div 2);
  TLabel(Sender).Left := TLabel(Sender).Left+X-FDragOfs;
  FDragging := True;
end;

procedure TFrmSmartLog.RulerItemMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
  if FDragging then
    TLabel(Sender).Left :=  TLabel(Sender).Left+X-FDragOfs
end;

procedure TFrmSmartLog.FirstIndMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  FDragging := False;
  Editor.Paragraph.FirstIndent := Trunc((FirstInd.Left+FDragOfs-GutterWid) / RulerAdj);
  LeftIndMouseUp(Sender, Button, Shift, X, Y);
end;

procedure TFrmSmartLog.LeftIndMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  FDragging := False;
  Editor.Paragraph.LeftIndent := Trunc((LeftInd.Left+FDragOfs-GutterWid) / RulerAdj)-Editor.Paragraph.FirstIndent;
  SelectionChange(Sender);
end;

procedure TFrmSmartLog.RightIndMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  FDragging := False;
  Editor.Paragraph.RightIndent := Trunc((Ruler.ClientWidth-RightInd.Left+FDragOfs-2) / RulerAdj)-2*GutterWid;
  SelectionChange(Sender);
end;

procedure TFrmSmartLog.UpdateCursorPos;
var
  CharPos: TPoint;
begin
  CharPos.Y := SendMessage(Editor.Handle, EM_EXLINEFROMCHAR, 0,
    Editor.SelStart);
  CharPos.X := (Editor.SelStart -
    SendMessage(Editor.Handle, EM_LINEINDEX, CharPos.Y, 0));
  Inc(CharPos.Y);
  Inc(CharPos.X);
  StatusBar.Panels[0].Text := Format(sColRowInfo, [CharPos.Y, CharPos.X]);
end;

procedure TFrmSmartLog.FormShow(Sender: TObject);
begin
  UpdateCursorPos;
  DragAcceptFiles(Handle, True);
  RichEditChange(nil);
  Editor.SetFocus;
  { Check if we should load a file from the command line }
  if (ParamCount > 0) and FileExists(ParamStr(1)) then
    PerformFileOpen(ParamStr(1));
end;

procedure TFrmSmartLog.WMDropFiles(var Msg: TWMDropFiles);
var
  CFileName: array[0..MAX_PATH] of Char;
  Po: TPoint;
  Item: TListItem;
  SelNode: TTreeNode;
  ChildNode: TTreeNode;
  nCount, i: integer;
  nLogId, nParentId: integer;
  NodeName, ExtName: string;
  //
  function PoInEditor: Boolean;
  var
    lt: TPoint;
  begin
    lt.X := Editor.Left;
    lt.Y := Editor.Top;
    lt := Editor.ClientToParent(lt, Self);
    if (Po.X > lt.X) and (Po.Y > lt.Y) and (Po.X < (lt.X + Editor.Width))
       and (Po.Y < (lt.Y + Editor.Height)) then
       Result := True
    else
       Result := False;
  end;
  //
  function PoInListView: Boolean;
  var
    lt: TPoint;
  begin
    lt.X := lvAffix.Left;
    lt.Y := lvAffix.Top;
    lt := lvAffix.ClientToParent(lt, Self);
    if (Po.X > lt.X) and (Po.Y > lt.Y) and (Po.X < (lt.X + lvAffix.Width))
       and (Po.Y < (lt.Y + lvAffix.Height)) then
       Result := True
    else
       Result := False;
  end;
  //
  function GetSelTreeNode: TTreeNode;
  var
    lt: TPoint;
  begin
    lt.X := tvList.Left;
    lt.Y := tvList.Top;
    if (Po.X > lt.X) and (Po.Y > lt.Y) and (Po.X < (lt.X + tvList.Width))
       and (Po.Y < (lt.Y + tvList.Height)) then  begin
       lt := tvList.ParentToClient(Po, Self);
       Result := tvList.GetNodeAt(lt.X, lt.y);
    end else
      Result := nil;
  end;
begin
  try
    GetCursorPos(Po);
    Po := ScreenToClient(Po);
    if PoInEditor then
    begin
      if Editor.ReadOnly then
      begin
        ShowMessage('文件是可读状态，不能修改文件内容');
        Exit;
      end;
      if DragQueryFile(Msg.Drop, 0, CFileName, MAX_PATH) > 0 then
      begin
        Editor.Lines.LoadFromFile(CFileName);
      end;
    end else if PoInListView then
    begin
      if Editor.ReadOnly then
      begin
        ShowMessage('文件是可读状态，不能修改文件内容');
        Exit;
      end;
      nCount := DragQueryFile(Msg.Drop, $FFFFFFFF, CFileName, MAX_PATH);
      for i := 0 to nCount - 1 do
      begin
        if DragQueryFile(Msg.Drop, i, CFileName, MAX_PATH) > 0 then
        begin
          Item := lvAffix.Items.Add;
          Item.Data := nil;
          Item.Caption := CFileName;
          FModified := True;
        end;
      end;
    end else begin
      if DragQueryFile(Msg.Drop, 0, CFileName, MAX_PATH) > 0 then
      begin
        SelNode := GetSelTreeNode;
        if SelNode <> nil then
        begin
          NodeName := ExtractFileName(CFileName);
          ExtName := ExtractFileExt(NodeName);
          NodeName := Copy(NodeName, 1, Length(NodeName) - Length(ExtName));
          nLogId := 0;
          if SelNode.Data <> nil then
             nParentId := integer(SelNode.Data)
          else
             nParentId := 0;
         
          if LM_AddLog(@nLogId, nParentId, PChar(NodeName)) then
          begin
            ChildNode := tvList.Items.AddChildObject(SelNode, NodeName, Pointer(nLogId));
            tvList.Select(ChildNode);
            TreeViewSelChg;
            Editor.ReadOnly := False;
            Editor.Lines.LoadFromFile(CFileName);
          end else
            ShowMessage('增加日志失败');
        end else begin
          CheckFileSave;
          PerformFileOpen(CFileName);
        end;
      end;
    end;
  finally
    DragFinish(Msg.Drop);
  end;
  Msg.Result := 0;
end;

procedure TFrmSmartLog.RichEditChange(Sender: TObject);
begin
  SetModified(Editor.Modified);
end;

procedure TFrmSmartLog.SetModified(Value: Boolean);
begin
  if Value then StatusBar.Panels[1].Text := sModified
  else StatusBar.Panels[1].Text := '';
end;

procedure TFrmSmartLog.ActionList2Update(Action: TBasicAction;
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

procedure TFrmSmartLog.acCreateChildNodeExecute(Sender: TObject);
begin
  //
end;

procedure TFrmSmartLog.lvAffixGetImageIndex(Sender: TObject;
  Item: TListItem);
var
  FileName: string;
  ExtName: string;
  Idx: integer;
  FileInfo:  SHFILEINFO;
  Icon: TIcon;
begin
  if TListView(Item.ListView).LargeImages <> nil then
  begin
    FileName := Item.Caption;
    ExtName := ExtractFileExt(FileName);
    Idx := FIconList.IndexOf(ExtName);
    if Idx >= 0 then
    begin
      Item.ImageIndex := Integer(FIconList.Objects[Idx]);
    end else begin
      FillChar(FileInfo, SizeOf(SHFILEINFO), 0);
      SHGetFileInfo(PChar(FileName), FILE_ATTRIBUTE_NORMAL, FileInfo,
        SizeOf(SHFILEINFO), SHGFI_USEFILEATTRIBUTES or SHGFI_ICON or SHGFI_LARGEICON);
      if FileInfo.hIcon > 0 then begin
         Icon := TIcon.Create;
         Icon.Handle := FileInfo.hIcon;
         Idx := TListView(Item.ListView).LargeImages.AddIcon(Icon);
         Item.ImageIndex := Idx;
         FIconList.AddObject(ExtName, TObject(Idx));
      end;
    end;
  end;
end;

procedure TFrmSmartLog.FormDestroy(Sender: TObject);
begin
  LM_DeleteCommentItems(FLogComments);
  FLogCommentCount := 0;
  SaveNodeLog(tvList.Selected);
  FIconList.Free;
  Windows.UnregisterHotKey(Handle, FMsgHotKeyId);
  AddMemoFrm.Free;
end;

procedure TFrmSmartLog.miSaveAsClick(Sender: TObject);
var
  Dlg: TSaveDialog;
  FileName: string;
begin
  if lvAffix.Selected <> nil then
  begin
    Dlg := TSaveDialog.Create(Self);
    try
      if Dlg.Execute then
      begin
        FileName := Dlg.FileName;
        if lvAffix.Selected.Data <> nil then
        begin
          if LM_SaveAsAffix(integer(lvAffix.Selected.Data), PChar(FileName)) then
             ShowMessage('另存文件成功')
          else
             ShowMessage('另存文件失败');
        end else begin
          if CopyFile(PChar(lvAffix.Selected.Caption), PChar(FileName), True) then
             ShowMessage('另存文件成功')
          else
             ShowMessage('另存文件失败');
        end;
      end;
    finally
      Dlg.Free;
    end;
  end;
end;

procedure TFrmSmartLog.pmListViewPopup(Sender: TObject);
begin
  if lvAffix.Selected = nil then
     miSaveAs.Enabled := False
  else
     miSaveAs.Enabled := True;
end;

procedure TFrmSmartLog.miAddAffixClick(Sender: TObject);
var
  Dlg: TOpenDialog;
  FileName: string;
  Item: TListItem;
begin
  Dlg := TOpenDialog.Create(Self);
  try
    if Dlg.Execute then
    begin
      FileName := Dlg.FileName;
      Item := lvAffix.Items.Add;
      Item.Caption := FileName;
      Item.Data := nil;
      FModified := True;
    end;
  finally
    Dlg.Free;
  end;
end;

function TFrmSmartLog.InitLogFile(FileName: string): Boolean;
var
  bSucc: Boolean;
  i: integer;
  str: string;
begin
  tvList.Items.Clear;
  FPreSelNode := nil;
  if LM_InitLogMgr(PChar(FileName)) then
  begin
    bSucc := True;
    if not LM_CheckPassword(PChar('')) then
    begin
      bSucc := False;
      for i := 0 to 3 do
      begin
        str := InputPwd;
        if str = '' then
           break;
        if LM_CheckPassword(PChar(str)) then
        begin
          bSucc := True;
          break;
        end else
          ShowMessage('密码错误');
      end;
    end;
    if bSucc then
    begin
      tvList.Items.AddChildObjectFirst(nil, WISDOM_TREE_TITLE, Pointer(WISDOM_TREE_NODE_ID));
      LoadNodeChild(nil);
      Result := True;
    end else begin
      LM_DestroyLogMgr;
      Result := False;
    end;
  end else begin
    ShowMessage('初始化日志文件失败');
    Result := False;
  end;
end;

procedure TFrmSmartLog.tvListGetImageIndex(Sender: TObject;
  Node: TTreeNode);
begin
  if Node.Count > 0 then
  begin
    if Node.Expanded then
       Node.ImageIndex := 1
    else
       Node.ImageIndex := 2;
  end else
    Node.ImageIndex := 0;
end;

procedure TFrmSmartLog.tvListGetSelectedIndex(Sender: TObject;
  Node: TTreeNode);
begin
  if Node.Count > 0 then
  begin
    if Node.Expanded then
       Node.ImageIndex := 1
    else
       Node.ImageIndex := 2;
  end else
    Node.ImageIndex := 0;
end;

procedure TFrmSmartLog.TreeViewSelChg;
var
  Node: TTreeNode;
begin
  Node := tvList.Selected;
  if Node <> nil then
  begin
    if Node.Count = 0 then
    begin
       LoadNodeChild(Node);
    end;
    LoadNodeLog(Node);
  end;
end;

procedure TFrmSmartLog.LoadNodeChild(Node: TTreeNode);
var
  pItems, p: PLOG_NODE_ITEM;
  nCount: integer;
  i: integer;
  ParentId: integer;
begin
  pItems := nil;
  nCount := 0;
  if Node = nil then
     ParentId := 0
  else
     ParentId := integer(Node.Data);
  if LM_GetChildNodes(ParentId, @pItems, @nCount) then
  begin
    if pItems <> nil then
    begin
      p := pItems;
      for i := 0 to nCount - 1 do
      begin
        tvList.Items.AddChildObject(Node, p^.szNodeName, Pointer(p^.nNodeId));
        Inc(p);
      end;
      LM_DeleteNodeItems(pItems);
    end;
  end;
end;

procedure TFrmSmartLog.LoadNodeLog(Node: TTreeNode);
var
  szTitle: PChar;
  szContent: PChar;
  nBufSize: integer;
  nReadTimes: integer;
  nCurrLine: integer;
  szCreateDate: string;
  szLastModiDate: string;
  Affixs, pItem: PInteger;
  nAffixCount: integer;
  i: integer;
  Item: TListItem;
  szFileName: string;
  Stream: TMemoryStream;
  pWisdoms, p: PWISDOM_ITEM;
  nCount: integer;
begin
  if FPreSelNode <> nil then
     SaveNodeLog(FPreSelNode);
  if (Node <> nil) and (Node <> FPreSelNode) then
  begin
    RefreshFormCaption(Node);
    szTitle := nil;
    szContent := nil;
    nBufSize := 0;
    SetLength(szCreateDate, 256);
    SetLength(szLastModiDate, 256);
    FillChar(szCreateDate[1], 256, 0);
    FillChar(szLastModiDate[1], 256, 0);
    SetLength(szFileName, 256);
    FillChar(szFileName[1], 256, 0);
    lvAffix.Clear;
    case DWORD(Node.Data) of
      WISDOM_TREE_NODE_ID:
           begin
             //
             Editor.Clear;
             if LM_GetAllWisdom(@pWisdoms, @nCount) then
             begin
               p := pWisdoms;
               for i := 0 to nCount - 1 do
               begin
                 Editor.SelStart := -1;
                 if (i mod 2) = 0 then
                    Editor.SelAttributes.Color := clBlack
                 else
                    Editor.SelAttributes.Color := clGray;
                 Editor.SelAttributes.Size := 10;
                 Editor.SelAttributes.Style := [fsBold];
                 Editor.SelText := IntToStr(i + 1) + '、' + Trim(p^.szWisdom) + #13#10;
                 Inc(p);
               end;
               LM_DeleteWisdoms(pWisdoms);
             end;
           end;
      SEARCH_TREE_NODE_ID:
           begin
             //
           end;
      else
      if LM_GetLog(integer(Node.Data), @szTitle, @szContent, @nBufSize,
               PChar(szCreateDate), PChar(szLastModiDate), @nReadTimes, @nCurrLine) then begin
         lblCreateDate.Caption := szCreateDate;
         lblModiDate.Caption := szLastModiDate;
         Stream := TMemoryStream.Create;
         try
           Stream.WriteBuffer(szContent^, nBufSize);
           Stream.Position := 0;
           Editor.Lines.LoadFromStream(Stream);
           Editor.Modified := False;
           GetCommentById(integer(Node.Data));
           Editor.ReadOnly := True;
           Editor.SetFocus;

           SendMessage(Editor.Handle, WM_VSCROLL, SB_VERT,
                       SendMessage(Editor.Handle, EM_LINEFROMCHAR, nCurrLine, 0));
           
         finally
           Stream.Free;
         end;
         //读取附件
         Affixs := nil;
         if LM_GetAffixList(integer(Node.Data), @Affixs, @nAffixCount) then
         begin
           pItem := Affixs;
           for i := 0 to nAffixCount - 1 do
           begin
             FillChar(szFileName[1], 256, 0);
             if LM_GetAffixInfo(pItem^, PChar(szFileName)) then
             begin
               Item := lvAffix.Items.Add;
               Item.Caption := szFileName;
               Item.Data := Pointer(pItem^);
             end;
             Inc(pItem);
           end;
           if Affixs <> nil then
              LM_DeleteInt(Affixs);
         end;
      end else
         ShowMessage('获取日志失败');
      if szTitle <> nil then
         LM_DeleteString(szTitle);
      if szContent <> nil then
         LM_DeleteString(szContent);
    end;
    FPreSelNode := Node;
  end;
  Editor.Modified := False;
  FModified := False;
end;

procedure TFrmSmartLog.tvListClick(Sender: TObject);
begin
  TreeViewSelChg;
end;

procedure TFrmSmartLog.tvListEdited(Sender: TObject; Node: TTreeNode;
  var S: String);
begin
  if Node <> nil then
  begin
    LM_UpdateTitle(integer(Node.Data), PChar(S));
  end;
end;

procedure TFrmSmartLog.SaveNodeLog(Node: TTreeNode);
var
  Stream: TMemoryStream;
  StrAffix: string;
  i: integer;
  nLogId: integer;
begin
  if Node <> nil then
  begin
    nLogId := integer(Node.Data);
    if CheckIsSpecLogId(nLogId)  then
       Exit;
    LM_UpdateLogLines(nLogId, Editor.SelStart);
    if (not Editor.ReadOnly) and (Editor.Modified or FModified) then
    begin
      StrAffix := '';
      for i := 0 to lvAffix.Items.Count - 1 do
      begin
        if lvAffix.Items.Item[i].Data = nil then
        begin
          StrAffix := StrAffix + lvAffix.Items.Item[i].Caption + '|';
        end;
      end;
      Stream := TMemoryStream.Create;
      try
        Editor.Lines.SaveToStream(Stream);
        if LM_UpdateLog(nLogId, PChar(Stream.Memory), Stream.Size, PChar(StrAffix), False) then
        begin
          Editor.Modified := False;
          FModified := False;
          Editor.PlainText := True;
          Stream.Clear;
          Editor.Lines.SaveToStream(Stream);
          if not LM_UpdateLogPlainText(nLogId, PChar(Stream.Memory), Stream.Size) then
             ShowMessage('更新日志文字失败');
          Editor.PlainText := False;
        end else
          ShowMessage('更新日志失败');
      finally
        Stream.Free;
      end;
    end;
  end;
end;

procedure TFrmSmartLog.miCreateChildNodeClick(Sender: TObject);
var
  nParentId: integer;
  StrLog: string;
  nLogId: integer;
  Node: TTreeNode;
begin
  StrLog := InputBox('输入日志名称', '输入日志或者节点名称', '');
  if StrLog <> '' then
  begin
    Node := tvList.Selected;
    if Node <> nil then
       nParentId := integer(Node.Data)
    else
       nParentId := 0;
    nLogId := 0;
    if LM_AddLog(@nLogId, nParentId, PChar(StrLog)) then
    begin
      tvList.Items.AddChildObject(Node, StrLog, Pointer(nLogId));
    end else
      ShowMessage('增加日志失败');
  end;
end;

procedure TFrmSmartLog.miDelAffixClick(Sender: TObject);
var
  Item: TListItem;
  nAffixId: integer;
begin
  Item := lvAffix.Selected;
  if Item <> nil then
  begin
    nAffixId := 0;
    if Item.Data <> nil then
    begin
      nAffixId := integer(Item.Data);
    end;
    lvAffix.DeleteSelected;
    if nAffixId > 0 then
    begin
       if LM_DeleteAffix(nAffixId) then
          ShowMessage('删除附件成功')
       else
          ShowMessage('删除附件失败');
    end;
  end;
end;

procedure TFrmSmartLog.miDelLogClick(Sender: TObject);
var
  Node: TTreeNode;
  nLogId: integer;
begin
  Node := tvList.Selected;
  if Node <> nil then
  begin
    if Node.Count > 0 then
    begin
      ShowMessage('不能删除有子节点的日志');
      Exit;
    end;
    if Node.Data <> nil then
    begin
      nLogId := integer(Node.Data);
      if nLogId > 0 then
      begin
        if LM_DeleteLog(nLogid) then
           ShowMessage('删除日志成功')
        else
           ShowMessage('删除日志失败');
      end;
    end;
    tvList.Items.Delete(Node);
  end;
end;

procedure TFrmSmartLog.tvListChange(Sender: TObject; Node: TTreeNode);
begin
  LoadNodeLog(Node);
end;

procedure TFrmSmartLog.InsertImage(FileName: string);
var
  RichOle: IRichEditOle;
  LockByte: ILockBytes;
  Storage: IStorage;
  FormatEtc: TFormatEtc;
  OleObj: IOleObject;
  ReObj: _REOBJECT;
  xt: TGUID;
  Site: IOleClientSite;
  wFileName: array[0..MAX_PATH - 1] of WideChar;
begin
  MultiByteToWideChar(Windows.GetACP,  0, PAnsiChar(FileName), -1, wFileName, MAX_PATH);
  SendMessage(Editor.Handle,  EM_GETOLEINTERFACE, 0, integer(@RichOle));
  CreateILockBytesOnHGlobal(0, True, LockByte);
  StgCreateDocfileOnILockBytes(LockByte, STGM_SHARE_EXCLUSIVE or STGM_CREATE or STGM_READWRITE,
                 0, Storage);
  FormatEtc.cfFormat := 0;
  FormatEtc.ptd := nil;
  FormatEtc.dwAspect := DVASPECT_CONTENT;
  FormatEtc.lindex := -1;
  FormatEtc.tymed := TYMED_NULL;
  FormatEtc.cfFormat := 0;
  FormatEtc.ptd := nil;
  FormatEtc.dwAspect := DVASPECT_CONTENT;
  FormatEtc.lindex := -1;
  FormatEtc.tymed := TYMED_NULL;
  OleCreateFromFile(GUID_NULL, wFileName, IID_IOleObject, 0,
     @FormatEtc, nil, Storage, OleObj);
  OleSetContainedObject(OleObj, True);
  FillChar(ReObj, SizeOf(_ReObject), 0);
  ReObj.cbStruct := SizeOf(_ReObject);
  OleObj.GetUserClassID(xt);
  ReObj.clsid := xt;
  ReObj.cp := REO_CP_SELECTION;

  ReObj.dvaspect :=DVASPECT_CONTENT;
  ReObj.dwFlags := REO_STATIC or REO_BELOWBASELINE;
  ReObj.dwUser := 0;
  ReObj.poleobj := OleObj;
  RichOle.GetClientSite(Site);
  Reobj.polesite := Site;
  ReObj.pstg := Storage;
  ReObj.sizel.cx := 0;
  ReObj.sizel.cy := 0;
  RichOle.InsertObject(ReObj);
end;

procedure TFrmSmartLog.EditInsertImageExecute(Sender: TObject);
var
  Dlg: TOpenDialog;
begin
  Dlg := TOpenDialog.Create(Self);
  try
    if Dlg.Execute then
       InsertImage(Dlg.FileName);
  finally
    Dlg.Free;
  end;
end;

procedure TFrmSmartLog.miCreateTopNodeClick(Sender: TObject);
var
  StrLog: string;
  nLogId: integer;
begin
  StrLog := InputBox('输入日志名称', '输入日志或者节点名称', '');
  if StrLog <> '' then
  begin
    nLogId := 0;
    if LM_AddLog(@nLogId, 0, PChar(StrLog)) then
    begin
      tvList.Items.AddChildObject(nil, StrLog, Pointer(nLogId));
    end else
      ShowMessage('增加日志失败');
  end;
end;

procedure TFrmSmartLog.fdTextFind(Sender: TObject);
var
  nPos, FindAt, nLength: integer;
  nLine: integer;
begin
  FSearchText := fdText.FindText;
  if frWholeWord in fdText.Options then
     Include(FSearchType, stWholeWord);
  if frMatchCase in fdText.Options then
     Include(FSearchType, stMatchCase);
  nPos := Editor.SelStart + Editor.SelLength;
  nLength := Length(Editor.Text) - nPos;
  FindAt := Editor.FindText(FSearchText, nPos, nLength, FSearchType);
  if FindAt <> -1 then
  begin
    Editor.SelStart := FindAt;
    Editor.SelLength := Length(FSearchText);
    nLine := SendMessage(Editor.Handle, EM_LINEFROMCHAR, Editor.SelStart, 0);
    Editor.SetFocus;
    SendMessage(Editor.Handle, WM_VSCROLL, SB_VERT, nLine + 2);
  end else
    ShowMessage('没有找到匹配的字符串：' + FSearchText);
end;

procedure TFrmSmartLog.fdTextShow(Sender: TObject);
begin
  fdText.FindText := FSearchText;
end;

procedure TFrmSmartLog.pmEditorPopup(Sender: TObject);
begin
  if Editor.SelLength > 0 then
  begin
     miWisdom.Enabled := True;
     miComment.Enabled := True;
     miDeleteComment.Enabled := True;
  end else
  begin
     miWisdom.Enabled := False;
     miComment.Enabled := False;
     miDeleteComment.Enabled := False;
  end;
end;

procedure TFrmSmartLog.miWisdomClick(Sender: TObject);
var
  Str: string;
  nRow: integer;
begin
  Str := Editor.SelText;
  if (Str <> '' ) and  (tvList.Selected <> nil) then
  begin
    nRow := SendMessage(Editor.Handle, EM_EXLINEFROMCHAR, 0,  Editor.SelStart);
    if LM_AddWisdom(Integer(tvList.Selected.Data), nRow, PChar(Str)) then
       ShowMessage('加入名言警句成功')
    else
       ShowMessage('加入名方警句失败，有可能已经存在');
  end
end;

procedure TFrmSmartLog.tvListDragOver(Sender, Source: TObject; X,
  Y: Integer; State: TDragState; var Accept: Boolean);
begin
  if Source = tvList then
  begin
    if (tvList.Selected <> nil) then
       Accept := True
    else
       Accept := False;
  end else
    Accept := False;
end;

procedure TFrmSmartLog.tvListEndDrag(Sender, Target: TObject; X,
  Y: Integer);
var
  XYNode: TTreeNode;
  SelNode: TTreeNode;
begin
  if (Sender = tvList) and (Target = tvList) and (tvList.Selected <> nil) then
  begin
    SelNode := tvList.Selected;
    XYNode := tvList.GetNodeAt(X, Y);
    if (XYNode <> nil) and (XYNode <> SelNode) and (XYNode <> SelNode.Parent) then
    begin
      if LM_UpdateParentLogId(integer(SelNode.Data), integer(XYNode.Data)) then
         SelNode.MoveTo(XYNode, naAddChild);
    end;
  end;
end;

{ TMyHintWindow }

procedure TMyHintWindow.ActivateHint(Rect: TRect; const AHint: string);
var
  P: TPoint;
begin
  //在这里取得一个适当的尺寸显示文字
  FHintBmp.Width := Rect.Right - Rect.Left;
  FHintBmp.Height := Rect.Bottom - Rect.Top + 4;
  DrawHintImg(FHintBmp, AHint);
  FWndBmp.Width := Rect.Right - Rect.Left + 23;
  FWndBmp.Height := Rect.Bottom - Rect.Top + 27;
  Inc(Rect.Right, 23);
  Inc(Rect.Bottom, 27);
  BoundsRect := Rect;
  if Left < Screen.DesktopLeft then
     Left := Screen.DesktopLeft;
  if Top < Screen.DesktopTop then
     Top := Screen.DesktopTop;
  if Left + Width > Screen.DesktopWidth then
     Left := Screen.DesktopWidth - Width;
  if Top + Height > Screen.DesktopHeight then
     Top := Screen.DesktopHeight - Height;
  GetDesktopImg(FWndBmp, BoundsRect);
  EffectHandle(FWndBmp, FHintBmp);
  P := ClientToScreen(Point(0, 0));
  SetWindowPos(Handle, HWND_TOPMOST, P.X, P.Y, 0, 0,
  SWP_SHOWWINDOW or SWP_NOACTIVATE or SWP_NOSIZE);
end;

constructor TMyHintWindow.Create(Aowner: TComponent);
begin
  inherited Create(AOwner);
  FWndBmp := TBitmap.Create;
  FWndBmp.PixelFormat := pf24bit;
  FHintBmp := TBitmap.Create;
end;

procedure TMyHintWindow.CreateParams(var Params: TCreateParams);
begin
  inherited CreateParams(Params);
  Params.Style := Params.Style and (not WS_BORDER);
end;

destructor TMyHintWindow.Destroy;
begin
  FWndBmp.Free;
  FHintBmp.Free;
  inherited;
end;

procedure TMyHintWindow.DrawHintImg(Bmp: TBitmap; AHint: string);
var
  R: TRect;
begin
  Bmp.Canvas.Brush.Color := Application.HintColor;
  Bmp.Canvas.Pen.Color := Application.HintColor;
  Bmp.Canvas.Rectangle(0, 0, Bmp.Width, Bmp.Height);
  Bmp.Canvas.Font.Color := Screen.HintFont.Color;
  R := Rect(0, 0, Bmp.Width, Bmp.Height);
  Inc(R.Left, 2);
  Inc(R.Top, 2);
  DrawText(Bmp.Canvas.Handle, PChar(AHint), -1, R, DT_LEFT or DT_NOPREFIX
            or DT_WORDBREAK or DrawTextBiDiModeFlagsReadingOnly);
end;

procedure TMyHintWindow.EffectHandle(WndBmp, HintBmp: TBitmap);
var
  R: TRect;
  i, j: Integer;
  P: PByteArray;
  Transt, TranstAngle: Integer;
begin
  R := Rect(0, 0, WndBmp.Width - 4, WndBmp.Height - 4);
  Frame3D(WndBmp.Canvas, R, clMedGray, clBtnShadow, 1);
  //作窗口底下的阴影效果
  Transt := 60;
  for j:= WndBmp.Height - 4 to WndBmp.Height - 1 do
  begin
    P := WndBmp.ScanLine[j];
    TranstAngle := Transt;
    for i:= 3 to WndBmp.Width - 1 do
    begin
      //如果正处于右下角
      if i > WndBmp.Width - 5 then
      begin
        P[3*i] := P[3*i] * TranstAngle div 100;
        P[3*i + 1] := P[3*i + 1] * TranstAngle div 100;
        P[3*i + 2] := P[3*i + 2] * TranstAngle div 100;
        TranstAngle := TranstAngle + 10;
        if TranstAngle > 90 then
           TranstAngle := 90;
      end else begin
        P[3*i] := P[3*i] * Transt div 100;
        P[3*i + 1] := P[3*i + 1] * Transt div 100;
        P[3*i + 2] := P[3*i + 2] * Transt div 100;
      end;
    end;
    Transt := Transt + 10;
  end;
  // 作窗口右边的阴影效果
  for j := 3 to WndBmp.Height - 5 do
  begin
    P := WndBmp.ScanLine[j];
    Transt := 60;
    for i:= WndBmp.Width - 4 to WndBmp.Width -1 do
    begin
      P[3*i] := P[3*i] * Transt div 100;
      P[3*i + 1] := P[3*i + 1] * Transt div 100;
      P[3*i + 2] := P[3*i + 2] * Transt div 100;
      Transt := Transt + 10;
    end;
  end;
  WndBmp.Canvas.Draw(10, 10, HintBmp);
end;

procedure TMyHintWindow.GetDesktopImg(Bmp: TBitmap; R: TRect);
var
  C: TCanvas;
begin
  C:= TCanvas.Create;
  try
    C.Handle := GetDC(0);
    Bmp.Canvas.CopyRect(Rect(0, 0, Bmp.Width, Bmp.Height), C, R);
  finally
    C.Free;
  end;
end;

procedure TMyHintWindow.NCPaint(DC: HDC);
begin
  //
end;

procedure TMyHintWindow.Paint;
begin
  Canvas.CopyRect(ClientRect, FWndBmp.Canvas, ClientRect);
end;

procedure TFrmSmartLog.GetCommentById(const nLogId: integer);
var
  p: PLOG_COMMENT_ITEM;
  i: integer;
  cf: CHARFORMAT2;
begin
  if FLogComments <> nil then
  begin
     LM_DeleteCommentItems(FLogComments);
     FLogComments := nil;
  end;
  FLogCommentCount := 0;
  if LM_GetComments(nLogId, @FLogComments, @FLogCommentCount) then
  begin
    p := FLogComments;
    for i := 0 to FLogCommentCount - 1 do
    begin
      Editor.SelStart := p^.nStart;
      Editor.SelLength := p^.nLength;
      FillChar(cf, SizeOf(CHARFORMAT2), 0);
      cf.cbSize := SizeOf(CHARFORMAT2);
      cf.dwMask := CFM_BACKCOLOR;
      cf.crBackColor := COMMENT_BACK_COLOR;
      Editor.Perform(EM_SETCHARFORMAT, SCF_SELECTION, Longint(@cf));
      Inc(p);
    end;
    Editor.SelStart := 0; 
  end;
end;

procedure TFrmSmartLog.miCommentClick(Sender: TObject);
var
  Str: string;
  pBuf: PChar;
  nBufSize: integer;
  nCommentId: integer;
begin
  Str := Editor.SelText;
  if (Str <> '' ) and  (tvList.Selected <> nil) then
  begin
    pBuf := nil;
    if GetComment(tvList.Selected.Text, pBuf, nBufSize) then
    begin
      if LM_AddComment(@nCommentId, Integer(tvList.Selected.Data), Editor.SelStart, Editor.SelLength,
           pBuf, nBufSize) then
         ShowMessage('加入备注成功')
      else
         ShowMessage('加入备注失败');
    end;
    if (pBuf <> nil) then
       FreeMem(pBuf);
  end
end;

function TFrmSmartLog.GetCommentIdByCurrSel(
  const nSelStart: integer): integer;
var
  i: integer;
  P: PLOG_COMMENT_ITEM;
begin
  Result := 0;
  if FLogComments <> nil then
  begin
    P := FLogComments;
    for i := 0 to FLogCommentCount - 1 do
    begin
      if (nSelStart >= P^.nStart) and (nSelStart <= P^.nStart + P^.nLength) then
      begin
        Result := P^.nCommentId;
        break;
      end;
      Inc(P);
    end;
  end;
end;

procedure TFrmSmartLog.EditorKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if Editor.ReadOnly then
  begin
    if Key = VK_SPACE then
    begin
      SendMessage(Editor.Handle, WM_VSCROLL, SB_PAGEDOWN, 0);
    end;
  end
end;

procedure TFrmSmartLog.miDeleteCommentClick(Sender: TObject);
begin
  if (Editor.SelLength > 0) and  (tvList.Selected <> nil) then
  begin
    if LM_DeleteCommentBySel(Integer(tvList.Selected.Data), Editor.SelStart, Editor.SelLength) then
       ShowMessage('删除备注成功')
    else
       ShowMessage('删除备注失败');

  end
end;

procedure TFrmSmartLog.RefreshFormCaption(Node: TTreeNode);
var
  Str: string;
  ParentNode: TTreeNode;
begin
  ParentNode := Node;
  Str := '';
  while ParentNode <> nil do
  begin
    if Str = '' then
       Str := ParentNode.Text
    else
       Str := ParentNode.Text + '->' + Str;
    ParentNode := ParentNode.Parent;
  end;
  Str := Str + ' - ' + Application.Title;
  Caption := Str;
end;

procedure TFrmSmartLog.edtSearchTextEnter(Sender: TObject);
begin
  if (edtSearchText.Text = '') or (edtSearchText.Text = '输入搜索词') then
     edtSearchText.SelectAll;
end;

procedure TFrmSmartLog.sbSearchClick(Sender: TObject);
var
  pItems, p: PLOG_NODE_ITEM;
  nCount: integer;
  i: integer;
  Node: TTreeNode;
begin
  if (edtSearchText.Text = '') or (edtSearchText.Text = '输入搜索词') then
  begin
    ShowMessage('请输入搜索词');
    Exit;
  end;

  pItems := nil;
  nCount := 0;
  Node := GetSearchNode;
  if Node <> nil then
  begin
    Node.DeleteChildren;
    if LM_SearchText(PChar(edtSearchText.Text), @pItems, @nCount) then
    begin
      Node.Text := '搜索结果(' + IntToStr(nCount) + ')';
      if pItems <> nil then
      begin
        p := pItems;
        for i := 0 to nCount - 1 do
        begin
          tvList.Items.AddChildObject(Node, p^.szNodeName, Pointer(p^.nNodeId));
          Inc(p);
        end;
        LM_DeleteNodeItems(pItems);
      end;
    end;
  end;
end;

function TFrmSmartLog.GetSearchNode: TTreeNode;
var
  i: integer;
begin
  Result := nil;
  for i := 0 to tvList.Items.Count - 1 do
  begin
    if DWORD(tvList.Items.Item[i].Data) = SEARCH_TREE_NODE_ID then
    begin
      Result := tvList.Items.Item[i];
      break;
    end;
  end;
  if Result = nil then
  begin
    Result := tvList.Items.AddChildObject(nil, '搜索结果', Pointer(SEARCH_TREE_NODE_ID));
  end;
end;

function TFrmSmartLog.CheckIsSpecLogId(const nLogId: DWORD): Boolean;
begin
  Result := (nLogId = WISDOM_TREE_NODE_ID) or (nLogId = SEARCH_TREE_NODE_ID);
end;

procedure TFrmSmartLog.miFindClick(Sender: TObject);
begin
  fdText.Execute;
end;

function TFrmSmartLog.RegisterHotKey(const HotKey: string): Boolean;
var
  Modifier: Cardinal;
  wKey: WORD;
begin
  FMsgHotKeyId := GlobalAddAtom(PChar(HotKey))-$C000;
  StringHotKeyToWord(HotKey, Modifier, wKey);
  if Windows.RegisterHotKey(Handle, FMsgHotKeyId, Modifier, wkey) then
     Result := True
  else
     Result := False;
end;

procedure TFrmSmartLog.StringHotKeyToWord(const Str: string;
  var fsModifier: Cardinal; var nKey: Word);
var
  P: PChar;
  S: string;
  function GetNextStr(var P: PChar): string;
  var
    Start: PChar;
  begin
    Start := P;
    Result := '';
    while((P^ <> Chr(0)) and (P^ <> '+')) do
      Inc(P);
    if (P - Start) > 0 then
       SetString(Result, Start, P - Start);
    if P^ = '+' then
       Inc(P);
  end;
begin
  fsModifier := 0;
  P := PChar(Str);
  S := GetNextStr(P);
  nKey := Ord('I');
  while(S <> '') do begin
    if (S = 'CTRL') then
       fsModifier := fsModifier or MOD_CONTROL
    else if (S = 'ALT') then
       fsModifier := fsModifier or MOD_ALT
    else if (S = 'SHIFT') then
       fsModifier := fsModifier or MOD_SHIFT
    else if (Length(S) = 1) then
       nKey := Ord(S[1]);
    S := GetNextStr(P);
  end;
end;
procedure TFrmSmartLog.WMHOTKEY(var Msg: TMessage);
begin
  AddMemoLog;
end;

procedure TFrmSmartLog.miAccountClick(Sender: TObject);
begin
  ShowAccountForm;
end;

procedure TFrmSmartLog.miAutoClick(Sender: TObject);
begin
  ShowAutoForm;
end;

procedure TFrmSmartLog.miSetPwdClick(Sender: TObject);
begin
  SetPassword;
end;

procedure TFrmSmartLog.ToolButton4Click(Sender: TObject);
var
  Node: TTreeNode;
begin
  Node := tvList.Selected;
  if (Node <> nil) and (Node.Count = 0) and (Node.Data <> nil) then
  begin
    LM_IncReadLogTime(Integer(Node.Data));
  end;
end;

procedure TFrmSmartLog.miRefreshClick(Sender: TObject);
var
  Node: TTreeNode;
begin
  Node := tvList.Selected;
  if (Node <> nil) then
  begin
    Node.DeleteChildren;
    FPreSelNode := nil;
    LoadNodeLog(Node);
  end;
end;

{ TRichEdit20 }

procedure TRichEdit20.CreateParams(var Params: TCreateParams);
const
  RichEditModuleName = 'RICHED20.DLL';
  ARYHideScrollBars: array[Boolean] of DWORD = (ES_DISABLENOSCROLL, 0);
  HideSelections: array[Boolean] of DWORD = (ES_NOHIDESEL, 0);
begin
  if FRichEditModule = 0 then
  begin
    FRichEditModule := LoadLibrary(RichEditModuleName);
    if FRichEditModule <= HINSTANCE_ERROR then FRichEditModule := 0;
  end;
  inherited CreateParams(Params);
  CreateSubClass(Params, 'RICHEDIT50W');
  with Params do
  begin
    Style := Style or ARYHideScrollBars[HideScrollBars] or
      HideSelections[HideSelection];
    WindowClass.style := WindowClass.style and not (CS_HREDRAW or CS_VREDRAW);
  end;
end;

procedure TFrmSmartLog.miWorkFlowClick(Sender: TObject);
begin
  AddWorkFlow;
end;

procedure TFrmSmartLog.miSyncClick(Sender: TObject);
begin
  ShowSyncPath;
end;

initialization

finalization
  if (FRichEditModule <> 0) then
     FreeLibrary(FRichEditModule);
end.
