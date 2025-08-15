#include "bifrost_input.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

namespace bifrost
{
	void InputHandler::PollEvents(GLFWwindow* window)
	{
		std::swap(current_state_, previous_state_);
        current_state_.clear();

        glfwPollEvents();

        for(auto& [key, action] : keybinds_)
        {
        	if (glfwGetKey(window, key) == GLFW_PRESS)
        		current_state_[action] = 1.0f;
        }

        for(auto& [button, action] : mouse_button_binds_)
        {
        	if (glfwGetMouseButton(window, button) == GLFW_PRESS)
        		current_state_[action] = 1.0f;
        }
	
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		ypos = (double)height - ypos;

		MouseAt = glm::vec2((float)xpos, (float)ypos);
		if (!mouse_pressed_ && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			mouse_pressed_ = true;
			MousePressedAt = glm::vec2((float)xpos, (float)ypos);
		}
		if (mouse_pressed_ && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{
			mouse_pressed_ = false;
			MouseReleasedAt = glm::vec2((float)xpos, (float)ypos);
		}

	}

	bool InputHandler::IsActionPressed(const std::string& action)
	{
		return current_state_.find(action) != current_state_.end();
	}

	bool InputHandler::IsActionJustPressed(const std::string& action)
	{
		if (current_state_.find(action) == current_state_.end())
			return false;
		else
			return previous_state_.find(action) == previous_state_.end();
	}

	bool InputHandler::IsActionJustReleased(const std::string& action)
	{
		if (previous_state_.find(action) == previous_state_.end())
			return false;
		else
			return current_state_.find(action) == current_state_.end();
	}

	float InputHandler::GetAxis(const std::string& action_left, const std::string& action_right)
	{
		float left = 0.0f;
		float right = 0.0f;

		if (current_state_.find(action_left) != current_state_.end())
			left = current_state_[action_left];

		if (current_state_.find(action_right) != current_state_.end())
			right = current_state_[action_right];

		return right - left;
	}

	glm::vec2 InputHandler::GetAxis(const std::string& action_left, const std::string& action_right, const std::string& action_down, const std::string& action_up)
	{
		float left = 0.0f;
		float right = 0.0f;
		float up = 0.0f;
		float down = 0.0f;

		if (current_state_.find(action_left) != current_state_.end())
			left = current_state_[action_left];

		if (current_state_.find(action_right) != current_state_.end())
			right = current_state_[action_right];

		if (current_state_.find(action_down) != current_state_.end())
			down = current_state_[action_down];

		if (current_state_.find(action_up) != current_state_.end())
			up = current_state_[action_up];

		glm::vec2 result = glm::vec2(right - left, up - down);
		if (glm::length(result) > 0.0f)
			result = normalize(result);

		return result;
	}

	void InputHandler::ClearBinds(const std::string& action)
	{
		std::erase_if(keybinds_, [action](const auto& item)
		{
			const auto& [_, value] = item;
			return value == action;
		});

		std::erase_if(mouse_button_binds_, [action](const auto& item)
		{
			const auto& [_, value] = item;
			return value == action;
		});
	}

	void InputHandler::AddKeyBind(unsigned int key, const std::string& action)
	{
		keybinds_[key] = action;
	}

	void InputHandler::AddMouseButtonBind(unsigned int button, const std::string& action)
	{
		mouse_button_binds_[button] = action;
	}
}
