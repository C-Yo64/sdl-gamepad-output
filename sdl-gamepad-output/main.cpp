
// SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#pragma comment(lib, "SDL3.lib")

// Windows
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <thread>

// ViGEm
#include <ViGEm/Client.h>
#pragma comment(lib, "ViGEmClient.lib")

#pragma comment(lib, "setupapi.lib")


SDL_Gamepad* gamepad = nullptr;
SDL_Event event;
bool running = false;

int controllerType;

PVIGEM_CLIENT client = nullptr;
PVIGEM_TARGET xinputTarget = nullptr;
PVIGEM_TARGET ds4Target = nullptr;

XUSB_REPORT xinputReport;
DS4_REPORT_EX ds4Report;

bool DS4_DPADUP = false;
bool DS4_DPADDOWN = false;
bool DS4_DPADLEFT = false;
bool DS4_DPADRIGHT = false;
static USHORT g_ds4Timestamp = 0;

static void Exit();

static void InitSDLGamepad();
static void UpdateDS4Dpad();
static void GamepadEvents();

static void InitViGEm(int controllerType);
static void CreateVirtualController();
static void ShutdownViGEm();

int main(int argc, char* argv[]) {
	atexit(Exit);
	std::cout << "Type 1 for XInput, 2 for DS4\n";
	std::cin >> controllerType;

	InitSDLGamepad();
	GamepadEvents();
	return 1;
}

static void Exit() {
	if (gamepad) {
		std::cout << "Controller disconnected: " << SDL_GetGamepadName(gamepad) << std::endl;
		SDL_CloseGamepad(gamepad);
		gamepad = nullptr;
	}
	ShutdownViGEm();
	client = nullptr;
	SDL_Quit();
}

static void InitSDLGamepad() {

	if (!SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_SENSOR)) {
		std::cerr << "SDL Initialization Error: " << SDL_GetError() << std::endl;
		return;
	}

	//gamepad = nullptr;
	running = true;

	std::cout << "Connect a controller\n";
}

static void UpdateDS4Dpad() {
	if (DS4_DPADUP && DS4_DPADRIGHT) {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_NORTHEAST);
	}
	else if (DS4_DPADUP && DS4_DPADLEFT) {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_NORTHWEST);
	}
	else if (DS4_DPADDOWN && DS4_DPADRIGHT) {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_SOUTHEAST);
	}
	else if (DS4_DPADDOWN && DS4_DPADLEFT) {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_SOUTHWEST);
	}
	else if (DS4_DPADUP) {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_NORTH);
	}
	else if (DS4_DPADDOWN) {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_SOUTH);
	}
	else if (DS4_DPADLEFT) {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_WEST);
	}
	else if (DS4_DPADRIGHT) {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_EAST);
	}
	else {
		DS4_SET_DPAD((PDS4_REPORT)&ds4Report.Report, DS4_BUTTON_DPAD_NONE);
	}
}

