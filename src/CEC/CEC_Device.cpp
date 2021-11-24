#include "CEC_Device.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_STM32F1) || defined(ARDUINO_ARCH_STM32F2) || defined(ARDUINO_ARCH_STM32F3) || defined(ARDUINO_ARCH_STM32F4)
# define STM32
# define CEC_HIGH 1
# define CEC_LOW 0
#else
# define CEC_HIGH LOW
# define CEC_LOW HIGH
#endif

CEC_Device::CEC_Device(int physicalAddress, int in_line, int out_line)
: CEC_LogicalDevice(physicalAddress)
, _isrTriggered(false)
, _lastLineState2(true)
, _in_line(in_line)
, _out_line(out_line)
{
}

void CEC_Device::Initialize(CEC_DEVICE_TYPE type)
{
#ifdef STM32
  pinMode(_in_line, OUTPUT_OPEN_DRAIN);
  _out_line = _in_line;
#else
  pinMode(_out_line, OUTPUT);
  pinMode( _in_line,  INPUT);
#endif  

  digitalWrite(_out_line, CEC_HIGH);
  delay(200);

  CEC_LogicalDevice::Initialize(type);
}

void CEC_Device::OnReady()
{
  // This is called after the logical address has been
  // allocated
  DbgPrint("Device ready\r\n");
}

void CEC_Device::OnReceive(int source, int dest, unsigned char* buffer, int count)
{
  // This is called when a frame is received.  To transmit
  // a frame call TransmitFrame.  To receive all frames, even
  // those not addressed to this device, set Promiscuous to true.
  DbgPrint("Packet received at %ld: %02d -> %02d: %02X \r\n", millis(), source, dest, ((source&0x0f)<<4)|(dest&0x0f));
  DbgPrint("Packet:\r\n");
  for (int i = 0; i < count; i++)
    DbgPrint("0x%02X ", buffer[i]);
  DbgPrint("\r\n");
}

bool CEC_Device::LineState()
{
  int state = digitalRead(_in_line);
  return state == CEC_HIGH;
}

void CEC_Device::SetLineState(bool state)
{
  digitalWrite(_out_line, state?CEC_HIGH:CEC_LOW);
  // give enough time for the line to settle before sampling
  // it
  delayMicroseconds(50);
  _lastLineState2 = LineState();
}

void CEC_Device::SignalIRQ()
{
  // This is called when the line has changed state
  _isrTriggered = true;
}

bool CEC_Device::IsISRTriggered()
{
  if (_isrTriggered)
  {
    _isrTriggered = false;
    return true;
  }
  return false;
}

void CEC_Device::Run()
{
  bool state = LineState();
  if (_lastLineState2 != state)
  {
    _lastLineState2 = state;
    SignalIRQ();
  }
  CEC_LogicalDevice::Run();
}
