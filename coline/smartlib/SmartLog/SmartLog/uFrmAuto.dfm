object FrmAuto: TFrmAuto
  Left = 235
  Top = 159
  Width = 564
  Height = 279
  BorderIcons = [biSystemMenu]
  Caption = #27773#36710#30456#20851
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCanResize = FormCanResize
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object gbOil: TGroupBox
    Left = 8
    Top = 8
    Width = 273
    Height = 193
    Caption = #27833#32791
    TabOrder = 0
    object Label1: TLabel
      Left = 152
      Top = 48
      Width = 48
      Height = 13
      Caption = #36215#22987#20844#37324
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label2: TLabel
      Left = 8
      Top = 24
      Width = 24
      Height = 13
      Caption = #21333#20215
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label3: TLabel
      Left = 152
      Top = 24
      Width = 24
      Height = 13
      Caption = #25968#37327
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label4: TLabel
      Left = 8
      Top = 48
      Width = 24
      Height = 13
      Caption = #37329#39069
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label5: TLabel
      Left = 152
      Top = 80
      Width = 24
      Height = 13
      Caption = #26085#26399
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label6: TLabel
      Left = 8
      Top = 80
      Width = 24
      Height = 13
      Caption = #26631#21495
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label7: TLabel
      Left = 8
      Top = 104
      Width = 24
      Height = 13
      Caption = #22791#27880
    end
    object edtMetre: TEdit
      Left = 184
      Top = 44
      Width = 81
      Height = 21
      TabOrder = 0
    end
    object edtPrice: TEdit
      Left = 40
      Top = 20
      Width = 65
      Height = 21
      TabOrder = 1
    end
    object edtCount: TEdit
      Left = 184
      Top = 20
      Width = 81
      Height = 21
      TabOrder = 2
    end
    object edtTotal: TEdit
      Left = 40
      Top = 44
      Width = 65
      Height = 21
      TabOrder = 3
    end
    object cbQua: TComboBox
      Left = 40
      Top = 76
      Width = 65
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 4
      Text = '93#'
      Items.Strings = (
        '93#'
        '97#')
    end
    object dtDate: TDateTimePicker
      Left = 184
      Top = 76
      Width = 81
      Height = 21
      Date = 40292.674004444450000000
      Time = 40292.674004444450000000
      TabOrder = 5
    end
    object mmComment: TMemo
      Left = 40
      Top = 112
      Width = 129
      Height = 73
      ScrollBars = ssVertical
      TabOrder = 6
    end
    object btnAdd: TButton
      Left = 176
      Top = 160
      Width = 67
      Height = 25
      Caption = #22686#21152'(&A)'
      TabOrder = 7
      OnClick = btnAddClick
    end
  end
  object GroupBox1: TGroupBox
    Left = 304
    Top = 8
    Width = 241
    Height = 193
    Caption = #20854#23427
    TabOrder = 1
    object Label8: TLabel
      Left = 8
      Top = 24
      Width = 24
      Height = 13
      Caption = #21333#20215
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label9: TLabel
      Left = 120
      Top = 24
      Width = 24
      Height = 13
      Caption = #25968#37327
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label10: TLabel
      Left = 8
      Top = 56
      Width = 24
      Height = 13
      Caption = #37329#39069
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label11: TLabel
      Left = 120
      Top = 56
      Width = 24
      Height = 13
      Caption = #26085#26399
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label12: TLabel
      Left = 8
      Top = 80
      Width = 24
      Height = 13
      Caption = #22791#27880
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object edtFeePrice: TEdit
      Left = 40
      Top = 20
      Width = 65
      Height = 21
      TabOrder = 0
    end
    object edtFeeCount: TEdit
      Left = 152
      Top = 20
      Width = 82
      Height = 21
      TabOrder = 1
    end
    object edtFeeTotal: TEdit
      Left = 40
      Top = 52
      Width = 65
      Height = 21
      TabOrder = 2
    end
    object dtFeeDate: TDateTimePicker
      Left = 151
      Top = 52
      Width = 82
      Height = 21
      Date = 40292.674004444450000000
      Time = 40292.674004444450000000
      TabOrder = 3
    end
    object mmFeeComment: TMemo
      Left = 32
      Top = 96
      Width = 121
      Height = 89
      ScrollBars = ssVertical
      TabOrder = 4
    end
    object btnFeeAdd: TButton
      Left = 160
      Top = 152
      Width = 75
      Height = 25
      Caption = #22686#21152'(&F)'
      TabOrder = 5
      OnClick = btnFeeAddClick
    end
  end
  object btnClose: TButton
    Left = 240
    Top = 216
    Width = 75
    Height = 25
    Caption = #20851#38381'(&C)'
    TabOrder = 2
    OnClick = btnCloseClick
  end
end
