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

/*
Thread for render GUI with Dear ImGui
GUI can show plots and change TX/RX/SDR settings int real time
*/

struct tx_cfg {
  int bitrate;
  int mod_order; // 2-BPSK, 4-QPSK, 16-QAM16
  int sps;       // samples per symbol
  int IR_type;   // 0-Rectangle, 1-Raised-Cosine
};

struct rx_cfg {
  int mod_order; // 2-BPSK, 4-QPSK, 16-QAM16
  int sps;       // samples per symbol
  int IR_type;   // 0-Rectangle, 1-Raised-Cosine

  float gardner_BnTs;
  float gardner_Kp;

  float costas_Kp;
  float costas_Ki;
};

void run_gui(tx_cfg &tx_config, rx_cfg &rx_config) {
  /*#################################################### INIT
   * ###############################################################*/
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  SDL_Window *window = SDL_CreateWindow(
      "Backend start", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024,
      768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);

  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Включить Docking

  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 330");
  /*##########################################################################################################################*/

  int modulation_order = 2;
  int SPS = 16;
  int TX_IR_type = 0;
  int bitrate = 0;
  static float data[100] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  float left_width = 450.0f;

  /*################################################# Main
   * ####################################################################*/
  /*Start rendering*/
  bool running = true;
  while (running) {
    /*handler for events (click on exit button)*/
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    /*start create frame*/
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);

    /*create main window*/
    if (ImGui::Begin("Simulator")) {
      /*create TabBar (line with button)*/
      if (ImGui::BeginTabBar("MyTabBar")) {
        /*Transmitter Tab*/
        if (ImGui::BeginTabItem("Transmitter")) {
          /*
            Separate main window on 2 part: left part - settings, right - plots.
            Each subwindow - child (use BeginChild for create)
          */

          /*left child*/
          {
            if (ImGui::BeginChild("TX", ImVec2(left_width, 0), true)) {
              /*title*/
              ImGui::Text("Settings");
              /*bits generator settings*/
              ImGui::SeparatorText("Bits");
              { ImGui::SliderInt("Bitrate", &bitrate, 1, 100); }

              /*modulator settings*/
              ImGui::SeparatorText("Modulator");
              {
                ImGui::RadioButton("BPSK", &modulation_order, 2);
                ImGui::RadioButton("QPSK", &modulation_order, 4);
                ImGui::RadioButton("QAM16", &modulation_order, 16);
              }

              /*Upsampler settings*/
              ImGui::SeparatorText("Upsampler");
              { ImGui::SliderInt("Samples per symbol", &SPS, 1, 100); }

              /*psf settings*/
              ImGui::SeparatorText("Pulse Shaping filter");
              {
                ImGui::RadioButton("Rectangle", &TX_IR_type, 0);
                ImGui::RadioButton("Rised-Cosine", &TX_IR_type, 1);
              }
            }

            ImGui::EndChild();
          }

          /*Paste this between childes (vertical separetor)*/
          ImGui::SameLine();

          /*right child*/
          {
            if (ImGui::BeginChild("Plots", ImVec2(0, 0), true)) {
              /*GetContentRegionAvail() return params of current window*/
              ImVec2 win_params = ImGui::GetContentRegionAvail();
              /*create plot with ImPlot*/
              if (ImPlot::BeginPlot("Signal1", ImVec2(-1, win_params.y / 2))) {
                ImPlot::SetupAxes("Time", "Amplitude");
                ImPlot::PlotLine("Signal", data, 11);
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("Spectrum", ImVec2(-1, win_params.y / 2))) {
                ImPlot::SetupAxes("Frequency, Hz", "Amplitude");
                ImPlot::PlotLine("Spectrum", data, 11);
                ImPlot::EndPlot();
              }

              ImGui::EndChild();
            }
          }

          ImGui::EndTabItem();
        }

        /*Receiver Tab*/
        if (ImGui::BeginTabItem("Receiver")) {
          /*
            Separate main window on 2 part: left part - settings, right - plots.
            Each subwindow - child (use BeginChild for create)
          */

          /*left child*/
          {
            if (ImGui::BeginChild("RX", ImVec2(left_width, 0), true)) {
              /*title*/
              ImGui::Text("Settings");

              /*modulator settings*/
              ImGui::SeparatorText("Modulator");
              {
                ImGui::RadioButton("BPSK", &modulation_order, 2);
                ImGui::RadioButton("QPSK", &modulation_order, 4);
                ImGui::RadioButton("QAM16", &modulation_order, 16);
              }

              /*Upsampler settings*/
              ImGui::SeparatorText("Downsampler");
              { ImGui::SliderInt("Samples per symbol", &SPS, 1, 100); }

              /*psf settings*/
              ImGui::SeparatorText("Pulse Shaping filter");
              {
                ImGui::RadioButton("Rectangle", &TX_IR_type, 0);
                ImGui::RadioButton("Rised-Cosine", &TX_IR_type, 1);
              }

              /*gardner settings (symbol sync)*/
              ImGui::SeparatorText("Gardner (symbol sync)");
              {
                ImGui::SliderFloat("BnTs", &rx_config.gardner_BnTs, 0, 10);
                ImGui::SliderFloat("gKp", &rx_config.gardner_Kp, 0, 10);
              }

              /*costas settings (freq sync)*/
              ImGui::SeparatorText("Costas (frequency sync)");
              {
                ImGui::SliderFloat("cKp", &rx_config.costas_Kp, 0, 10);
                ImGui::SliderFloat("Ki", &rx_config.costas_Ki, 0, 10);
              }
            }

            ImGui::EndChild();
          }

          /*Paste this between childes (vertical separetor)*/
          ImGui::SameLine();

          /*right child*/
          {
            if (ImGui::BeginChild("Plots", ImVec2(0, 0), true)) {
              /*GetContentRegionAvail() return params of current window*/
              ImVec2 win_params = ImGui::GetContentRegionAvail();
              /*create plot with ImPlot*/
              if (ImPlot::BeginPlot("Signal1", ImVec2(-1, win_params.y / 3))) {
                ImPlot::SetupAxes("Time", "Amplitude");
                ImPlot::PlotLine("Signal", data, 11);
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("Spectrum", ImVec2(-1, win_params.y / 3))) {
                ImPlot::SetupAxes("Frequency, Hz", "Amplitude");
                ImPlot::PlotLine("Spectrum", data, 11);
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("Bit Error Rate",
                                    ImVec2(-1, win_params.y / 3))) {
                ImPlot::SetupAxes("Time", "BER");
                ImPlot::PlotLine("BER", data, 11);
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("Some", ImVec2(-1, win_params.y / 3))) {
                ImPlot::SetupAxes("Time", "BER");
                ImPlot::PlotLine("BER", data, 11);
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("Some2", ImVec2(-1, win_params.y / 3))) {
                ImPlot::SetupAxes("Time", "BER");
                ImPlot::PlotLine("BER", data, 11);
                ImPlot::EndPlot();
              }

              ImGui::EndChild();
            }
          }

          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }

      ImGui::End();
    }

    ImGui::Render();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }

  /*#################################################################################################################*/

  /*########################################## Free memory
   * ##########################################################*/
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  /*#################################################################################################################*/
}

