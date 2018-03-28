#define BOOST_TEST_MODULE worhp_test
#include <boost/lexical_cast.hpp>
#include <boost/test/included/unit_test.hpp>
#include <pagmo/algorithm.hpp>
#include <pagmo/io.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/ackley.hpp>
#include <pagmo/problems/cec2006.hpp>
#include <pagmo/problems/hock_schittkowsky_71.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/luksan_vlcek1.hpp>
#include <pagmo/problems/rastrigin.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/types.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo_plugins_nonfree/worhp.hpp>

#ifdef _MSC_VER
#define WORHP_LIB ".\\worhp_c.dll"
#elif defined __APPLE__
#define WORHP_LIB "./libworhp_c.dylib"
#elif defined __MINGW32__
#define WORHP_LIB ".\\libworhp_c.dll"
#else
#define WORHP_LIB "./libworhp_c.so"
#endif

using namespace pagmo;

/*-----------------------------------------------------------------------
 *
 * Minimise    f
 *
 * subject to      -0.5 <= x1 <=  INFTY
 *                   -2 <= x2 <=  INFTY
 *                    0 <= x3 <=  2
 *                   -2 <= x4 <=  2
 *                         g1 ==  1
 *               -INFTY <= g2 <= -1
 *                  2.5 <= g3 <=  5
 *
 * where         f (x1,x2,x3,x4) = x1^2 + 2 x2^2 - x3
 *               g1(x1,x2,x3,x4) = x1^2 + x3^2 + x1x3
 *               g2(x1,x2,x3,x4) = x3 - x4
 *               g3(x1,x2,x3,x4) = x2 + x4
 *
 * Optimal solution
 *                     x*  = (0, 0.5, 1, 2)
 *                   f(x*) = -0.5
 *                   g(x*) = (1, -1, 2.5)
 *
 *-----------------------------------------------------------------------*/
struct worhp_test_problem {
    vector_double fitness(const vector_double &x) const
    {
        vector_double retval(5, 0);
        retval[0] = x[0] * x[0] + 2 * x[1] * x[1] - x[2];
        retval[1] = x[0] * x[0] + x[2] * x[2] + x[0] * x[2];
        retval[2] = x[2] - x[3] + 1;
        retval[3] = x[1] + x[3] - 5;
        retval[4] = -(x[1] + x[3] - 2.5);
        return retval;
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{-0.5, -2, 0, -2}, {5, 5, 2, 2}};
    }
    vector_double::size_type get_nec() const
    {
        return 1;
    }
    vector_double::size_type get_nic() const
    {
        return 3;
    }
    sparsity_pattern gradient_sparsity() const
    {
        return {{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 2}, {2, 2}, {2, 3}, {3, 1}, {3, 3}, {4, 1}, {4, 3}};
    }
    vector_double gradient(const vector_double &x) const
    {
        return {2 * x[0], 4 * x[1], -1, 2 * x[0] + x[2], 2 * x[2] + x[0], 1, -1, 1, 1, -1, -1};
    }
    std::vector<sparsity_pattern> hessians_sparsity() const
    {
        return {
            {{0, 0}, {1, 1}}, {{0, 0}, {2, 0}, {2, 2}}, {}, {}, {},
        };
    }
    std::vector<vector_double> hessians(const vector_double &) const
    {
        return {{2, 4}, {2, 1, 2}, {}, {}, {}};
    }
};

