#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <functional>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>

namespace bifrost
{
	class InputHandler
	{
	public:
		void PollEvents(GLFWwindow* window);

		bool IsActionPressed(const std::string& action);
		bool IsActionJustPressed(const std::string& action);
		bool IsActionJustReleased(const std::string& action);

		float GetAxis(const std::string& action_left, const std::string& action_right);
		glm::vec2 GetAxis(const std::string& action_left, const std::string& action_right, const std::string& action_down, const std::string& action_up);

		void ClearBinds(const std::string& action);
		void AddKeyBind(unsigned int key, const std::string& action);
		void AddMouseButtonBind(unsigned int button, const std::string& action);

		void BindOnPressed(const std::string& action, std::function<void()> callback);
		void BindOnHeld(const std::string& action, std::function<void()> callback);
		void BindOnReleased(const std::string& action, std::function<void()> callback);

		glm::vec2 MouseAt{};
		glm::vec2 MousePressedAt{};
		glm::vec2 MouseReleasedAt{};
	private:
		std::unordered_map<std::string, float> current_state_{};
		std::unordered_map<std::string, float> previous_state_{};
		std::map<unsigned int, std::string> keybinds_{};
		std::map<unsigned int, std::string> mouse_button_binds_{};
		std::unordered_map<std::string, std::vector<std::function<void()>>> on_just_pressed_callbacks_{};
		std::unordered_map<std::string, std::vector<std::function<void()>>> on_held_callbacks_{};
		std::unordered_map<std::string, std::vector<std::function<void()>>> on_just_released_callbacks_{};
		bool mouse_pressed_;
	};
}
