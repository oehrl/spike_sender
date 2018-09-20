#include <chrono>
#include <fstream>
#include <iostream>

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
  const double speed = 10.0;

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

  const double t_max = spikes.back().time;
  double simulation_time = 0.0;
  auto current_iterator = spikes.begin();

  std::cout << "Start transmission!" << std::endl;
  using clock_t = std::chrono::steady_clock;
  const auto start_time = clock_t::now();
  do {
    const auto current_time = clock_t::now();
    const std::chrono::duration<double> elapsed_time =
        current_time - start_time;
    simulation_time = elapsed_time.count() * 1000.0 * speed;

    while (current_iterator != spikes.end() &&
           current_iterator->time <= simulation_time) {
      std::cout << "Spike: " << current_iterator->neuron_id << std::endl;
      sd.Record(*current_iterator);
      ++current_iterator;
    }

    std::cout << simulation_time << "/" << t_max << "ms" << std::endl;
  } while (simulation_time < t_max);
}