BOOST_AUTO_TEST_CASE(construction)
{
    // We test construction of the worhp uda
    BOOST_CHECK_NO_THROW((worhp{false, "ANY_NAME_WILL_WORK"}));
    worhp uda{false, "ANY_NAME_WILL_WORK"};
    // We check the default values
    BOOST_CHECK_EQUAL(uda.get_log().size(), 0u);
    BOOST_CHECK(uda.get_extra_info().find("WORHP evolve was never successfully called") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(evolve)
{
    worhp uda{true, WORHP_LIB};
    // uda.set_verbosity(1u);
    problem p{worhp_test_problem{}};
    p.set_c_tol(1e-7);
    uda.evolve(population{p, 1u});
    // uda.evolve(population{hock_schittkowsky_71{}, 1u});
    // uda.evolve(population{luksan_vlcek1{10u}, 1u});
    // uda.evolve(population{rastrigin{10u}, 1u});
    // uda.evolve(population{rosenbrock{5u}, 1u});
}

BOOST_AUTO_TEST_CASE(verbosity)
{
    // We test the verbosity mechanism when the original worhp screen output is deactivated
    {
        worhp uda{false, WORHP_LIB};
        problem p{worhp_test_problem{}};
        BOOST_CHECK_NO_THROW(uda.set_verbosity(1u));
        BOOST_CHECK_EQUAL(uda.get_verbosity(), 1u);
        uda.evolve(population{p, 1u});
        BOOST_CHECK(uda.get_log().size() > 0);
    }
    // We test the verbosity mechanism when the original worhp screen output is active
    {
        worhp uda{true, WORHP_LIB};
        problem p{worhp_test_problem{}};
        BOOST_CHECK_THROW(uda.set_verbosity(1u), std::invalid_argument);
        uda.evolve(population{p, 1u});
        BOOST_CHECK_EQUAL(uda.get_log().size(), 0);
    }
}

BOOST_AUTO_TEST_CASE(parameters_setting)
{
    worhp uda{true, WORHP_LIB};
    problem p{worhp_test_problem{}};
    // bool
    BOOST_CHECK_NO_THROW(uda.set_bool_option("Valid", true));
    BOOST_CHECK_NO_THROW(uda.evolve(population{p, 1u}));
    BOOST_CHECK_NO_THROW(uda.set_bool_option("invalid_bool_option", true));
    BOOST_CHECK_THROW(uda.evolve(population{p, 1u}), std::invalid_argument);
    BOOST_CHECK(uda.get_bool_options()["Valid"] == true);
    uda.reset_bool_options();
    BOOST_CHECK(uda.get_bool_options().size() == 0);
    BOOST_CHECK_NO_THROW(uda.set_bool_options({{"Valid1", true}, {"Valid2", false}}));
    BOOST_CHECK(uda.get_bool_options().size() == 2);
    BOOST_CHECK_NO_THROW(uda.set_bool_options({{"invalid_bool_option", true}, {"Valid3", false}}));
    BOOST_CHECK(uda.get_bool_options().size() == 4);
    BOOST_CHECK_THROW(uda.evolve(population{p, 1u}), std::invalid_argument);
    BOOST_CHECK_NO_THROW(uda.reset_bool_options());
    BOOST_CHECK(uda.get_bool_options().size() == 0);
    // numeric
    BOOST_CHECK_NO_THROW(uda.set_numeric_option("Valid", 13.2));
    BOOST_CHECK_NO_THROW(uda.evolve(population{p, 1u}));
    BOOST_CHECK_NO_THROW(uda.set_numeric_option("invalid_numeric_option", 1));
    BOOST_CHECK_THROW(uda.evolve(population{p, 1u}), std::invalid_argument);
    BOOST_CHECK(uda.get_numeric_options()["Valid"] == 13.2);
    uda.reset_numeric_options();
    BOOST_CHECK(uda.get_numeric_options().size() == 0);
    BOOST_CHECK_NO_THROW(uda.set_numeric_options({{"Valid1", 1}, {"Valid2", 2}}));
    BOOST_CHECK(uda.get_numeric_options().size() == 2);
    BOOST_CHECK_NO_THROW(uda.set_numeric_options({{"invalid_numeric_option", 1}, {"Valid3", 2}}));
    BOOST_CHECK(uda.get_numeric_options().size() == 4);
    BOOST_CHECK_THROW(uda.evolve(population{p, 1u}), std::invalid_argument);
    BOOST_CHECK_NO_THROW(uda.reset_numeric_options());
    BOOST_CHECK(uda.get_numeric_options().size() == 0);
    // integer
    BOOST_CHECK_NO_THROW(uda.set_integer_option("Valid", 13));
    BOOST_CHECK_NO_THROW(uda.evolve(population{p, 1u}));
    BOOST_CHECK_NO_THROW(uda.set_integer_option("invalid_integer_option", 132));
    BOOST_CHECK_THROW(uda.evolve(population{p, 1u}), std::invalid_argument);
    BOOST_CHECK(uda.get_integer_options()["Valid"] == 13);
    uda.reset_integer_options();
    BOOST_CHECK(uda.get_integer_options().size() == 0);
    BOOST_CHECK_NO_THROW(uda.set_integer_options({{"Valid1", 1}, {"Valid2", 2}}));
    BOOST_CHECK(uda.get_integer_options().size() == 2);
    BOOST_CHECK_NO_THROW(uda.set_integer_options({{"invalid_integer_option", 1}, {"Valid3", 2}}));
    BOOST_CHECK(uda.get_integer_options().size() == 4);
    BOOST_CHECK_THROW(uda.evolve(population{p, 1u}), std::invalid_argument);
    BOOST_CHECK_NO_THROW(uda.reset_integer_options());
    BOOST_CHECK(uda.get_integer_options().size() == 0);
}