// PlaybackController.hpp
#pragma once

class PlaybackController {
public:
    enum class State { Idle, Playing, Paused, Done };

    int   tick(float dt);        // returns the number of animation steps for this frame
    void  play();
    void  pause();
    void  reset();
    void  setSpeed(int speed);   // 1..8
    void  notifyDone();          // called by the renderer when animation is complete
    State state() const { return state_; }

private:
    State state_       { State::Idle };
    int   speed_       { 4 };
    float accumulator_ { 0.f };
};