int main(int argc, char *argv[]) {

  // /*check arguments*/
  // if (argc != 4) {
  //   printf("Programm wait 3 argument:\n \
  //     1.char* pluto usb_uri (example: )\n \
  //     2.int rx or tx (0-rx, 1-tx)\n \
  //     3.int time to work (in seconds)");

  //   return 1;
  // }

  tx_cfg tx_config{
      10, // bitrate
      2,  // modulation order
      10, // sps
      0   // pulse shaping filter IR
  };

  rx_cfg rx_config{2,  // modulation order
                   10, // sps
                   0,  // pulse shaping filter IR
                   1,  1, 1, 1};

  std::thread gui_thread(run_gui, std::ref(tx_config), std::ref(rx_config));
  gui_thread.join();

  // /*tx or rx mode*/
  // int mode = std::stoi(argv[2]);

  // int time_to_work = std::stoi(argv[3]);

  // /*create config for SDR*/
  // sdr_config_t config;
  // config.usb_uri = argv[1];
  // config.buff_size = 1920;
  // config.rx_carrier_freq = 793e6;
  // config.tx_carrier_freq = 793e6;
  // config.rx_sample_rate = 1e6;
  // config.tx_sample_rate = 1e6;
  // config.rx_gain = 35.0;
  // config.tx_gain = -15.0;

  // std::cout << config.usb_uri << std::endl;

  // /*files for tx/rx samples*/
  // char *rxdata = "../pcm/rxdata.pcm";
  // char *txdata = "../pcm/txdata.pcm";

  // /*setup pluto*/
  // struct SoapySDRDevice *sdr = setup_pluto_sdr(&config);
  // struct SoapySDRStream *rxStream = setup_stream(sdr, &config, 1);
  // struct SoapySDRStream *txStream = setup_stream(sdr, &config, 0);

  // /*create buffers*/
  // int16_t tx_buffer[2 * config.buff_size];
  // int16_t rx_buffer[2 * config.buff_size];

  // /*if we want only rx*/
  // if (mode == 0) {
  //   start_rx(sdr, rxStream, rx_buffer, config.buff_size, config.buff_size,
  //            rxdata, time_to_work);
  // } else if (mode == 1) {
  //   std::vector<std::complex<int16_t>> complex_samples =
  //       read_pcm(std::string("../../pcm/tx_samples.pcm"));

  //   int uncomplex_samples_size = complex_samples.size() * 2;
  //   std::vector<int16_t> uncomplex_samples;
  //   uncomplex_samples.reserve(uncomplex_samples_size);

  //   for (int i = 0; i < complex_samples.size(); ++i) {
  //     uncomplex_samples.push_back(complex_samples[i].real());
  //     uncomplex_samples.push_back(complex_samples[i].imag());
  //   }

  //   start_tx(sdr, txStream, rxStream, rx_buffer, uncomplex_samples.data(),
  //            complex_samples.size(), config.buff_size, txdata, time_to_work);
  // } else {
  //   printf("Invalid mode. Enter mode=0 or mode=1");
  // }

  // close_pluto_sdr(sdr, rxStream, txStream);

  return EXIT_SUCCESS;
}