#include "gui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx9.h"


#include "globals.h"

#include <stdexcept>
#include "../fonts/Quicksand.h"
#include "../util/elements.h"
#include "../../ext/imgui/imgui_internal.h"

namespace fonts {
	ImFont* quicksand;
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

	fonts::quicksand = io.Fonts->AddFontFromMemoryTTF(font_quicksand, sizeof(font_quicksand), 19.0f, &fontConfig);

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

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.11f, 0.59f, 0.93f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.88f, 1.00f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.32f, 0.32f, 0.33f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.88f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.59f, 0.93f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.88f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.11f, 0.59f, 0.93f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.11f, 0.59f, 0.93f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.11f, 0.59f, 0.93f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.88f, 1.00f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.32f, 0.32f, 0.33f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.11f, 0.59f, 0.93f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.03f, 0.38f, 0.43f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.03f, 0.24f, 0.27f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.88f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.11f, 0.59f, 0.93f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.88f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.11f, 0.59f, 0.93f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.88f, 1.00f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	style.WindowPadding = { 14,14 };
	style.FramePadding = { 3,3 };
	style.CellPadding = { 4,4 };
	style.ItemSpacing = { 10,4 };
	style.IndentSpacing = 21;
	style.ScrollbarSize = 14;
	style.WindowRounding = 4.0f;
	style.ChildRounding = 3.0f;
	style.FrameRounding = 5.0f;
	style.GrabRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.WindowTitleAlign = { 0.5f,0.5f };
	style.ChildBorderSize = 0.f;
	style.FrameBorderSize = 0.f;
	style.WindowBorderSize = 0.f;

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

	float sz = ImGui::GetTextLineHeight() + 4;

	ImGui::SetNextWindowSize({ 560, 400 });
	//ImGui::ShowStyleEditor();

