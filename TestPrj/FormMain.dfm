object frmMain: TfrmMain
  Left = 0
  Top = 0
  Caption = 'Wave Out Test'
  ClientHeight = 441
  ClientWidth = 624
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object Button1: TButton
    Left = 24
    Top = 24
    Width = 75
    Height = 25
    Action = actStart
    TabOrder = 0
  end
  object Button2: TButton
    Left = 120
    Top = 24
    Width = 75
    Height = 25
    Action = actStop
    TabOrder = 1
  end
  object groupboxSineGen: TGroupBox
    Left = 24
    Top = 96
    Width = 297
    Height = 113
    Caption = 'Sine Gen'
    TabOrder = 2
    object lblSineGenVol: TLabel
      Left = 17
      Top = 27
      Width = 40
      Height = 15
      Alignment = taRightJustify
      Caption = 'Volume'
    end
    object lblSineGenFreq: TLabel
      Left = 34
      Top = 75
      Width = 23
      Height = 15
      Alignment = taRightJustify
      Caption = 'Freq'
    end
    object trackbarSineGenVol: TTrackBar
      Left = 64
      Top = 24
      Width = 225
      Height = 45
      Max = 100
      Frequency = 5
      Position = 50
      TabOrder = 0
      OnChange = trackbarSineGenVolChange
    end
    object trackbarSineGenFreq: TTrackBar
      Left = 64
      Top = 72
      Width = 225
      Height = 45
      Max = 10000
      Min = 20
      Frequency = 500
      Position = 440
      TabOrder = 1
      OnChange = trackbarSineGenFreqChange
    end
  end
  object groupboxFMGen: TGroupBox
    Left = 336
    Top = 96
    Width = 273
    Height = 209
    Caption = 'FM Gen'
    TabOrder = 3
    object lblFMGenVol: TLabel
      Left = 9
      Top = 27
      Width = 40
      Height = 15
      Alignment = taRightJustify
      Caption = 'Volume'
    end
    object lblFMGenCarrierFreq: TLabel
      Left = 14
      Top = 75
      Width = 35
      Height = 15
      Alignment = taRightJustify
      Caption = 'Carrier'
    end
    object lblFMGenModFreq: TLabel
      Left = 15
      Top = 123
      Width = 34
      Height = 15
      Alignment = taRightJustify
      Caption = 'Mod F'
    end
    object lblFMGenModIndex: TLabel
      Left = 18
      Top = 171
      Width = 31
      Height = 15
      Alignment = taRightJustify
      Caption = 'Mod I'
    end
    object trackbarFMGenVol: TTrackBar
      Left = 56
      Top = 24
      Width = 209
      Height = 45
      Max = 100
      Frequency = 5
      Position = 50
      TabOrder = 0
      OnChange = trackbarFMGenVolChange
    end
    object trackbarFMGenCarrierFreq: TTrackBar
      Left = 56
      Top = 72
      Width = 209
      Height = 45
      Max = 10000
      Min = 20
      Frequency = 500
      Position = 440
      TabOrder = 1
      OnChange = trackbarFMGenCarrierFreqChange
    end
    object trackbarFMGenModFreq: TTrackBar
      Left = 56
      Top = 120
      Width = 209
      Height = 45
      Max = 5000
      Min = 1
      Frequency = 250
      Position = 220
      TabOrder = 2
      OnChange = trackbarFMGenModFreqChange
    end
    object trackbarFMGenModIndex: TTrackBar
      Left = 56
      Top = 168
      Width = 209
      Height = 45
      Max = 100
      Frequency = 5
      Position = 20
      TabOrder = 3
      OnChange = trackbarFMGenModIndexChange
    end
  end
  object ActionManager1: TActionManager
    Left = 376
    Top = 32
    StyleName = 'Platform Default'
    object actStart: TAction
      Caption = 'Start'
      OnExecute = actStartExecute
      OnUpdate = EnabledIfStopped
    end
    object actStop: TAction
      Caption = 'Stop'
      OnExecute = actStopExecute
      OnUpdate = EnabledIfRunning
    end
  end
end
