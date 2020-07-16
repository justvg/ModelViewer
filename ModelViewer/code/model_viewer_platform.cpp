#include "model_viewer_platform_common.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// NOTE(georgy): These are for malloc() and memset()
#include <stdlib.h>
#include <string.h>

static void
GLFWKeyCallback(GLFWwindow* Window, int Key, int ScanCode, int Action, int Mods)
{
	game_input* Input = (game_input*)glfwGetWindowUserPointer(Window);

	if (Key == GLFW_KEY_W)
	{
		if (Action == GLFW_PRESS)
		{
			Input->W.EndedDown = true;
			++Input->W.HalfTransitionCount;
		}
		else if (Action == GLFW_RELEASE)
		{
			Input->W.EndedDown = false;
			++Input->W.HalfTransitionCount;
		}
	}
	if (Key == GLFW_KEY_A)
	{
		if (Action == GLFW_PRESS)
		{
			Input->A.EndedDown = true;
			++Input->A.HalfTransitionCount;
		}
		else if (Action == GLFW_RELEASE)
		{
			Input->A.EndedDown = false;
			++Input->A.HalfTransitionCount;
		}
	}
	if (Key == GLFW_KEY_D)
	{
		if (Action == GLFW_PRESS)
		{
			Input->D.EndedDown = true;
			++Input->D.HalfTransitionCount;
		}
		else if (Action == GLFW_RELEASE)
		{
			Input->D.EndedDown = false;
			++Input->D.HalfTransitionCount;
		}
	}
	if (Key == GLFW_KEY_S)
	{
		if (Action == GLFW_PRESS)
		{
			Input->S.EndedDown = true;
			++Input->S.HalfTransitionCount;
		}
		else if (Action == GLFW_RELEASE)
		{
			Input->S.EndedDown = false;
			++Input->S.HalfTransitionCount;
		}
	}
}

static void
GLFWMouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods)
{
	game_input* Input = (game_input*)glfwGetWindowUserPointer(Window);

	if (Button == GLFW_MOUSE_BUTTON_1)
	{
		if (Action == GLFW_PRESS)
		{
			Input->MouseLeft.EndedDown = true;
			++Input->MouseLeft.HalfTransitionCount;
		}
		else if (Action == GLFW_RELEASE)
		{
			Input->MouseLeft.EndedDown = false;
			++Input->MouseLeft.HalfTransitionCount;
		}
	}
	if (Button == GLFW_MOUSE_BUTTON_2)
	{
		if (Action == GLFW_PRESS)
		{
			Input->MouseRight.EndedDown = true;
			++Input->MouseRight.HalfTransitionCount;
		}
		else if (Action == GLFW_RELEASE)
		{
			Input->MouseRight.EndedDown = false;
			++Input->MouseRight.HalfTransitionCount;
		}
	}
}

static void
GLFWCursorPosCallback(GLFWwindow* Window, double XPos, double YPos)
{
	game_input* Input = (game_input*)glfwGetWindowUserPointer(Window);

	Input->MouseDeltaX = (float)((int)XPos - Input->MouseX);
	Input->MouseDeltaY = (float)((int)YPos - Input->MouseY);

	Input->MouseX = (int)XPos;
	Input->MouseY = (int)YPos;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* Window = glfwCreateWindow(900, 540, "AnimationViewer", 0, 0);
	glfwMakeContextCurrent(Window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(Window, GLFWKeyCallback);
	glfwSetMouseButtonCallback(Window, GLFWMouseButtonCallback);
	glfwSetCursorPosCallback(Window, GLFWCursorPosCallback);
	// glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	game_memory GameMemory = {};
	GameMemory.PermanentStorageSize = Megabytes(256);
	GameMemory.TemporaryStorageSize = Gigabytes(3);
	GameMemory.PermanentStorage = malloc(GameMemory.PermanentStorageSize + GameMemory.TemporaryStorageSize);
	GameMemory.TemporaryStorage = (uint8_t*)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;

	if (GameMemory.TemporaryStorage)
	{
		memset(GameMemory.PermanentStorage, 0, GameMemory.PermanentStorageSize + GameMemory.TemporaryStorageSize);

		game_input GameInput = {};
		GLFWmonitor* Monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* VidMode = glfwGetVideoMode(Monitor);
		int32_t GameUpdateHz = VidMode->refreshRate;
		float TargetSecondsPerFrame = 1.0f / GameUpdateHz;
		GameInput.dt = TargetSecondsPerFrame;
		glfwSetWindowUserPointer(Window, &GameInput);

		glewInit();

		glViewport(0, 0, 900, 540);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		// glEnable(GL_FRAMEBUFFER_SRGB);

		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

		while (!glfwWindowShouldClose(Window))
		{
			GameInput.MouseDeltaX = GameInput.MouseDeltaY = 0.0f;
			GameInput.MouseLeft.HalfTransitionCount = GameInput.MouseRight.HalfTransitionCount = 0;
			for (uint32_t ButtonIndex = 0;
				ButtonIndex < ArrayCount(GameInput.Buttons);
				ButtonIndex++)
			{
				GameInput.Buttons[ButtonIndex].HalfTransitionCount = 0;
			}

			glfwPollEvents();

			UpdateAndRender(&GameMemory, &GameInput, 900, 540);

			glfwSwapBuffers(Window);
		}
	}

	return(0);
}