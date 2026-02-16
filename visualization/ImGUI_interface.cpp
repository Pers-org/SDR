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

#include "../includes/ImGUI_interface.h"
#include "../includes/general/subfuncs.hpp"
#include "../includes/pluto_lib.hpp"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"

template <typename T> ImPlotPoint get_value(int idx, void *user_data) {
  auto *vec = static_cast<std::vector<T> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx]);
}

template <typename T> ImPlotPoint get_I(int idx, void *user_data) {
  auto *vec = static_cast<std::vector<std::complex<T>> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx].real());
}

template <typename T> ImPlotPoint get_Q(int idx, void *user_data) {
  auto *vec = static_cast<std::vector<std::complex<T>> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx].imag());
}

ImPlotPoint get_points(int idx, void *data) {
  auto *vec = static_cast<std::vector<std::complex<double>> *>(data);

  const auto &s = (*vec)[idx];

  return ImPlotPoint(s.real(), s.imag());
}

void run_gui(tx_cfg &tx_config, rx_cfg &rx_config, sdr_config_t &sdr_config) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

  SDL_Window *window = SDL_CreateWindow(
      "Backend start", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024,
      768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  SDL_GLContext gl_context = SDL_GL_CreateContext(window);

  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 330");

  float left_width = 450.0f;
  bool running = true;

  float mhz_rx = 800;
  float mhz_tx = 800;
  float samplerate_rx = 1;
  float samplerate_tx = 1;

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        tx_config.run = false;
        rx_config.run = false;
        running = false;
      }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);

    if (ImGui::Begin("Simulator")) {
      if (ImGui::BeginTabBar("MyTabBar")) {
        if (ImGui::BeginTabItem("Transmitter")) {
          if (ImGui::BeginChild("TX", ImVec2(left_width, 0), true)) {
            ImGui::Text("Settings");

            // ImGui::SeparatorText("Bits");
            // ImGui::SliderInt("Bitrate", &tx_config.bitrate, 1, 100);

            ImGui::SeparatorText("Modulator");
            ImGui::RadioButton("BPSK", &tx_config.mod_order, 2);
            ImGui::RadioButton("QPSK", &tx_config.mod_order, 4);
            ImGui::RadioButton("QAM16", &tx_config.mod_order, 16);

            ImGui::SeparatorText("Upsampler");
            ImGui::SliderInt("Samples per symbol", &tx_config.sps, 2, 100);

            ImGui::SeparatorText("Pulse Shaping filter");
            ImGui::RadioButton("Rectangle", &tx_config.IR_type, 0);
            ImGui::RadioButton("Rised-Cosine", &tx_config.IR_type, 1);

            ImGui::EndChild();
          }

          ImGui::SameLine();

          if (ImGui::BeginChild("TX_Plots", ImVec2(0, 0), true)) {
            ImVec2 win_params = ImGui::GetContentRegionAvail();
            if (ImPlot::BeginPlot("Bits", ImVec2(-1, win_params.y / 2))) {
              ImPlot::SetupAxisLimits(ImAxis_X1, 0, tx_config.bits.size(),
                                      ImGuiCond_Always);

              ImPlot::SetupAxisLimits(ImAxis_Y1, -0.3, 1.5, ImGuiCond_Always);
              ImPlot::PlotLineG("Bits", get_value<int16_t>, &tx_config.bits,
                                tx_config.bits.size());
              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("I/Q constellation",
                                  ImVec2(-1, win_params.y / 2))) {
              ImPlot::SetupAxisLimits(ImAxis_X1, -5, 5, ImGuiCond_Always);
              ImPlot::SetupAxisLimits(ImAxis_Y1, -5, 5, ImGuiCond_Always);

              ImPlot::PlotScatterG("Symbols", get_points, &tx_config.symbols,
                                   tx_config.symbols.size());

              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("Samples", ImVec2(-1, win_params.y / 2))) {
              ImPlot::SetupAxisLimits(ImAxis_X1, 0,
                                      tx_config.tx_samples.size() / 2,
                                      ImGuiCond_Always);

              ImPlot::SetupAxisLimits(ImAxis_Y1, -32500, 32500,
                                      ImGuiCond_Always);
              ImPlot::PlotLineG("I component", get_I<int16_t>,
                                &tx_config.tx_samples,
                                tx_config.tx_samples.size() / 2);
              ImPlot::PlotLineG("Q component", get_Q<int16_t>,
                                &tx_config.tx_samples,
                                tx_config.tx_samples.size() / 2);
              ImPlot::EndPlot();
            }

            ImGui::EndChild();
          }

          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Receiver")) {
          if (ImGui::BeginChild("RX", ImVec2(left_width, 0), true)) {
            ImGui::Text("Settings");

            ImGui::SeparatorText("Modulator");
            ImGui::RadioButton("BPSK", &rx_config.mod_order, 2);
            ImGui::RadioButton("QPSK", &rx_config.mod_order, 4);
            ImGui::RadioButton("QAM16", &rx_config.mod_order, 16);

            ImGui::SeparatorText("Downsampler");
            ImGui::SliderInt("Samples per symbol", &rx_config.sps, 1, 100);

            ImGui::SeparatorText("Pulse Shaping filter");
            ImGui::RadioButton("Rectangle", &rx_config.IR_type, 0);
            ImGui::RadioButton("Rised-Cosine", &rx_config.IR_type, 1);

            ImGui::SeparatorText("Gardner (symbol sync)");
            ImGui::SliderFloat("BnTs", &rx_config.gardner_BnTs, 0, 10);
            ImGui::SliderFloat("gKp", &rx_config.gardner_Kp, 0, 10);

            ImGui::SeparatorText("Costas (frequency sync)");
            ImGui::SliderFloat("cKp", &rx_config.costas_Kp, 0, 10);
            ImGui::SliderFloat("Ki", &rx_config.costas_Ki, 0, 10);

            ImGui::EndChild();
          }

          ImGui::SameLine();

          if (ImGui::BeginChild("RX_Plots", ImVec2(0, 0), true)) {
            ImVec2 win_params = ImGui::GetContentRegionAvail();
            if (ImPlot::BeginPlot("I/Q samples",
                                  ImVec2(-1, win_params.y / 2))) {
              ImPlot::SetupAxes("Time", "Amplitude");
              ImPlot::PlotLineG("I component", get_I<int16_t>,
                                &rx_config.rx_samples,
                                rx_config.rx_samples.size() / 2);
              ImPlot::PlotLineG("Q component", get_Q<int16_t>,
                                &rx_config.rx_samples,
                                rx_config.rx_samples.size() / 2);
              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("Matched filter output",
                                  ImVec2(-1, win_params.y / 2))) {
              ImPlot::SetupAxes("Time", "Amplitude");
              ImPlot::PlotLineG("I component", get_I<double>,
                                &rx_config.mf_samples_out,
                                rx_config.mf_samples_out.size() / 2);
              ImPlot::PlotLineG("Q component", get_Q<double>,
                                &rx_config.mf_samples_out,
                                rx_config.mf_samples_out.size() / 2);
              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("I/Q constellation",
                                  ImVec2(-1, win_params.y / 2))) {
              ImPlot::SetupAxisLimits(ImAxis_X1, -5, 5, ImGuiCond_Always);
              ImPlot::SetupAxisLimits(ImAxis_Y1, -5, 5, ImGuiCond_Always);

              ImPlot::PlotScatterG("Symbols", get_points,
                                   &rx_config.raw_symbols,
                                   rx_config.raw_symbols.size());

              ImPlot::EndPlot();
            }

            ImGui::EndChild();
          }

          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("SDR")) {
          if (ImGui::BeginChild("SDR_Settings", ImVec2(left_width, 0), true)) {
            ImGui::Text("Settings");

            ImGui::SeparatorText("Frequency");

            if (ImGui::SliderFloat("Rx carrier, MHz", &mhz_rx, 1, 20000,
                                   "%.3f")) {
              sdr_config.rx_carrier_freq = mhz_rx * 1e6;
            }

            if (ImGui::SliderFloat("Tx carrier, MHz", &mhz_tx, 1, 20000,
                                   "%.3f")) {
              sdr_config.tx_carrier_freq = mhz_tx * 1e6;
            }

            ImGui::SeparatorText("Gain");

            ImGui::SliderFloat("Rx gain, db", &sdr_config.rx_gain, 0, 50,
                               "%.3f");
            ImGui::SliderFloat("Tx gain, db", &sdr_config.tx_gain, -15, 0,
                               "%.3f");

            ImGui::SeparatorText("Sample rate");
            if (ImGui::SliderFloat("Rx sample rate, Msamples", &samplerate_rx,
                                   0, 2)) {
              sdr_config.rx_sample_rate = samplerate_rx * 1e6;
            }

            if (ImGui::SliderFloat("Tx sample rate, Msamples", &samplerate_tx,
                                   0, 2)) {
              sdr_config.rx_sample_rate = samplerate_rx * 1e6;
            }

            ImGui::EndChild();
          }

          ImGui::SameLine();

          if (ImGui::BeginChild("SDR_Plots", ImVec2(0, 0), true)) {
            ImGui::EndChild();
          }

          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }

      ImGui::End();
    }

    ImGui::Render();
    glViewport(0, 0, 1024, 768);
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
