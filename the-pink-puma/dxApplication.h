#pragma once

#include "windowApplication.h"
#include "dxDevice.h"
#include "DirectXMath.h"
#include "clock.h"
#include "diInstance.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"

#include <array>

namespace mini
{
	class DxApplication : public mini::WindowApplication
	{
	public:
		explicit DxApplication(HINSTANCE hInstance,
			int wndWidth = Window::m_defaultWindowWidth,
			int wndHeight = Window::m_defaultWindowHeight,
			std::wstring wndTitle = L"DirectX Window");

	protected:
		int MainLoop() override;

		const Clock& getClock() const { return m_clock; }
		virtual void Render();
		virtual void Update(const Clock& c) { }

		void UpdateBuffer(const dx_ptr<ID3D11Buffer>& buffer, const void* data, size_t count);
		template<typename T>
		void UpdateBuffer(const dx_ptr<ID3D11Buffer>& buffer, const T& data)
		{
			UpdateBuffer(buffer, &data, sizeof(T));
		}
		//***************** NEW *****************
		//Additional overload to copy data from a vector

		template<typename T>
		void UpdateBuffer(const dx_ptr<ID3D11Buffer>& buffer, const std::vector<T>& data)
		{
			UpdateBuffer(buffer, data.data(), data.size() * sizeof(T));
		}

		bool HandleCameraInput(double dt);

		//***************** NEW *****************

		//Resets pipeline back to rendering into program window
		void ResetRenderTarget();
		void RenderToDepthBuffer();

		void ClearDepthBuffer();
		void ClearDepthAndStencilBuffer();

		DxDevice m_device;

		DiInstance m_inputDevice;
		Mouse m_mouse;
		Keyboard m_keyboard;

		inline static constexpr float ROTATION_SPEED = 0.005f;
		inline static constexpr float MOVEMENT_SPEED = 1.0f;
		inline static constexpr float ROBOT_SPEED = 0.4f;

		FPSCamera m_camera;

	private:

		mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
		mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
		Viewport m_viewport;
		Clock m_clock;

	};
}