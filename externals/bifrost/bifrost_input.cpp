#include "bifrost_input.h"

#include <GLFW/glfw3.h>
#include <string>

namespace bifrost
{
	void InputHandler::PollEvents()
	{
        glfwPollEvents();

        current_state_.clear();

        for(auto& [key, action] : keybinds_)
        {
        	if (glfwGetKey(window_, key) == GLFW_PRESS)
        		current_state_[action] = 1.0f;
        }
	}

	void InputHandler::Swap()
	{
		std::swap(current_state_, previous_state_);
	}

	bool InputHandler::IsActionPressed(const std::string& action)
	{
		return current_state_.find(action) == current_state_.end();
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

	void InputHandler::AddActionKeyBind(unsigned int key, const std::string& action)
	{
		keybinds_[key] = action;
	}
}
