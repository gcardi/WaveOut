object frmeLevelMeter: TfrmeLevelMeter
  Left = 0
  Top = 0
  Width = 115
  Height = 240
  DoubleBuffered = True
  Color = clBtnFace
  ParentBackground = False
  ParentColor = False
  ParentDoubleBuffered = False
  TabOrder = 0
  OnResize = FrameResize
  object PaintBox1: TPaintBox
    AlignWithMargins = True
    Left = 3
    Top = 3
    Width = 109
    Height = 234
    Align = alClient
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clCream
    Font.Height = -11
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    OnPaint = PaintBox1Paint
    ExplicitLeft = 8
    ExplicitTop = 80
    ExplicitWidth = 105
    ExplicitHeight = 105
  end
  object tmrPeak: TTimer
    Enabled = False
    OnTimer = tmrPeakTimer
    Left = 8
    Top = 48
  end
end
