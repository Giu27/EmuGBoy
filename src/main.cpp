//Copyright (C) 2026  Giuseppe Caruso
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui_memory_editor.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <utils.h>
#include <gb.h>

static MemoryEditor mem_edit;

// Main code
int main(int, char**) {
    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }

    // Create window with SDL_Renderer graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow("GameBoy Emulator", (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);
    if (renderer == nullptr) {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        
    style.FontScaleDpi = main_scale;        

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Initial windows state
    bool show_registers = true;
    bool show_memory = false;
    bool single_stepping = true;
    bool step = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //setup GB Emu
    Gb gb;
    gb.loadRom("roms/test_roms/blargg/cpu_instrs/individual/08-misc instrs.gb");
    int cycles = 0;
    int cycles_this_frame = 0;
    int cycles_per_frame = 70224;

    // Main loop
    bool done = false;
    while (!done){
        cycles_this_frame = 0;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        if (!single_stepping || step) {
            while ((!single_stepping && (cycles_this_frame < cycles_per_frame)) || step) {
                cycles = gb.cpu.step();
                cycles_this_frame += cycles;
                step = false;
            }
            
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Show a simple window that we create ourselves. We use a Begin/End pair to create a named window. Always at (0, 0)
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        {
            ImGui::Begin("EmuGBoy", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

            //TODO: Move rom selector here

            ImGui::Text("Windows:");               
            ImGui::Checkbox("Registers", &show_registers);
            ImGui::Checkbox("Memory", &show_memory);
            ImGui::Text("Other tools:");
            ImGui::Checkbox("Pause Emulation", &single_stepping);
            ImGui::SameLine();
            if (ImGui::Button("step")) {
                step = true;
            } else {
                step = false;
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        {
            ImGui::Begin("Video output:");
            //TODO: Actually do something
            ImGui::End();
        }

        if (show_registers) {
            ImGui::Begin("Registers", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::BeginTable("RegistersTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Register");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();

                auto addWRegisterRow = [](const char* name, uint16_t value) { //Function to add 16 bits registers rows to a table
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", name);
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%04X", value);
                };

                auto addBRegisterRow = [](const char* name, uint8_t value) { //Function to add 8 bits registers rows to a table
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", name);
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%02X", value);
                };

                auto addFRegisterRow = [](const char* name, bool value) { //Function to add Flags rows to a table
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", name);
                    ImGui::TableNextColumn();
                    ImGui::Text("0b%d", value);
                };

                addWRegisterRow("PC", gb.cpu.registers.pc);
                addWRegisterRow("SP", gb.cpu.registers.sp);
                addBRegisterRow("IR", gb.cpu.registers.ir);
                addBRegisterRow("IE", gb.cpu.registers.ie);
                addBRegisterRow("A", gb.cpu.registers.a); 
                addWRegisterRow("BC", gb.cpu.registers.bc);
                addWRegisterRow("DE", gb.cpu.registers.de);
                addWRegisterRow("HL", gb.cpu.registers.hl);
                addFRegisterRow("z", getBit(gb.cpu.registers.f, 7));
                addFRegisterRow("n", getBit(gb.cpu.registers.f, 6));
                addFRegisterRow("h", getBit(gb.cpu.registers.f, 5));
                addFRegisterRow("c", getBit(gb.cpu.registers.f, 4));

                ImGui::EndTable();
            }

            ImGui::End();
        }

        if (show_memory) {
            mem_edit.DrawWindow("Memory", gb.memory, MEMORY_SIZE);
        }

        // Rendering
        ImGui::Render();
        SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Loop end
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}