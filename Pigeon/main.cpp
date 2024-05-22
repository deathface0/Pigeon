#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PigeonClient/PigeonClientGUI.h"

template <typename K, typename V>
std::pair<K, V> removeLastElement(std::unordered_map<K, V>& m) {
    if (m.empty()) {
        throw std::out_of_range("Cannot remove element from an empty unordered_map");
    }

    auto lastIter = m.end();
    --lastIter; // Move to the last element (arbitrarily chosen)

    std::pair<K, V> lastElement = *lastIter; // Copy the last element
    m.erase(lastIter); // Erase the last element

    return lastElement; // Return the copied element
}

int main(int argc, char** argv)
{
    if (PigeonClientGUI::CreateImGuiWindow() != 0)
    {
        std::cerr << "Error to create SDL window " << std::endl;
    }

    PigeonClientGUI::SetUpImGui();

    GUIUtils::textureLoader("Images/");
    GUIUtils::fontLoader("Fonts/");

    PigeonClientGUIInfo::donwloadPath = File::getHomeDirectory();

    while (!shouldClose)
    {
        while (SDL_PollEvent(&PigeonClientGUI::currentEvent))
        {
            ImGui_ImplSDL2_ProcessEvent(&PigeonClientGUI::currentEvent);
            if (PigeonClientGUI::currentEvent.type == SDL_QUIT)
            {
                shouldClose = true;
            }
        }
        PigeonClientGUI::StartImGuiFrame();

        PigeonClientGUI::MainWindow();

        PigeonClientGUI::Render();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    PigeonClientGUI::Cleanup();
    return 0;
}