  
' Cooler Temperatures are sent & received as 2 byte unsigned integers. Temperatures are represented in degrees Kelvin x10
' Resolution is 0.1 degree so a Temperature of -22.3 degC = 250.7 degK is represented by the integer 2507
' Cooler Temperature = (Temperature in degC + 273) * 10. Actual Temperature in degC = (Cooler temperature - 2730)/10
' Note there is a latency of one reading when changing the cooler status. The new status is returned on the next reading.
' Maximum Temperature is +35.0degC and minimum temperature is -40degC

Public Sub CoolerTemp()   'Sends cooler Set-point + ON/OFF and receives current cooler temperature & status

Dim CoolTemp As Integer
Dim CoolTempK As Double
Dim CoolerStatus As String

CommandBlock(0) = 64
CommandBlock(1) = 30    ' Cooler command number is decimal 30 = 0x1E
CommandBlock(2) = 226
CommandBlock(3) = 9     ' 9*256 (=2304) + 226 = 0x9E2 = 2530 (-20degC or 253.0 degK)
CommandBlock(4) = 1     ' cooler on = 1, cooler off = 0
CommandBlock(5) = 0
CommandBlock(6) = 0     ' no parameters
CommandBlock(7) = 0

ByteCount& = 8          'command block size 8

Call SendBlock

ByteCount& = 3          'read back 2 bytes of temperature data, & 1 byte cooler control

Success = ReadFile(BlockIOHandle&, AddressFor(ReportBuffer(0)), ByteCount&, BytesReturned&, 0)

CoolTemp = (ReportBuffer(1) * 256) + ReportBuffer(0)
CoolTempK = (CoolTemp - 2730) / 10
If (ReportBuffer(2) > 0) Then CoolerStatus = "ON, " Else CoolerStatus = "OFF, "

MsgBox "Cooler = " + CoolerStatus + "Temperature = " + Str$(CoolTemp) + " degK*10, or " + Str$(CoolTempK) + " degC  ", vbInformation, "Cooler Information"

End Sub
