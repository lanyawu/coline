object frmWorkFlow: TfrmWorkFlow
  Left = 442
  Top = 191
  Width = 299
  Height = 292
  BorderIcons = [biSystemMenu]
  Caption = #24037#20316#27969#27700
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCanResize = FormCanResize
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 24
    Width = 36
    Height = 13
    Caption = #26102#38388#27573
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 8
    Top = 56
    Width = 48
    Height = 13
    Caption = #24037#20316#20869#23481
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object edtTimeSect: TEdit
    Left = 56
    Top = 20
    Width = 217
    Height = 21
    TabOrder = 0
  end
  object mmFlow: TMemo
    Left = 8
    Top = 80
    Width = 265
    Height = 137
    ScrollBars = ssVertical
    TabOrder = 1
  end
  object btnAdd: TButton
    Left = 96
    Top = 232
    Width = 75
    Height = 25
    Caption = #22686#21152'(&A)'
    TabOrder = 2
    OnClick = btnAddClick
  end
  object btnClose: TButton
    Left = 192
    Top = 232
    Width = 75
    Height = 25
    Caption = #20851#38381'(&C)'
    TabOrder = 3
    OnClick = btnCloseClick
  end
end