	ImGui::Begin("Astral Project", &gui::open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	{
		auto draw = ImGui::GetWindowDrawList();

		auto pos = ImGui::GetWindowPos();
		auto size = ImGui::GetWindowSize();
		//draw->AddText(fonts::quicksand, 17.0f, ImVec2(pos.x + 49, pos.y + 18), ImColor(192, 203, 229), "Astral");

		
		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			//draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + 70), ImColor(24, 24, 24), 9.0f, ImDrawFlags_RoundCornersTop);
			/*
			
						if (ImGui::Button("Aim", ImVec2(75, 30)))
			{
				globals::tab = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("Visuals", ImVec2(75, 30)))
			{
				globals::tab = 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Movement", ImVec2(75, 30)))
			{
				globals::tab = 2;
			}
			
			*/

			ImGui::SetCursorPos({ 8,56 });
			ImGui::BeginGroup();
			{
				if (elements::tab("Aim", globals::tab == 0)) { globals::tab = 0; }
				if (elements::tab("Visuals", globals::tab == 1)) { globals::tab = 1; }
				if (elements::tab("Misc", globals::tab == 2)) { globals::tab = 2; }
			}
			ImGui::EndGroup();
		
		}
		ImGui::EndGroup();
		
		/*
		
				if (globals::tab == 0)
		{
			ImGui::Checkbox("Aimbot", &globals::aimbot);
			ImGui::Checkbox("Silent Aim", &globals::silentAim);
			ImGui::SliderFloat("Aim Smoothing", &globals::aimSmoothing, 0.f, 1.f);
			ImGui::SliderFloat("FOV", &globals::aimbotFov, 1.f, 179.f);
		}
		else if (globals::tab == 1)
		{
			ImVec4 test = { 0,0,0,255 };
			ImGui::Checkbox("Chams", &globals::showChams);
			ImGui::SameLine();
			ImGui::Hotkey(&globals::chamKey, ImVec2(sz * 1.5f, sz * 1.15f));
			ImGui::Checkbox("Visible Only", &globals::visibleOnly);
			ImGui::ColorEdit3("Cham Visible Color", globals::chamsVisible);
			ImGui::ColorEdit3("Cham Hidden Color", globals::chamsHidden);
			ImGui::ColorButton("Test", test);
			ImGui::ListBox("Materials", &globals::selectedMaterial, globals::materials, 13);
			ImGui::DragInt("FOV", &globals::playerFov, 1, 10, 180);
			ImGui::Checkbox("ESP", &globals::esp);
			ImGui::Checkbox("Healthbar", &globals::espHealthBar);
		}
		else if (globals::tab == 2)
		{
			ImGui::Checkbox("Bunnyhop", &globals::bhop);
			ImGui::Checkbox("Edgebug", &globals::edgebug);
			ImGui::Checkbox("Radar", &globals::radar);
			ImGui::Checkbox("Reveal Ranks (not working)", &globals::revealRanks);
		}

		*/

		float tabFontSize = 25.f;

		switch (globals::tab)
		{
			case 0:
				draw->AddText(fonts::quicksand, tabFontSize, ImVec2(pos.x + 72, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.4f), "Aim");

				ImGui::SetCursorPos({ 75, 86 });
				ImGui::BeginChild("Aim", ImVec2(240, 300));
				{
					ImGui::Checkbox("Aimbot", &globals::aimbot);
					ImGui::Checkbox("Silent Aim", &globals::silentAim);
					ImGui::SliderFloat("Aim Smoothing", &globals::aimSmoothing, 0.f, 1.f);
					ImGui::SliderFloat("FOV", &globals::aimbotFov, 1.f, 179.f);
				}
				ImGui::EndChild();
				break;
			case 1:

				draw->AddText(fonts::quicksand, tabFontSize, ImVec2(pos.x + 72, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.4f), "Visuals");

				ImGui::SetCursorPos({ 75, 86 });

				ImGui::BeginChild("Visuals", ImVec2(240,300));
				{
					ImGui::Checkbox("Chams", &globals::showChams);
					ImGui::SameLine();
					ImGui::Hotkey(&globals::chamKey, ImVec2(sz * 1.5f, sz * 1.15f));
					ImGui::Checkbox("Visible Only", &globals::visibleOnly);
					ImGui::ColorEdit3("Cham Visible Color", globals::chamsVisible);
					ImGui::ColorEdit3("Cham Hidden Color", globals::chamsHidden);
					ImGui::ListBox("Materials", &globals::selectedMaterial, globals::materials, 13);
					ImGui::DragInt("FOV", &globals::playerFov, 1, 10, 180);
					ImGui::Checkbox("ESP", &globals::esp);
					ImGui::Checkbox("Healthbar", &globals::espHealthBar);
				}
				ImGui::EndChild();

				ImGui::SetCursorPos({ 330, 86 });
				ImGui::BeginChild("Visuals", ImVec2(240, 175));
				{
					ImGui::Checkbox("ESP", &globals::esp);
					ImGui::Checkbox("Healthbar", &globals::espHealthBar);
				}
				ImGui::EndChild();

				break;
			case 2:
				draw->AddText(fonts::quicksand, tabFontSize, ImVec2(pos.x + 72, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.4f), "Misc");

				ImGui::SetCursorPos({ 75, 86 });

				ImGui::BeginChild("Misc", ImVec2(240, 300));
				{
					ImGui::Checkbox("Bunnyhop", &globals::bhop);
					ImGui::Checkbox("Edgebug", &globals::edgebug);
					ImGui::Checkbox("Radar", &globals::radar);
					ImGui::Checkbox("Reveal Ranks (not working)", &globals::revealRanks);
				}
				ImGui::EndChild();

				break;
			default:
				break;
		}

		/*
		
				ImGui::SetCursorPos({ 125, 19 });
		ImGui::BeginGroup(); {
			if (elements::tab("Aim", globals::tab == 0)) globals::tab == 0;
			ImGui::SameLine();
			if (elements::tab("Visuals", globals::tab == 1)) globals::tab == 1;
			ImGui::SameLine();
			if (elements::tab("Misc", globals::tab == 2)) globals::tab == 2;
		}
		ImGui::EndGroup();

		ImGui::Separator();

		switch (globals::tab)
		{
		case 0:
			draw->AddText(fonts::quicksand, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Aim");
			ImGui::SetCursorPos({ 25,85 });
			ImGui::BeginChild("##container", ImVec2(190, 275), false, ImGuiWindowFlags_NoScrollbar); {
				ImGui::Checkbox("Aimbot", &globals::aimbot);
				ImGui::Checkbox("Silent Aim", &globals::silentAim);
				ImGui::SliderFloat("Aim Smoothing", &globals::aimSmoothing, 0.f, 1.f);
				ImGui::SliderFloat("FOV", &globals::aimbotFov, 1.f, 179.f);
			}
			ImGui::EndChild();
			break;
		case 1:
			draw->AddText(fonts::quicksand, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Visuals");
			ImGui::SetCursorPos({ 25,85 });
			ImGui::BeginChild("##container", ImVec2(190, 275), false, ImGuiWindowFlags_NoScrollbar); {
				ImGui::Checkbox("Chams", &globals::showChams);
				ImGui::SameLine();
				ImGui::Hotkey(&globals::chamKey, ImVec2(sz * 1.5f, sz * 1.15f));
				ImGui::Checkbox("Visible Only", &globals::visibleOnly);
				ImGui::ColorEdit3("Cham Visible Color", globals::chamsVisible);
				ImGui::ColorEdit3("Cham Hidden Color", globals::chamsHidden);
				ImGui::ListBox("Materials", &globals::selectedMaterial, globals::materials, 13);
			}
			ImGui::EndChild();

			draw->AddText(fonts::quicksand, 14.0f, ImVec2(pos.x + 285, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "ESP");

			ImGui::SetCursorPos({ 285, 85 });
			ImGui::BeginChild("##container1", ImVec2(190, 275), false, ImGuiWindowFlags_NoScrollbar); {
				ImGui::Checkbox("ESP", &globals::esp);
				ImGui::Checkbox("ESP Healthbar", &globals::espHealthBar);
			}
			ImGui::EndChild();
			break;
		}
		
		*/
	}


	ImGui::End();
	/*
			if (ImGui::BeginTabBar("items"))
		{
			if (ImGui::BeginTabItem("Combat"))
			{
				ImGui::Checkbox("Aimbot", &globals::aimbot);
				ImGui::Checkbox("Silent Aim", &globals::silentAim);
				ImGui::SliderFloat("Aim Smoothing", &globals::aimSmoothing, 0.f, 1.f);
				ImGui::SliderFloat("FOV", &globals::fov, 1.f, 179.f);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Visuals"))
			{
				ImGui::Checkbox("Chams", &globals::showChams);
				ImGui::Checkbox("Visible Only", &globals::visibleOnly);
				ImGui::ColorEdit3("Cham Visible Color", globals::chamsVisible);
				ImGui::ColorEdit3("Cham Hidden Color", globals::chamsHidden);
				ImGui::ListBox("Materials", &globals::selectedMaterial, globals::materials, 13);

				ImGui::Checkbox("ESP", &globals::esp);
				ImGui::Checkbox("Healthbar", &globals::espHealthBar);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Movement"))
			{
				ImGui::Checkbox("Bunnyhop", &globals::bhop);
				ImGui::Checkbox("Reveal Ranks", &globals::revealRanks);
			}

			ImGui::EndTabBar();
		}


		ImGui::End();
	
	
	*/

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
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