static void GamepadEvents() {
	while (running) {
		if (SDL_WaitEventTimeout(&event, 2)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				running = false;
				break;


			case SDL_EVENT_GAMEPAD_ADDED:
				if (!gamepad && xinputTarget == nullptr && ds4Target == nullptr) {
					gamepad = SDL_OpenGamepad(event.gdevice.which);
					if (gamepad) {
						std::cout << "Controller connected: " << SDL_GetGamepadName(gamepad) << std::endl;

						// Enable motion sensors
						if (SDL_GamepadHasSensor(gamepad, SDL_SENSOR_GYRO)) {
							SDL_SetGamepadSensorEnabled(gamepad, SDL_SENSOR_GYRO, true);
							//SDL_SetGamepadSensorEnabled(gamepad, SDL_SENSOR_GYRO, 100);
							std::cout << "Gyroscope detected (DS4 mode is required to use it)" << std::endl;
						}
						if (SDL_GamepadHasSensor(gamepad, SDL_SENSOR_ACCEL)) {
							SDL_SetGamepadSensorEnabled(gamepad, SDL_SENSOR_ACCEL, true);
							//SDL_SetGamepadSensorEnabled(gamepad, SDL_SENSOR_ACCEL, 100);
							std::cout << "Accelerometer detected (DS4 mode is required to use it)" << std::endl;
						}

						InitViGEm(controllerType);
					}
					else {
						std::cerr << "Failed to connect to controller: " << SDL_GetError() << std::endl;
					}
				}
				break;

			case SDL_EVENT_GAMEPAD_REMOVED:
				if (gamepad && event.gdevice.which == SDL_GetGamepadID(gamepad)) {
					running = false;
				}
				break;

			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
				if (controllerType == 1 && xinputTarget != nullptr) {

					// Map SDL gamepad buttons to XInput buttons
					switch (event.gbutton.button) {
					case SDL_GAMEPAD_BUTTON_SOUTH:
						xinputReport.wButtons |= XUSB_GAMEPAD_A;
						break;
					case SDL_GAMEPAD_BUTTON_EAST:
						xinputReport.wButtons |= XUSB_GAMEPAD_B;
						break;
					case SDL_GAMEPAD_BUTTON_WEST:
						xinputReport.wButtons |= XUSB_GAMEPAD_X;
						break;
					case SDL_GAMEPAD_BUTTON_NORTH:
						xinputReport.wButtons |= XUSB_GAMEPAD_Y;
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
						xinputReport.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
						xinputReport.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;
						break;
					case SDL_GAMEPAD_BUTTON_BACK:
						xinputReport.wButtons |= XUSB_GAMEPAD_BACK;
						break;
					case SDL_GAMEPAD_BUTTON_START:
						xinputReport.wButtons |= XUSB_GAMEPAD_START;
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_STICK:
						xinputReport.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
						xinputReport.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_UP:
						xinputReport.wButtons |= XUSB_GAMEPAD_DPAD_UP;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
						xinputReport.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
						xinputReport.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
						xinputReport.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;
						break;
					case SDL_GAMEPAD_BUTTON_GUIDE:
						xinputReport.wButtons |= XUSB_GAMEPAD_GUIDE;
						break;
					}

					vigem_target_x360_update(client, xinputTarget, xinputReport);
					break;

				}
				else if (controllerType == 2 && ds4Target != nullptr) {

					// Map SDL gamepad buttons to DS4 buttons
					switch (event.gbutton.button) {
					case SDL_GAMEPAD_BUTTON_SOUTH:
						ds4Report.Report.wButtons |= DS4_BUTTON_CROSS;
						break;
					case SDL_GAMEPAD_BUTTON_EAST:
						ds4Report.Report.wButtons |= DS4_BUTTON_CIRCLE;
						break;
					case SDL_GAMEPAD_BUTTON_WEST:
						ds4Report.Report.wButtons |= DS4_BUTTON_SQUARE;
						break;
					case SDL_GAMEPAD_BUTTON_NORTH:
						ds4Report.Report.wButtons |= DS4_BUTTON_TRIANGLE;
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
						ds4Report.Report.wButtons |= DS4_BUTTON_SHOULDER_LEFT;
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
						ds4Report.Report.wButtons |= DS4_BUTTON_SHOULDER_RIGHT;
						break;
					case SDL_GAMEPAD_BUTTON_BACK:
						ds4Report.Report.wButtons |= DS4_BUTTON_SHARE;
						break;
					case SDL_GAMEPAD_BUTTON_START:
						ds4Report.Report.wButtons |= DS4_BUTTON_OPTIONS;
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_STICK:
						ds4Report.Report.wButtons |= DS4_BUTTON_THUMB_LEFT;
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
						ds4Report.Report.wButtons |= DS4_BUTTON_THUMB_RIGHT;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_UP:
						DS4_DPADUP = true;
						UpdateDS4Dpad();
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
						DS4_DPADDOWN = true;
						UpdateDS4Dpad();
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
						DS4_DPADLEFT = true;
						UpdateDS4Dpad();
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
						DS4_DPADRIGHT = true;
						UpdateDS4Dpad();
						break;
					case SDL_GAMEPAD_BUTTON_GUIDE:
						ds4Report.Report.wButtons |= DS4_SPECIAL_BUTTON_PS;
						break;
					}

					vigem_target_ds4_update_ex(client, ds4Target, ds4Report);
				}
				break;
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
				if (controllerType == 1 && xinputTarget != nullptr) {

					// Map SDL gamepad buttons to XInput buttons
					switch (event.gbutton.button) {
					case SDL_GAMEPAD_BUTTON_SOUTH:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_A;
						break;
					case SDL_GAMEPAD_BUTTON_EAST:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_B;
						break;
					case SDL_GAMEPAD_BUTTON_WEST:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_X;
						break;
					case SDL_GAMEPAD_BUTTON_NORTH:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_Y;
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_LEFT_SHOULDER;
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_RIGHT_SHOULDER;
						break;
					case SDL_GAMEPAD_BUTTON_BACK:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_BACK;
						break;
					case SDL_GAMEPAD_BUTTON_START:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_START;
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_STICK:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_LEFT_THUMB;
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_RIGHT_THUMB;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_UP:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_DPAD_UP;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_DPAD_DOWN;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_DPAD_LEFT;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_DPAD_RIGHT;
						break;
					case SDL_GAMEPAD_BUTTON_GUIDE:
						xinputReport.wButtons &= ~XUSB_GAMEPAD_GUIDE;
						break;
					}

					vigem_target_x360_update(client, xinputTarget, xinputReport);
					break;

				}
				else if (controllerType == 2 && ds4Target != nullptr) {

					// Map SDL gamepad buttons to DS4 buttons
					switch (event.gbutton.button) {
					case SDL_GAMEPAD_BUTTON_SOUTH:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_CROSS;
						break;
					case SDL_GAMEPAD_BUTTON_EAST:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_CIRCLE;
						break;
					case SDL_GAMEPAD_BUTTON_WEST:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_SQUARE;
						break;
					case SDL_GAMEPAD_BUTTON_NORTH:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_TRIANGLE;
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_SHOULDER_LEFT;
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_SHOULDER_RIGHT;
						break;
					case SDL_GAMEPAD_BUTTON_BACK:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_SHARE;
						break;
					case SDL_GAMEPAD_BUTTON_START:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_OPTIONS;
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_STICK:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_THUMB_LEFT;
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
						ds4Report.Report.wButtons &= ~DS4_BUTTON_THUMB_RIGHT;
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_UP:
						DS4_DPADUP = false;
						UpdateDS4Dpad();
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
						DS4_DPADDOWN = false;
						UpdateDS4Dpad();
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
						DS4_DPADLEFT = false;
						UpdateDS4Dpad();
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
						DS4_DPADRIGHT = false;
						UpdateDS4Dpad();
						break;
					case SDL_GAMEPAD_BUTTON_GUIDE:
						ds4Report.Report.wButtons &= ~DS4_SPECIAL_BUTTON_PS;
						break;
					}

					vigem_target_ds4_update_ex(client, ds4Target, ds4Report);
				}
				break;

			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
				if (controllerType == 1 && xinputTarget != nullptr) {

					// Map SDL gamepad axis to XInput axis (and scale it to a 0-255 range)
					switch (event.gaxis.axis) {
					case SDL_GAMEPAD_AXIS_LEFTX:
						xinputReport.sThumbLX = event.gaxis.value;
						break;
					case SDL_GAMEPAD_AXIS_LEFTY:
						xinputReport.sThumbLY = -event.gaxis.value;
						break;
					case SDL_GAMEPAD_AXIS_RIGHTX:
						xinputReport.sThumbRX = event.gaxis.value;
						break;
					case SDL_GAMEPAD_AXIS_RIGHTY:
						xinputReport.sThumbRY = -event.gaxis.value;
						break;
					case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
						xinputReport.bLeftTrigger = static_cast<BYTE>((event.gaxis.value / 32767.0) * 255);
						break;
					case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
						xinputReport.bRightTrigger = static_cast<BYTE>((event.gaxis.value / 32767.0) * 255);
						break;
					}

					vigem_target_x360_update(client, xinputTarget, xinputReport);
				}
				else if (controllerType == 2 && ds4Target != nullptr) {

					// Map SDL gamepad axis to DS4 axis (and scale it to a 0-255 range)
					switch (event.gaxis.axis) {
					case SDL_GAMEPAD_AXIS_LEFTX:
						ds4Report.Report.bThumbLX = static_cast<BYTE>((event.gaxis.value + 32767) / 65535.0 * 255);
						break;
					case SDL_GAMEPAD_AXIS_LEFTY:
						ds4Report.Report.bThumbLY = static_cast<BYTE>((event.gaxis.value + 32767) / 65535.0 * 255);
						break;
					case SDL_GAMEPAD_AXIS_RIGHTX:
						ds4Report.Report.bThumbRX = static_cast<BYTE>((event.gaxis.value + 32767) / 65535.0 * 255);
						break;
					case SDL_GAMEPAD_AXIS_RIGHTY:
						ds4Report.Report.bThumbRY = static_cast<BYTE>((event.gaxis.value + 32767) / 65535.0 * 255);
						break;
					case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
						ds4Report.Report.bTriggerL = static_cast<BYTE>((event.gaxis.value / 32767.0) * 255);
						break;
					case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
						ds4Report.Report.bTriggerR = static_cast<BYTE>((event.gaxis.value / 32767.0) * 255);
						break;
					}

					vigem_target_ds4_update_ex(client, ds4Target, ds4Report);
				}
				break;

			case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:

				if (controllerType == 2 && ds4Target != nullptr) {

					// Map SDL gamepad sensor data to DS4 report
					if (event.gsensor.sensor == SDL_SENSOR_GYRO) {
						const float GYRO_SCALE = 57.2957795f * 16.0f;
						ds4Report.Report.wGyroX = static_cast<SHORT>(event.gsensor.data[0] * GYRO_SCALE);
						ds4Report.Report.wGyroY = static_cast<SHORT>(-event.gsensor.data[1] * GYRO_SCALE);
						ds4Report.Report.wGyroZ = static_cast<SHORT>(-event.gsensor.data[2] * GYRO_SCALE);
						//std::cout << "Gyro: X=" << ds4Report.Report.wGyroX << ", Y=" << ds4Report.Report.wGyroY << ", Z=" << ds4Report.Report.wGyroZ << std::endl;
						//std::cout << "SDL Gyro: X=" << event.gsensor.data[0] << ", Y=" << event.gsensor.data[1] << ", Z=" << event.gsensor.data[2] << std::endl;
					}
					else if (event.gsensor.sensor == SDL_SENSOR_ACCEL) {
						const float ACCEL_SCALE = 8192.0f / 9.80665f;
						ds4Report.Report.wAccelX = static_cast<SHORT>(event.gsensor.data[0] * ACCEL_SCALE);
						ds4Report.Report.wAccelY = static_cast<SHORT>(-event.gsensor.data[1] * ACCEL_SCALE);
						ds4Report.Report.wAccelZ = static_cast<SHORT>(-event.gsensor.data[2] * ACCEL_SCALE);
						//std::cout << "Accel: X=" << ds4Report.Report.wAccelX << ", Y=" << ds4Report.Report.wAccelY << ", Z=" << ds4Report.Report.wAccelZ << std::endl;
						//std::cout << "SDL Accel: X=" << event.gsensor.data[0] << ", Y=" << event.gsensor.data[1] << ", Z=" << event.gsensor.data[2] << std::endl;
					}

					vigem_target_ds4_update_ex(client, ds4Target, ds4Report);
				}
				break;
			}
		}
	}
}

