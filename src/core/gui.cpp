#include "gui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx9.h"


#include "globals.h"

#include <stdexcept>
#include "../fonts/Quicksand.h"
#include "../fonts/Segoe UI.h"
#include "../util/elements.h"
#include "../../ext/imgui/imgui_internal.h"
#include "../util/custom_elements.hpp"

#include <chrono>

c_gui cGui;

namespace fonts {
	ImFont* quicksand;
	ImFont* titleFont;
}



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wideParam, LPARAM longParam);

LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wideParam, LPARAM longParam);

bool gui::SetupWindowClass(const char* windowClassName) noexcept
{
	// populate window class
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = NULL;

	// register
	if (!RegisterClassEx(&windowClass))
		return false;
	return true;
}
void gui::DestroyWindowClass() noexcept
{
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::SetupWindow(const char* windowName) noexcept
{
	// create temp window
	window = CreateWindow(
		windowClass.lpszClassName,
		windowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		100,
		100,
		0,
		0,
		windowClass.hInstance,
		0
	);

	if (!window)
		return false;

	return true;
}
void gui::DestroyWindow() noexcept
{
	if (window)
		DestroyWindow(window);
}

bool gui::SetupDirectX() noexcept
{
	const auto handle = GetModuleHandle("d3d9.dll");

	if (!handle)
		return false;

	using CreateFn = LPDIRECT3D9(__stdcall*)(UINT);

	const auto create = reinterpret_cast<CreateFn>(GetProcAddress(
		handle,
		"Direct3DCreate9"
	));

	if (!create)
		return false;

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9)
		return false;

	D3DPRESENT_PARAMETERS params = {};
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = D3DFMT_UNKNOWN;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = NULL;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = window;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = NULL;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	if (d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_NULLREF,
		window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
		&params,
		&device
	) < 0) return false;

	return true;
}

void gui::DestroyDirectX() noexcept
{
	if (device)
	{
		device->Release();
		device = NULL;
	}

	if (d3d9)
	{
		d3d9->Release();
		d3d9 = NULL;
	}
}

void gui::Setup()
{
	if (!SetupWindowClass("astral"))
		throw std::runtime_error("Failed to create window class.");

	if(!SetupWindow("astral window"))
		throw std::runtime_error("Failed to create window.");

	if(!SetupDirectX())
		throw std::runtime_error("Failed to create device.");

	DestroyWindow();
	DestroyWindowClass();
}

void gui::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept
{
	auto params = D3DDEVICE_CREATION_PARAMETERS{};
	device->GetCreationParameters(&params);
	
	window = params.hFocusWindow;

	originalWindowProcess = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
	);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
	{
		return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
	};

	ImGuiIO& io = ImGui::GetIO();

	io.LogFilename = nullptr;
	io.IniFilename = nullptr;
	ImFontConfig fontConfig;
	fontConfig.FontDataOwnedByAtlas = false;

	//fonts::quicksand = io.Fonts->AddFontFromMemoryTTF(segoe, sizeof(segoe), 17.f, &fontConfig);
	//fonts::titleFont = io.Fonts->AddFontFromMemoryTTF(segoe, sizeof(segoe), 17.f, &fontConfig);
	//fonts::titleFont->DisplayOffset = {0,-5};

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;

	const ImColor bgColor = ColorFromBytes(11, 11, 11);
	const ImColor lightBgColor = ColorFromBytes(82, 82, 85);
	const ImColor veryLightBgColor = ColorFromBytes(90, 90, 95);

	const ImColor panelColor = ColorFromBytes(51, 51, 55);
	const ImColor panelHoverColor = ColorFromBytes(29, 151, 236);
	const ImColor panelActiveColor = ColorFromBytes(0, 224, 255);

	const ImColor textColor = ColorFromBytes(255, 255, 255);
	const ImColor textDisabledColor = ColorFromBytes(151, 151, 151);
	const ImColor borderColor = ColorFromBytes(78, 78, 78);


	style.WindowTitleAlign = { 0.5f,0.5f };


	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);

	setup = true;
}

void gui::Destroy() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	SetWindowLongPtr(
		window,
		GWLP_WNDPROC,
		reinterpret_cast<LONG_PTR>(originalWindowProcess)
	);

	DestroyDirectX();
}



