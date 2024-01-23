#include <stdio.h>

#include <memory>
#include <mutex>
#include <vector>

// Constant for the maximum expected frames.
constexpr static size_t kMaxFrames = 600000;

enum FrameState { NOT_PRESENT, FALSE, TRUE };

class FrameAnalyzer {
 public:
  void ParseCameraFile(const std::filesystem::path file) {
    std::vector<FrameState> frame_state(kMaxFrames);

    std::ifstream infile(file.string());
    if (!infile.fail()) {
      std::string a, b;
      while (infile >> a >> b) {
        // Get the index of the frame for our vector.
        int index = stoi(a.substr(0, a.size() - 1));  // Leave off last character as it's a comma.
        if (index < 0 || index > kMaxFrames)
          throw std::runtime_error(
              "Error: Invalid frame! Frame numbers must be greater than or equal to 0 and less than kMaxFrames.");
        if (b == "true")
          frame_state[index] = FrameState::TRUE;
        else if (b == "false")
          frame_state[index] = FrameState::FALSE;
        else
          throw std::runtime_error("Error: Invalid frame vote! Frame votes may only be 'true' or 'false'.");
      }

      {
        std::scoped_lock lock(update_mutex_);
        this->UpdateFrameData(frame_state);
      }
      return;
    }

    throw std::runtime_error("Error: Unable to open file " + file.string());
  }

  void PrintAnalysisResults() {
    std::scoped_lock lock(update_mutex_);
    int all_true = 0;
    int majority_true = 0;
    int any_true = 0;

    for (int i = 0; i < kMaxFrames; i++) {
      if (frame_count_[i] == true_count_[i] && frame_count_[i] > 0) {
        all_true++;
        majority_true++;
        any_true++;
      } else if (true_count_[i] * 2 >= frame_count_[i] && frame_count_[i] > 0) {
        majority_true++;
        any_true++;
      } else if (true_count_[i] > 0 && frame_count_[i] > 0) {
        any_true++;
      }
    }

    std::cout << "Frames with majority of the present votes true: " << majority_true << std::endl;
    std::cout << "Frames with any of the present votes true: " << any_true << std::endl;
    std::cout << "Frames with all of the present votes true: " << all_true << std::endl;
  }

 private:
  // Takes in a frame state vector that contains the analysis results from a single camera and updates which frames have
  // been seen and which frames are valid.
  void UpdateFrameData(const std::vector<FrameState>& frame_state) {
    if (frame_state.size() != kMaxFrames) throw std::runtime_error("Error: Frame state data is an unexpected size!");
    for (size_t i = 0; i < frame_state.size(); i++) {
      if (frame_state[i] == FrameState::FALSE) {
        frame_count_[i]++;
      }
      if (frame_state[i] == FrameState::TRUE) {
        frame_count_[i]++;
        true_count_[i]++;
      }
    }
  }

  std::array<int, kMaxFrames> frame_count_{};
  std::array<int, kMaxFrames> true_count_{};
  std::mutex update_mutex_;  // To be locked when data is modified here.
};
