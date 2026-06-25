#include <asserter/src/asserter.hpp>
#include <entry_point/entry_point.hpp>

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Minimal state
// ---------------------------------------------------------------------------

struct TestState {};

// ---------------------------------------------------------------------------
// Instrumented base
// ---------------------------------------------------------------------------

struct Test_Base {
    Test_Base(std::string name) : m_name(std::move(name)) {}
    const std::string& name_ref() const { return m_name; }
private:
    std::string m_name;
};

// ---------------------------------------------------------------------------
// Instrumented module
// Each instance records whether init, run, and release were called,
// and cancels the loop on the first run call.
// ---------------------------------------------------------------------------

struct Counters {
    int init_count    = 0;
    int run_count     = 0;
    int release_count = 0;
};

template <typename Base>
struct Test_Module : public Base {

    template <typename... U>
    Test_Module(Counters& counters, U&&... init)
        : Base(std::forward<U>(init)...)
        , m_counters(counters)
    {}

    Test_Module(Test_Module&&) = default;

    void init(auto done) {
        ++m_counters.init_count;
        done([this]() {
            ++m_counters.release_count;
        });
    }

    void run(auto& /*state*/, float /*ft*/, auto cancel) {
        ++m_counters.run_count;
        cancel();
    }

    Counters& m_counters;
};

// ---------------------------------------------------------------------------
// Test
// ---------------------------------------------------------------------------

int main()
{
    TestState state;

    Counters a, b, c;

    entry_point::execute_main_loop(state, 60, std::make_tuple(
        Test_Module<Test_Base>(a, "module-a"),
        Test_Module<Test_Base>(b, "module-b"),
        Test_Module<Test_Base>(c, "module-c")
    ));

    // Each module should have been initialised exactly once
    ASSERT(a.init_count    == 1);
    ASSERT(b.init_count    == 1);
    ASSERT(c.init_count    == 1);

    // Each module should have been run at least once
    ASSERT(a.run_count     >= 1);
    ASSERT(b.run_count     >= 1);
    ASSERT(c.run_count     >= 1);

    // Each module should have been released exactly once
    ASSERT(a.release_count == 1);
    ASSERT(b.release_count == 1);
    ASSERT(c.release_count == 1);

    return 0;
}
