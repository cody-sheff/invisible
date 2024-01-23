#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "frame_data.h"
#include "thread_pool.h"

/*
This application, takehome, takes the absolute path to a directory of cam files as the sole input, analyzes the frame
votes within, and outputs the numer of frames that have a majority of present votes true, any present votes true, and
all votes true.
*/
int main(int argc, char *argv[]) {
  if (argc != 2)
    throw std::runtime_error(
        "Error: The frame processing system should take a single argument, which is the absolute path to the folder "
        "you wish to analyze data from.");
  std::filesystem::path folder = std::string(argv[1]);
  if (!std::filesystem::is_directory(folder)) throw std::runtime_error("Error: Requested folder does not exist!");

  ThreadPool thread_pool(std::thread::hardware_concurrency());
  FrameAnalyzer frame_analyzer;

  // Create an individual thread for each txt file, as per the requirements.
  // Only analyze .txt files with 'cam' in the path as an initial validation check.
  for (const auto &file : std::filesystem::directory_iterator(folder)) {
    if (file.path().extension() != ".txt" || file.path().filename().string().find("cam") == std::string::npos) continue;

    thread_pool.QueueJob([file, &frame_analyzer]() { frame_analyzer.ParseCameraFile(file); });
  }

  while (thread_pool.AreJobsEnqueued()) {
    sleep(1);  // Sleep here so we're not constantly doing this check.
  }

  frame_analyzer.PrintAnalysisResults();

  return 0;
}