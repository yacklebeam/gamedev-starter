#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <map>
#include <string>

namespace bifrost
{
	class InputHandler
	{
	public:
		InputHandler(GLFWwindow* window) : window_{window} {};

		void PollEvents();

		bool IsActionPressed(const std::string& action);
		bool IsActionJustPressed(const std::string& action);
		bool IsActionJustReleased(const std::string& action);

		float GetAxis(const std::string& action_left, const std::string& action_right);
		glm::vec2 GetAxis(const std::string& action_left, const std::string& action_right, const std::string& action_down, const std::string& action_up);

		void AddKeyBind(unsigned int key, const std::string& action);
		void AddMouseButtonBind(unsigned int button, const std::string& action);

	private:
		std::unordered_map<std::string, float> current_state_{};
		std::unordered_map<std::string, float> previous_state_{};
		std::map<unsigned int, std::string> keybinds_{};
		std::map<unsigned int, std::string> mouse_button_binds_{};
		GLFWwindow* window_;
	};
}
