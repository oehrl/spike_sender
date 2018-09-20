cmake_minimum_required(VERSION 3.10)

project(spike_sender)

find_package(contra COMPONENTS core zmq REQUIRED)
find_package(nesci COMPONENTS layout producer consumer REQUIRED)

add_executable(spike_sender spike_sender.cpp)
target_link_libraries(spike_sender PRIVATE contra contra_zmq nesci_producer)

add_executable(dummy_receiver dummy_receiver.cpp)
target_link_libraries(dummy_receiver PRIVATE contra contra_zmq nesci_consumer)