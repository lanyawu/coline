object frmSyncPath: TfrmSyncPath
  Left = 280
  Top = 208
  Width = 575
  Height = 489
  BorderIcons = [biSystemMenu]
  Caption = #21516#27493#25991#20214
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
  object TLabel
    Left = 16
    Top = 16
    Width = 36
    Height = 14
    Caption = #28304#36335#24452
    Font.Charset = GB2312_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = #23435#20307'-'#26041#27491#36229#22823#23383#31526#38598
    Font.Style = []
    ParentFont = False
  end
  object Label1: TLabel
    Left = 16
    Top = 56
    Width = 48
    Height = 14
    Caption = #30446#26631#36335#24452
    Font.Charset = GB2312_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = #23435#20307'-'#26041#27491#36229#22823#23383#31526#38598
    Font.Style = []
    ParentFont = False
  end
  object sbSrcPath: TSpeedButton
    Left = 416
    Top = 12
    Width = 23
    Height = 22
    Caption = '...'
    OnClick = sbSrcPathClick
  end
  object sbDestPath: TSpeedButton
    Left = 416
    Top = 52
    Width = 23
    Height = 22
    Caption = '...'
    OnClick = sbDestPathClick
  end
  object edtSrcPath: TEdit
    Left = 72
    Top = 13
    Width = 329
    Height = 21
    TabOrder = 0
    Text = 'edtSrcPath'
  end
  object edtDestPath: TEdit
    Left = 72
    Top = 53
    Width = 329
    Height = 21
    TabOrder = 1
    Text = 'edtDestPath'
  end
  object btnIndex: TBitBtn
    Left = 456
    Top = 51
    Width = 75
    Height = 25
    Caption = #21019#24314#32034#24341
    TabOrder = 2
    OnClick = btnIndexClick
  end
  object btnAnalyse: TBitBtn
    Left = 312
    Top = 88
    Width = 75
    Height = 25
    Caption = #27604#36739'(&C)'
    TabOrder = 3
    OnClick = btnAnalyseClick
  end
  object btnSyc: TBitBtn
    Left = 400
    Top = 88
    Width = 75
    Height = 25
    Caption = #21516#27493'(&S)'
    TabOrder = 4
    OnClick = btnSycClick
  end
  object mmInfo: TMemo
    Left = 8
    Top = 136
    Width = 537
    Height = 305
    Color = clMenuText
    Font.Charset = GB2312_CHARSET
    Font.Color = clWhite
    Font.Height = -13
    Font.Name = #23435#20307'-'#26041#27491#36229#22823#23383#31526#38598
    Font.Style = []
    Lines.Strings = (
      '')
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 5
  end
end
