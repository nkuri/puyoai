cmake_minimum_required(VERSION 2.8)

cpu_setup("sample")

cpu_add_executable(sample sample.cc cpu_interface.cc)
cpu_add_runner(run.sh)
cpu_target_link_common_libraries(sample)
