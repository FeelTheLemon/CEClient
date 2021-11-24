#include "CEClient.h"
#include <Keyboard.h>
#include <Mouse.h>
#include "Keyboard_codes.h"

#define phy1 (unsigned char)((_physicalAddress >> 8) & 0xFF)
#define phy2 (unsigned char)((_physicalAddress >> 0) & 0xFF)

// create a new instance of CEClient
CEClient::CEClient(int physicalAddress, int inputPin, int outputPin) : 
	CEC_Device(physicalAddress, inputPin, outputPin<0 ? inputPin : outputPin) {

	_ready = false;
}

// init CEClient 
void CEClient::begin(CEC_DEVICE_TYPE type) {

	CEC_Device::Initialize(type);
}

// return the ready state
bool CEClient::isReady() {

	return _ready;
}

// write a packet on the bus
bool CEClient::write(int targetAddress, unsigned char* buffer, int count) {

	return CEC_Device::TransmitFrame(targetAddress, buffer, count);
}

// return the logical address
int CEClient::getLogicalAddress() {

	return _logicalAddress;
}

// enable-disable promiscuous mode
void CEClient::setPromiscuous(bool promiscuous) {

	Promiscuous = promiscuous;
}

// enable-disable monitor mode
void CEClient::setMonitorMode(bool monitorMode) {

	MonitorMode = monitorMode;
}

// set callback function when a transmit is complete
void CEClient::onTransmitCompleteCallback(OnTransmitCompleteCallbackFunction callback) {

	_onTransmitCompleteCallback = callback;
}

// set callback function when a new packet is received
void CEClient::onReceiveCallback(OnReceiveCallbackFunction callback) {

	_onReceiveCallback = callback;
}

// run, to be executed in the loop for the FSM
void CEClient::run() {

	CEC_Device::Run();
}


// ----- PRIVATE METHODS -----

// OnTransmitComplete redefinition, if a callback function is available, call it
void CEClient::OnTransmitComplete(bool success) {

	if(_onTransmitCompleteCallback) 
		_onTransmitCompleteCallback(success);
	
	CEC_Device::OnTransmitComplete(success);
}

void CEClient::reportPhysAddr(int dest)
{
	unsigned char frame[4] = { 0x84, phy1, phy2, 0x04 };
	TransmitFrame(0x0F,frame,sizeof(frame));
}

void CEClient::reportStreamState(int dest)
{	// report stream state (playing)
	unsigned char frame[3] = { 0x82, phy1, phy2 };
	TransmitFrame(0x0F,frame,sizeof(frame));
}

void CEClient::reportPowerState(int dest)
{	// report power state (on)
	unsigned char frame[2] = { 0x90, 0x00 };
	TransmitFrame(0x00,frame,sizeof(frame));
} 

void CEClient::reportCECVersion(int dest)
{	// report CEC version (v1.3a)
	unsigned char frame[2] = { 0x9E, 0x04 };
	TransmitFrame(0x00,frame,sizeof(frame));
}

void CEClient::reportOSDName(int dest)
{	// FIXME: name hardcoded
	unsigned char frame[5] = { 0x47, 'H','T','P','C' };
	TransmitFrame(0x00,frame,sizeof(frame));
}
void CEClient::reportVendorID(int dest)
{	// report fake vendor ID
	unsigned char frame[4] = { 0x87, 0x00, 0xF1, 0x0E };
	TransmitFrame(0x00,frame,sizeof(frame));
} 

void handleKey(unsigned char key) {
	switch (key) {
		case 0x00: Keyboard.press(KEY_RETURN); break; // OK key
		case 0x01: Keyboard.press(KEY_UP_ARROW); break;
		case 0x02: Keyboard.press(KEY_DOWN_ARROW); break;
		case 0x03: Keyboard.press(KEY_LEFT_ARROW); break;
		case 0x04: Keyboard.press(KEY_RIGHT_ARROW); break;
		case 0x0B: Keyboard.press(KEY_C); break; // list key
		case 0x0D: Keyboard.press(KEY_ESC); break; // back keyb1
		case 0x20:  break; // 0 key
		case 0x21:  break; // 1 key
		case 0x22:  break; // 2 key
		case 0x23:  break; // 3 key
		case 0x24:  break; // 4 key
		case 0x25:  break; // 5 key
		case 0x26:  break; // 6 key
		case 0x27:  break; // 7 key
		case 0x28:  break; // 8 key
		case 0x29:  break; // 9 key
		case 0x35: Keyboard.press(KEY_I); break; // i key
		case 0x44: Keyboard.press(KEY_P); break;  // media play
		case 0x45: Keyboard.press(KEY_X); break; // media stop
		case 0x46: Keyboard.press(KEY_SPACE); break; // media pause
		case 0x47:  break; // media record
		case 0x48:  break; // media left
		case 0x49:  break; // media right
		case 0x4B: Keyboard.press(KEY_PAGE_UP); break; // channel +
		case 0x4C: Keyboard.press(KEY_PAGE_DOWN); break; // channel -
		case 0x51: Keyboard.press(KEY_T); break; // subtitle key
		case 0x53: Keyboard.press(KEY_HOME); break; // tv guide key
		case 0x71: break; // blue key
		case 0x72: break; // red key
		case 0x73: break; // green key
		case 0x74: break; // yellow key
		case 0x76: break; // text key
		default: DbgPrint("unknown key: 0x%02X\n", key); break;
	}
}

// OnReceive redefinition
void CEClient::OnReceive(int source, int dest, unsigned char* buffer, int count) {
	//if (MonitorMode)
	//{
		CEC_Device::OnReceive(source,dest,buffer,count);
	//	return;
	//}
	if (count == 0) return;
	//if (dest != _logicalAddress) return;
	
	switch (buffer[0]) {
		case 0x36:
			DbgPrint("standby\n");
			break;
		case 0x83:
			reportPhysAddr(source);
			break;
		case 0x86:
			if (buffer[1] == phy1 && buffer[2] == phy2)
				reportStreamState(source);
			break;
		case 0x8F:
			reportPowerState(source);
			break;
		case 0x9F:
			reportCECVersion(source);
			break;
		case 0x46:
			reportOSDName(source);
			break;
		case 0x8C:
			reportVendorID(source);
			break;
		case 0x44:
			handleKey(buffer[1]);
			break;
		case 0x45:
			Keyboard.releaseAll();
			break;
		default:
			CEC_Device::OnReceive(source,dest,buffer,count);
			break;
	}
}

// OnReady redefinition, to save the current status
void CEClient::OnReady() {

	_ready = true;
}
