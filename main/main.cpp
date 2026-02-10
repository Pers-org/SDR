/*
  This code work with Pluto SDR. He init and setup SDR, receive/transmit samples
*/
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <complex.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "../includes/general/subfuncs.hpp"
#include "../includes/pluto_lib/pluto_lib.hpp"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"

void run_gui() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  SDL_Window *window = SDL_CreateWindow(
      "Backend start", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024,
      768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);

  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Включить Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Включить Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Включить Docking

  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 330");

  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      std::cout << "Processing some event: " << event.type
                << " timestamp: " << event.motion.timestamp << std::endl;
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);

    {
      static int counter = 0;

      ImGui::Begin("Hello, world!");
      ImGui::Text("This is some useful text.");
      if (ImGui::Button("Button"))
        counter++;
      ImGui::Text("counter = %d", counter);
      ImGui::End();
    }

    ImGui::Render();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

int main(int argc, char *argv[]) {

  /*check arguments*/
  if (argc != 4) {
    printf("Programm wait 3 argument:\n \
      1.char* pluto usb_uri (example: )\n \
      2.int rx or tx (0-rx, 1-tx)\n \
      3.int time to work (in seconds)");

    return 1;
  }

  std::thread gui_thread(run_gui);
  gui_thread.join();

  /*tx or rx mode*/
  int mode = std::stoi(argv[2]);

  int time_to_work = std::stoi(argv[3]);

  /*create config for SDR*/
  sdr_config_t config;
  config.usb_uri = argv[1];
  config.buff_size = 1920;
  config.rx_carrier_freq = 793e6;
  config.tx_carrier_freq = 793e6;
  config.rx_sample_rate = 1e6;
  config.tx_sample_rate = 1e6;
  config.rx_gain = 35.0;
  config.tx_gain = -15.0;

  std::cout << config.usb_uri << std::endl;

  /*files for tx/rx samples*/
  char *rxdata = "../pcm/rxdata.pcm";
  char *txdata = "../pcm/txdata.pcm";

  /*setup pluto*/
  struct SoapySDRDevice *sdr = setup_pluto_sdr(&config);
  struct SoapySDRStream *rxStream = setup_stream(sdr, &config, 1);
  struct SoapySDRStream *txStream = setup_stream(sdr, &config, 0);

  /*create buffers*/
  int16_t tx_buffer[2 * config.buff_size];
  int16_t rx_buffer[2 * config.buff_size];

  /*if we want only rx*/
  if (mode == 0) {
    start_rx(sdr, rxStream, rx_buffer, config.buff_size, config.buff_size,
             rxdata, time_to_work);
  } else if (mode == 1) {
    std::vector<std::complex<int16_t>> complex_samples =
        read_pcm(std::string("../../pcm/tx_samples.pcm"));

    int uncomplex_samples_size = complex_samples.size() * 2;
    std::vector<int16_t> uncomplex_samples;
    uncomplex_samples.reserve(uncomplex_samples_size);

    for (int i = 0; i < complex_samples.size(); ++i) {
      uncomplex_samples.push_back(complex_samples[i].real());
      uncomplex_samples.push_back(complex_samples[i].imag());
    }

    start_tx(sdr, txStream, rxStream, rx_buffer, uncomplex_samples.data(),
             complex_samples.size(), config.buff_size, txdata, time_to_work);
  } else {
    printf("Invalid mode. Enter mode=0 or mode=1");
  }

  close_pluto_sdr(sdr, rxStream, txStream);

  return EXIT_SUCCESS;
}