void gui::Render() noexcept
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowStyleEditor();

	float sz = ImGui::GetTextLineHeight() + 4;

	if (globals::watermark)
		RenderWatermark();

	if (gui::open)
	{
		ImGui::Begin("astralproject.xyz", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{

			ImGui::SetWindowSize(ImVec2(670, 400));
			auto p = ImGui::GetWindowPos();
			auto __draw = ImGui::GetWindowDrawList();
			static int tab = 0;

			__draw->AddRectFilled(p + ImVec2(0, 30), p + ImVec2(150, 395), ImColor(31, 31, 41, 255));

			ImGui::SetCursorPos(ImVec2(0, 30));;
			ImGui::BeginGroup();
			{
				if (cGui.tab("aim", tab == 0, ImVec2(150, 25)))
					tab = 0;
				if (cGui.tab("visuals", tab == 1, ImVec2(150, 25)))
					tab = 1;
				if (cGui.tab("movement", tab == 2, ImVec2(150, 25)))
					tab = 2;
				if (cGui.tab("miscellaneous", tab == 3, ImVec2(150, 25)))
					tab = 3;
				if (cGui.tab("skins", tab == 4, ImVec2(150, 25)))
					tab = 4;
				if (cGui.tab("configuraion", tab == 5, ImVec2(150, 25)))
					tab = 5;
				ImGui::Text("     welcome");
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.03f, 0.43f, 0.89f, 1.00f), "0x7n");
			}
			ImGui::EndGroup();


			ImGui::SetCursorPos(ImVec2(156, 30));
			ImGui::BeginGroup();
			{
				switch (tab)
				{
				case 0:
					ImGui::BeginChild("aimbot", ImVec2(250, 365));
					{
						ImGui::BeginGroup();
						ImGui::Checkbox("enabled", &globals::aimbot);
						ImGui::Checkbox("silent aim", &globals::silentAim);
						ImGui::SliderFloat("aim smoothing", &globals::aimSmoothing, 0.f, 1.f);
						ImGui::SliderFloat("aimbot fov", &globals::aimbotFov, 1.f, 179.f);
						ImGui::EndGroup();
					}

					ImGui::EndChild();

					ImGui::SameLine(0, 8);

					ImGui::BeginGroup();
					{
						;
						ImGui::BeginChild("filters", ImVec2(250, 90));
						{

							ImGui::BeginGroup();
							ImGui::Button("button", ImVec2(125, 25));
							ImGui::EndGroup();
						}
						ImGui::EndChild();


						ImGui::BeginChild("weapons", ImVec2(250, 270));
						{

							ImGui::BeginGroup();

							ImGui::EndGroup();
						}
						ImGui::EndChild();
					}
					ImGui::EndGroup();

					break;
				case 1:
					ImGui::BeginChild("chams", ImVec2(250, 365));
					{


						ImGui::BeginGroup();
						ImGui::Checkbox("enabled", &globals::showChams);
						ImGui::Checkbox("visible only", &globals::visibleOnly);
						ImGui::ColorEdit3("cham visible color", globals::chamsVisible, ImGuiColorEditFlags_NoInputs);
						ImGui::ColorEdit3("cham hidden color", globals::chamsHidden, ImGuiColorEditFlags_NoInputs);
						ImGui::Combo("cham material", &globals::selectedMaterial, globals::materials, 17);

						ImGui::EndGroup();
					}

					ImGui::EndChild();

					ImGui::SameLine(0, 8);

					ImGui::BeginGroup();
					{

						ImGui::BeginChild("esp", ImVec2(250, 90));
						{

							ImGui::BeginGroup();

							ImGui::Checkbox("enabled", &globals::esp);
							ImGui::Checkbox("healthbar", &globals::espHealthBar);

							ImGui::EndGroup();
						}
						ImGui::EndChild();

						ImGui::BeginChild("glow", ImVec2(250, 270));
						{

							ImGui::BeginGroup();

							ImGui::Checkbox("glow", &globals::glow);
							ImGui::Checkbox("teammate glow", &globals::teamGlow);
							ImGui::ColorEdit3("enemy glow color", globals::enemyGlowCol, ImGuiColorEditFlags_NoInputs);
							ImGui::ColorEdit3("teammate glow color", globals::teammateGlowCol, ImGuiColorEditFlags_NoInputs);

							ImGui::EndGroup();
						}
						ImGui::EndChild();


					}
					ImGui::EndGroup();
					break;
				case 2:
					ImGui::BeginGroup();
					{

						ImGui::BeginChild("movement", ImVec2(250, 270));
						{

							ImGui::BeginGroup();

							ImGui::Checkbox("bunnyhop", &globals::bhop);
							ImGui::Checkbox("edgebug", &globals::edgebug);

							ImGui::EndGroup();
						}
						ImGui::EndChild();
					}
					ImGui::EndGroup();
					break;
				case 3:
					ImGui::BeginGroup();
					{

						ImGui::BeginChild("miscellaneous", ImVec2(250, 270));
						{
							ImGui::BeginGroup();
							ImGui::SliderInt("camera fov", &globals::playerFov, 10, 180);
							ImGui::Checkbox("radar", &globals::radar);
							ImGui::Checkbox("reveal ranks (not working)", &globals::revealRanks);
							ImGui::Checkbox("watermark", &globals::watermark);
							ImGui::EndGroup();
						}
						ImGui::EndChild();
					}
					ImGui::EndGroup();
					break;
				case 4:

					break;
				case 5:

					break;
				case 6:

					break;
				}
			}
			ImGui::EndGroup();
		}
		ImGui::End();
	}

	//ImGui::ShowStyleEditor();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}


void gui::RenderWatermark() noexcept
{
	auto end = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(end);

	ImGui::SetNextWindowPos({ 10,10 });
	ImGui::Begin("Another Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
	ImGui::Text("astralproject.xyz | %s", std::ctime(&time)); 
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.03f, 0.43f, 0.89f, 1.00f), "Project");
	ImGui::End();
}


LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wideParam, LPARAM longParam)
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		gui::open = !gui::open;

	// pass messages to imgui
	if (gui::open && ImGui_ImplWin32_WndProcHandler(window, message, wideParam, longParam))
		return true;

	return CallWindowProc(gui::originalWindowProcess,
		window,
		message,
		wideParam,
		longParam
	);
}