object frmAccount: TfrmAccount
  Left = 357
  Top = 177
  Width = 338
  Height = 320
  BorderIcons = [biSystemMenu]
  Caption = #25910#25903
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCanResize = FormCanResize
  OnCreate = FormCreate
  DesignSize = (
    322
    282)
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 322
    Height = 257
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 0
    object Label1: TLabel
      Left = 8
      Top = 16
      Width = 24
      Height = 13
      Caption = #25910#20837
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label2: TLabel
      Left = 168
      Top = 16
      Width = 24
      Height = 13
      Caption = #25903#20986
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label3: TLabel
      Left = 8
      Top = 48
      Width = 24
      Height = 13
      Caption = #39033#30446
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label4: TLabel
      Left = 168
      Top = 48
      Width = 24
      Height = 13
      Caption = #32463#25163
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label5: TLabel
      Left = 8
      Top = 112
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
      Caption = #22320#22336
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object edtIncoming: TEdit
      Left = 40
      Top = 12
      Width = 113
      Height = 21
      TabOrder = 0
    end
    object edtPayout: TEdit
      Left = 200
      Top = 12
      Width = 97
      Height = 21
      TabOrder = 1
    end
    object cbItem: TComboBox
      Left = 40
      Top = 44
      Width = 113
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 2
      Text = #24037#36164
      Items.Strings = (
        #24037#36164
        #22870#37329
        #22806#39033
        #25152#24471#31246
        #25237#36164#25910#30410
        #25151#31199'('#21253#27700#30005')'
        #32784#29992#21697
        #20132#36890
        #25506#20146#25903#20986
        #26053#28216
        #39135#21697
        #34915#26381
        #20070#36153
        #21307#38498)
    end
    object cbUserName: TComboBox
      Left = 200
      Top = 44
      Width = 97
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 3
    end
    object dtDate: TDateTimePicker
      Left = 40
      Top = 108
      Width = 113
      Height = 21
      Date = 40292.563030590280000000
      Time = 40292.563030590280000000
      TabOrder = 4
    end
    object mmComment: TMemo
      Left = 8
      Top = 136
      Width = 305
      Height = 113
      Lines.Strings = (
        '')
      TabOrder = 5
    end
    object edtAddr: TEdit
      Left = 40
      Top = 76
      Width = 257
      Height = 21
      TabOrder = 6
    end
  end
  object btnAdd: TButton
    Left = 161
    Top = 262
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #22686#21152'(&A)'
    TabOrder = 1
    OnClick = btnAddClick
  end
  object btnClose: TButton
    Left = 248
    Top = 262
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #20851#38381'(&C)'
    TabOrder = 2
    OnClick = btnCloseClick
  end
end
