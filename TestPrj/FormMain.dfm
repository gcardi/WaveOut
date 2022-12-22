object frmMain: TfrmMain
  Left = 0
  Top = 0
  Caption = 'frmMain'
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
