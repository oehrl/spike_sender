#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include <contra/relay.hpp>
#include <contra/zmq/zeromq_transport.hpp>
#include <nesci/producer/spike_detector.hpp>

// struct Spike {
//   double time;
//   unsigned int neuron_id;
// };

int main(int argc, char** argv) {
  contra::Relay<contra::ZMQTransport> relay{contra::ZMQTransport::Type::SERVER,
                                            "tcp://*:5555", false};
  nesci::producer::SpikeDetector sd{"spike_detector"};

  // ms of simulation time per second
  const double speed = argc > 2 ? std::stod(argv[2]) : 10.0;

  // Send x spikes at ones
  const int spikes_per_package = argc > 3 ? std::stoi(argv[3]) : 5;

  std::fstream file(argv[1]);
  if (!file) {
    std::cerr << "Failed to open file: " << argv[1] << std::endl;
    return -1;
  }

  std::vector<nesci::producer::SpikeDetector::Datum> spikes;
  std::cout << "Reading spikes...";
  while (!file.eof()) {
    nesci::producer::SpikeDetector::Datum spike{0.0, 0};
    file >> spike.time;
    file >> spike.neuron_id;
    spikes.push_back(spike);
  }
  std::cout << " done!" << std::endl;
  std::cout << "Got " << spikes.size() << " spikes." << std::endl;
  std::cout << "Time range from " << spikes.front().time << " to "
            << spikes.back().time << "ms" << std::endl;
  std::cout << "The simulation speed is " << speed
            << "ms of simulation time per second in real world time"
            << std::endl;
  std::cout << "The simulation will take "
            << (spikes.back().time - spikes.front().time) / speed
            << " seconds to complete." << std::endl;
  std::cout << "Sending every " << spikes_per_package << " spikes!"
            << std::endl;

  const double t_0 = spikes.front().time;
  const double t_max = spikes.back().time;
  double simulation_time = spikes.front().time;
  auto current_iterator = spikes.begin();

  std::cout << "Press enter to start transmission!" << std::endl;
  std::cin.get();
  using clock_t = std::chrono::steady_clock;
  const auto start_time = clock_t::now();
  int recorded_spikes = 0;
  do {
    const auto current_time = clock_t::now();
    const std::chrono::duration<double> elapsed_time =
        current_time - start_time;
    simulation_time = t_0 + elapsed_time.count() * speed;

    while (current_iterator != spikes.end() &&
           current_iterator->time <= simulation_time) {
      std::cout << "{" << current_iterator->time << ","
                << current_iterator->neuron_id << "}" << std::endl;
      //   std::cout << "Spike: " << current_iterator->neuron_id << std::endl;
      sd.Record(*current_iterator);
      ++recorded_spikes;
      if (recorded_spikes >= spikes_per_package) {
        relay.Send(sd.node());
        sd.node().reset();
        recorded_spikes = 0;
        std::cout << "Sending!" << std::endl;
      }
      ++current_iterator;
    }

    // std::cout << simulation_time << "/" << t_max << "ms" << std::endl;
  } while (simulation_time < t_max);

  if (recorded_spikes > 0) {
    relay.Send(sd.node());
    std::cout << "Sending!" << std::endl;
  }
}