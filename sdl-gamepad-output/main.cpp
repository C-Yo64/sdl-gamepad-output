
// SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#pragma comment(lib, "SDL3.lib")

// Windows
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>

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

static void InitSDLGamepad();
static void GamepadEvents();

static void InitViGEm(int controllerType);
static void CreateVirtualController();
static void ShutdownViGEm();

int main(int argc, char* argv[]) {
	std::cout << "Type 1 for XInput, 2 for DS4\n";
	std::cin >> controllerType;

	InitSDLGamepad();
	GamepadEvents();
	
	ShutdownViGEm();
	return 1;
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

static void GamepadEvents() {
	while (running && SDL_WaitEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			running = false;
			break;


		case SDL_EVENT_GAMEPAD_ADDED:
			if (!gamepad) {
				gamepad = SDL_OpenGamepad(event.gdevice.which);
				if (gamepad) {
					std::cout << "Controller connected: " << SDL_GetGamepadName(gamepad) << std::endl;
					InitViGEm(controllerType);
				}
				else {
					std::cerr << "Failed to connect to controller: " << SDL_GetError() << std::endl;
				}
			}
			break;

		case SDL_EVENT_GAMEPAD_REMOVED:
			if (gamepad && event.gdevice.which == SDL_GetGamepadID(gamepad)) {
				std::cout << "Controller disconnected: " << SDL_GetGamepadName(gamepad) << std::endl;
				SDL_CloseGamepad(gamepad);
				gamepad = nullptr;
				running = false;
			}
			break;

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			if (controllerType == 1 && xinputTarget != nullptr) {
				XUSB_REPORT xinputReport;
				XUSB_REPORT_INIT(&xinputReport);

				// Map SDL gamepad buttons to XInput buttons
				switch (event.gbutton.button) {
				case SDL_GAMEPAD_BUTTON_SOUTH:
					xinputReport.wButtons ^= XUSB_GAMEPAD_A;
					break;
				case SDL_GAMEPAD_BUTTON_EAST:
					xinputReport.wButtons ^= XUSB_GAMEPAD_B;
					break;
				case SDL_GAMEPAD_BUTTON_WEST:
					xinputReport.wButtons ^= XUSB_GAMEPAD_X;
					break;
				case SDL_GAMEPAD_BUTTON_NORTH:
					xinputReport.wButtons ^= XUSB_GAMEPAD_Y;
					break;
				case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
					xinputReport.wButtons ^= XUSB_GAMEPAD_LEFT_SHOULDER;
					break;
				case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
					xinputReport.wButtons ^= XUSB_GAMEPAD_RIGHT_SHOULDER;
					break;
				case SDL_GAMEPAD_BUTTON_BACK:
					xinputReport.wButtons ^= XUSB_GAMEPAD_BACK;
					break;
				case SDL_GAMEPAD_BUTTON_START:
					xinputReport.wButtons ^= XUSB_GAMEPAD_START;
					break;
				case SDL_GAMEPAD_BUTTON_LEFT_STICK:
					xinputReport.wButtons ^= XUSB_GAMEPAD_LEFT_THUMB;
					break;
				case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
					xinputReport.wButtons ^= XUSB_GAMEPAD_RIGHT_THUMB;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_UP:
					xinputReport.wButtons ^= XUSB_GAMEPAD_DPAD_UP;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
					xinputReport.wButtons ^= XUSB_GAMEPAD_DPAD_DOWN;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
					xinputReport.wButtons ^= XUSB_GAMEPAD_DPAD_LEFT;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
					xinputReport.wButtons ^= XUSB_GAMEPAD_DPAD_RIGHT;
					break;
				case SDL_GAMEPAD_BUTTON_GUIDE:
					xinputReport.wButtons ^= XUSB_GAMEPAD_GUIDE;
					break;
				}
				vigem_target_x360_update(client, xinputTarget, xinputReport);
				break;

			}
			else if (controllerType == 2 && ds4Target != nullptr) {
				DS4_REPORT ds4Report;
				DS4_REPORT_INIT(&ds4Report);

				// Map SDL gamepad buttons to DS4 buttons
				switch (event.gbutton.button) {
				case SDL_GAMEPAD_BUTTON_SOUTH:
					ds4Report.wButtons ^= DS4_BUTTON_CROSS;
					break;
				case SDL_GAMEPAD_BUTTON_EAST:
					ds4Report.wButtons ^= DS4_BUTTON_CIRCLE;
					break;
				case SDL_GAMEPAD_BUTTON_WEST:
					ds4Report.wButtons ^= DS4_BUTTON_SQUARE;
					break;
				case SDL_GAMEPAD_BUTTON_NORTH:
					ds4Report.wButtons ^= DS4_BUTTON_TRIANGLE;
					break;
				case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
					ds4Report.wButtons ^= DS4_BUTTON_SHOULDER_LEFT;
					break;
				case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
					ds4Report.wButtons ^= DS4_BUTTON_SHOULDER_RIGHT;
					break;
				case SDL_GAMEPAD_BUTTON_BACK:
					ds4Report.wButtons ^= DS4_BUTTON_SHARE;
					break;
				case SDL_GAMEPAD_BUTTON_START:
					ds4Report.wButtons ^= DS4_BUTTON_OPTIONS;
					break;
				case SDL_GAMEPAD_BUTTON_LEFT_STICK:
					ds4Report.wButtons ^= DS4_BUTTON_THUMB_LEFT;
					break;
				case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
					ds4Report.wButtons ^= DS4_BUTTON_THUMB_RIGHT;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_UP:
					ds4Report.wButtons ^= DS4_BUTTON_DPAD_NORTH;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
					ds4Report.wButtons ^= DS4_BUTTON_DPAD_SOUTH;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
					ds4Report.wButtons ^= DS4_BUTTON_DPAD_WEST;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
					ds4Report.wButtons ^= DS4_BUTTON_DPAD_EAST;
					break;
				case SDL_GAMEPAD_BUTTON_GUIDE:
					ds4Report.wButtons ^= DS4_SPECIAL_BUTTON_PS;
					break;
				}
				vigem_target_ds4_update(client, ds4Target, ds4Report);
			}
			break;

			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
				if (controllerType == 1 && xinputTarget != nullptr) {
					XUSB_REPORT xinputReport;
					XUSB_REPORT_INIT(&xinputReport);
					// Map SDL gamepad axis to XInput axis

					switch (event.gaxis.axis) {
					case SDL_GAMEPAD_AXIS_LEFTX:
						xinputReport.sThumbLX = event.gaxis.value;
						break;
					case SDL_GAMEPAD_AXIS_LEFTY:
						xinputReport.sThumbLY = event.gaxis.value;
						break;
					case SDL_GAMEPAD_AXIS_RIGHTX:
						xinputReport.sThumbRX = event.gaxis.value;
						break;
					case SDL_GAMEPAD_AXIS_RIGHTY:
						xinputReport.sThumbRY = event.gaxis.value;
						break;
					case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
						xinputReport.bLeftTrigger = static_cast<BYTE>(event.gaxis.value);
						break;
					case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
						xinputReport.bRightTrigger = static_cast<BYTE>(event.gaxis.value);
						break;
					}
					vigem_target_x360_update(client, xinputTarget, xinputReport);
				}
				else if (controllerType == 2 && ds4Target != nullptr) {
					DS4_REPORT ds4Report;
					DS4_REPORT_INIT(&ds4Report);

					// Map SDL gamepad axis to DS4 axis
					switch (event.gaxis.axis) {
					case SDL_GAMEPAD_AXIS_LEFTX:
						ds4Report.bThumbLX = static_cast<BYTE>(event.gaxis.value);
						break;
					case SDL_GAMEPAD_AXIS_LEFTY:
						ds4Report.bThumbLY = static_cast<BYTE>(event.gaxis.value);
						break;
					case SDL_GAMEPAD_AXIS_RIGHTX:
						ds4Report.bThumbRX = static_cast<BYTE>(event.gaxis.value);
						break;
					case SDL_GAMEPAD_AXIS_RIGHTY:
						ds4Report.bThumbRY = static_cast<BYTE>(event.gaxis.value);
						break;
					case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
						ds4Report.bTriggerL = static_cast<BYTE>(event.gaxis.value);
						break;
					case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
						ds4Report.bTriggerR = static_cast<BYTE>(event.gaxis.value);
						break;
					}
					vigem_target_ds4_update(client, ds4Target, ds4Report);
				}
				break;
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

static void CreateVirtualController() {
	if (controllerType == 1) {
		std::cout << "Creating virtual XInput controller\n";
		xinputTarget = vigem_target_x360_alloc();

		VIGEM_ERROR error = vigem_target_add(client, xinputTarget);
		if (!VIGEM_SUCCESS(error)) {
			std::cerr << "Failed to add XInput target: " << error << std::endl;
			vigem_target_free(xinputTarget);
		}
		else {
			std::cout << "Virtual XInput controller created successfully\n";

			XUSB_REPORT xinputReport;
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

			DS4_REPORT ds4Report;
			DS4_REPORT_INIT(&ds4Report);

			vigem_target_ds4_update(client, ds4Target, ds4Report);
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
	}
	if (ds4Target != nullptr) {
		vigem_target_remove(client, ds4Target);
		vigem_target_free(ds4Target);
	}
	if (client != nullptr) {
		vigem_disconnect(client);
		vigem_free(client);
	}
	std::cout << "Shut down ViGEm client\n";
	return;
}