static void InitViGEm(int controllerType) {
	std::cout << "Starting ViGEm client\n";

	client = vigem_alloc();
	if (client == nullptr) {
		std::cerr << "Failed to allocate ViGEm client memory\n";
		return;
	}

	CONST VIGEM_ERROR error = vigem_connect(client);
	if (!VIGEM_SUCCESS(error)) {
		std::cerr << "Failed to connect to ViGEm bus: " << error << std::endl;
		vigem_free(client);
		return;
	}

	std::cout << "ViGEm started successfully\n";
	CreateVirtualController();
}

VOID CALLBACK X360Notification(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData) {
	SDL_RumbleGamepad(gamepad, static_cast<Uint16>(LargeMotor) * 257, static_cast<Uint16>(SmallMotor) * 257, 100);
}


std::atomic<bool> g_Running{ true };
std::thread DS4NotificationThread;

void DS4Notification() {
	DS4_OUTPUT_BUFFER buffer;

	while (g_Running) {
		VIGEM_ERROR err = vigem_target_ds4_await_output_report_timeout(client, ds4Target, 50, &buffer);
		if (VIGEM_SUCCESS(err) && gamepad != nullptr) {
			Uint8 SmallMotor = 0;
			Uint8 LargeMotor = 0;

			// Handle regular reports and EX reports
			if (buffer.Buffer[0] == 0x11) {
				// EX Report
				SmallMotor = buffer.Buffer[6];
				LargeMotor = buffer.Buffer[7];
			}
			else {
				// Standard Report
				SmallMotor = buffer.Buffer[4];
				LargeMotor = buffer.Buffer[5];
			}
			SDL_RumbleGamepad(gamepad, static_cast<Uint16>(LargeMotor) * 257, static_cast<Uint16>(SmallMotor) * 257, 100);
			//std::cout << "Rumble: LargeMotor=" << static_cast<int>(LargeMotor) << ", SmallMotor=" << static_cast<int>(SmallMotor) << std::endl;
		}
		else if (err == VIGEM_ERROR_TARGET_NOT_PLUGGED_IN || err == VIGEM_ERROR_INVALID_TARGET) {
			break;
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}


static void CreateVirtualController() {
	if (controllerType == 1) {
		std::cout << "Creating virtual XInput controller\n";
		xinputTarget = vigem_target_x360_alloc();

		VIGEM_ERROR error = vigem_target_add(client, xinputTarget);
		VIGEM_ERROR error2 = vigem_target_x360_register_notification(client, xinputTarget, X360Notification, nullptr);
		if (!VIGEM_SUCCESS(error)) {
			std::cerr << "Failed to add XInput target: " << error << std::endl;
			vigem_target_free(xinputTarget);
		}
		else if (!VIGEM_SUCCESS(error2)) {
			std::cerr << "Failed to register XInput notification: " << error2 << std::endl;
			vigem_target_free(xinputTarget);
		}
		else {
			std::cout << "Virtual XInput controller created successfully\n";

			XUSB_REPORT_INIT(&xinputReport);

			vigem_target_x360_update(client, xinputTarget, xinputReport);
		}
	}
	else if (controllerType == 2) {
		std::cout << "Creating virtual DS4 controller\n";

		ds4Target = vigem_target_ds4_alloc();

		VIGEM_ERROR error = vigem_target_add(client, ds4Target);
		if (!VIGEM_SUCCESS(error)) {
			std::cerr << "Failed to add DS4 target: " << error << std::endl;
			vigem_target_free(ds4Target);
		}
		else {
			std::cout << "Virtual DS4 controller created successfully\n";

			std::memset(&ds4Report, 0, sizeof(DS4_REPORT_EX));
			DS4_REPORT_INIT((PDS4_REPORT)&ds4Report.Report);

			UpdateDS4Dpad();
			ds4Report.Report.wAccelZ = 8192;

			vigem_target_ds4_update_ex(client, ds4Target, ds4Report);

			g_Running = true;
			DS4NotificationThread = std::thread(DS4Notification);
		}
	}
	else {
		std::cerr << "Invalid controller type selected\n";
	}

}

static void ShutdownViGEm() {
	if (xinputTarget != nullptr) {
		vigem_target_remove(client, xinputTarget);
		vigem_target_free(xinputTarget);
		xinputTarget = nullptr;
	}
	if (ds4Target != nullptr) {
		g_Running = false;
		if (DS4NotificationThread.joinable()) {
			DS4NotificationThread.join();
		}

		vigem_target_remove(client, ds4Target);
		vigem_target_free(ds4Target);
		ds4Target = nullptr;
	}
	if (client != nullptr) {
		vigem_disconnect(client);
		vigem_free(client);
	}
	std::cout << "Shut down ViGEm client\n";
	return;
}