#include <chrono>
#include <fstream>
#include <iostream>

#include <contra/relay.hpp>
#include <contra/zmq/zeromq_transport.hpp>
#include <nesci/consumer/spike_detector.hpp>

int main(int argc, char** argv) {
  contra::Relay<contra::ZMQTransport> relay{contra::ZMQTransport::Type::CLIENT,
                                            "tcp://localhost:5555", false};
  nesci::consumer::SpikeDetector sd{"spike_detector"};

  unsigned int num_spikes = 0;
  while (true) {
    const auto nodes = relay.Receive();
    for (const auto node : nodes) {
      sd.SetNode(&node);

      const auto timesteps = sd.GetTimesteps();
      for (const auto timestep : timesteps) {
        const auto neuron_ids = sd.GetNeuronIds(timestep);
        for (const auto neuron_id : neuron_ids) {
          ++num_spikes;
          std::cout << "{" << timestep << "," << neuron_id << "}" << std::endl;
        }
      }
    }
    if (nodes.size() > 0) {
          std::cout << "Recevied " << num_spikes << " so far!" << std::endl;
    }
  }
}