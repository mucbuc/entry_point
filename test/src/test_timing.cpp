#include <asserter/src/asserter.hpp>
#include <entry_point/entry_point.hpp>

#include <chrono>
#include <string>

// ---------------------------------------------------------------------------
// Minimal state
// ---------------------------------------------------------------------------

struct TestState {};

// ---------------------------------------------------------------------------
// Minimal base
// ---------------------------------------------------------------------------

struct Test_Base {
    Test_Base(std::string name) : m_name(std::move(name)) {}
    const std::string& name_ref() const { return m_name; }
private:
    std::string m_name;
};

// ---------------------------------------------------------------------------
// Timing module
// Runs for exactly target_frames frames, then cancels. The outer test
// measures wall-clock elapsed time and checks it is close to 1 second.
// ---------------------------------------------------------------------------

template <typename Base>
struct Timing_Module : public Base {

    static constexpr int target_frames = 60;
    static constexpr int tolerance     = 10; // frames

    template <typename... U>
    Timing_Module(int& frame_count, U&&... init)
        : Base(std::forward<U>(init)...)
        , m_frame_count(frame_count)
    {}

    Timing_Module(Timing_Module&&) = default;

    void init(auto done) {
        done([](){});
    }

    void run(auto& /*state*/, float /*ft*/, auto cancel) {
        ++m_frame_count;
        if (m_frame_count >= target_frames) {
            cancel();
        }
    }

    int& m_frame_count;
};

// ---------------------------------------------------------------------------
// Test
// ---------------------------------------------------------------------------

int main()
{
    using namespace std::chrono;

    TestState state;
    int frame_count = 0;

    const auto t0 = system_clock::now();

    entry_point::execute_main_loop(state, 60, std::make_tuple(
        Timing_Module<Test_Base>(frame_count, "timing-module")
    ));

    const auto t1 = system_clock::now();
    const duration<double> elapsed(t1 - t0);

    // 60 frames at 60 fps should take 1.0s; allow +/- 10 frames of tolerance
    constexpr double expected    = 1.0;
    constexpr double tolerance_s = Timing_Module<Test_Base>::tolerance / 60.0;

    ASSERT(elapsed.count() >= expected - tolerance_s);
    ASSERT(elapsed.count() <= expected + tolerance_s);

    return 0;
}
