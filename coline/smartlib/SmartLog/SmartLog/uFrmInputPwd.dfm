object FrmInputPwd: TFrmInputPwd
  Left = 378
  Top = 377
  Width = 285
  Height = 125
  ActiveControl = edtPwd
  BorderIcons = [biSystemMenu]
  Caption = #36755#20837#23494#30721
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnCanResize = FormCanResize
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 24
    Width = 24
    Height = 13
    Caption = #23494#30721
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object edtPwd: TEdit
    Left = 40
    Top = 20
    Width = 201
    Height = 21
    PasswordChar = '*'
    TabOrder = 0
    OnKeyDown = edtPwdKeyDown
  end
  object btnOk: TButton
    Left = 40
    Top = 64
    Width = 75
    Height = 25
    Caption = #30830#23450'(&O)'
    TabOrder = 1
    OnClick = btnOkClick
  end
  object btnClose: TButton
    Left = 144
    Top = 64
    Width = 75
    Height = 25
    Caption = #20851#38381'(&C)'
    TabOrder = 2
    OnClick = btnCloseClick
  end
end
