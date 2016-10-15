object FrmSetPwd: TFrmSetPwd
  Left = 401
  Top = 255
  Width = 234
  Height = 182
  BorderIcons = [biSystemMenu]
  Caption = #35774#32622#23494#30721
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCanResize = FormCanResize
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 16
    Top = 16
    Width = 48
    Height = 13
    Caption = #21407#26377#23494#30721
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 28
    Top = 48
    Width = 36
    Height = 13
    Caption = #26032#23494#30721
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label3: TLabel
    Left = 16
    Top = 80
    Width = 48
    Height = 13
    Caption = #30830#35748#23494#30721
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object btnOk: TButton
    Left = 24
    Top = 112
    Width = 75
    Height = 25
    Caption = #30830#35748'(&O)'
    TabOrder = 0
    OnClick = btnOkClick
  end
  object btnClose: TButton
    Left = 120
    Top = 112
    Width = 75
    Height = 25
    Caption = #20851#38381'(&C)'
    TabOrder = 1
    OnClick = btnCloseClick
  end
  object edtOldPwd: TEdit
    Left = 72
    Top = 12
    Width = 121
    Height = 21
    PasswordChar = '*'
    TabOrder = 2
  end
  object edtNewPwd: TEdit
    Left = 72
    Top = 44
    Width = 121
    Height = 21
    PasswordChar = '*'
    TabOrder = 3
  end
  object edtChkPwd: TEdit
    Left = 72
    Top = 76
    Width = 121
    Height = 21
    PasswordChar = '*'
    TabOrder = 4
  end
end
