// PlaybackController.cpp
#include "PlaybackController.hpp"
#include <cmath>
#include <algorithm>

// steps/sec = 2^(speed-1) * 20  →  speed 1: 20/s, speed 8: 2560/s
int PlaybackController::tick(float dt)
{
    if (state_ != State::Playing) return 0;
    float rate = std::pow(2.f, static_cast<float>(speed_ - 1)) * 20.f;
    accumulator_ += rate * dt;
    int steps = static_cast<int>(accumulator_);
    accumulator_ -= static_cast<float>(steps);
    return steps;
}

void PlaybackController::play()
{
    if (state_ == State::Idle || state_ == State::Paused)
        state_ = State::Playing;
}
void PlaybackController::pause()
{
    if (state_ == State::Playing) state_ = State::Paused;
}
void PlaybackController::reset()
{
    state_       = State::Idle;
    accumulator_ = 0.f;
}
void PlaybackController::setSpeed(int s)
{
    speed_ = std::clamp(s, 1, 8);
}
void PlaybackController::notifyDone()
{
    state_ = State::Done;
}