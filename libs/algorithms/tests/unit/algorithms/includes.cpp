//  Copyright (c) 2014-2015 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/parallel_set_operations.hpp>
#include <hpx/modules/testing.hpp>

#include <boost/next_prior.hpp>
#include <boost/utility.hpp>

#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////////
int seed = std::random_device{}();
std::mt19937 gen(seed);

template <typename ExPolicy, typename IteratorTag>
void test_includes1(ExPolicy policy, IteratorTag)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::uniform_int_distribution<> dis(0, c1.size() - 1);
    std::size_t start = dis(gen);
    std::uniform_int_distribution<> dist(0, c1.size() - start - 1);
    std::size_t end = start + dist(gen);

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);

    HPX_TEST_LTE(start, end);

    base_iterator start_it = boost::next(std::begin(c1), start);
    base_iterator end_it = boost::next(std::begin(c1), end);

    {
        bool result = hpx::parallel::includes(policy, iterator(std::begin(c1)),
            iterator(std::end(c1)), start_it, end_it);

        bool expected =
            std::includes(std::begin(c1), std::end(c1), start_it, end_it);

        // verify values
        HPX_TEST_EQ(result, expected);
    }

    {
        std::vector<std::size_t> c2;
        std::copy(start_it, end_it, std::back_inserter(c2));

        if (!c2.empty())
        {
            std::uniform_int_distribution<> dis(0, c2.size() - 1);
            ++c2[dis(gen)];    //-V104

            bool result =
                hpx::parallel::includes(policy, iterator(std::begin(c1)),
                    iterator(std::end(c1)), std::begin(c2), std::end(c2));

            bool expected = std::includes(
                std::begin(c1), std::end(c1), std::begin(c2), std::end(c2));

            // verify values
            HPX_TEST_EQ(result, expected);
        }
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_includes1_async(ExPolicy p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::uniform_int_distribution<> dis(0, c1.size() - 1);
    std::size_t start = dis(gen);
    std::uniform_int_distribution<> dist(0, c1.size() - start - 1);
    std::size_t end = start + dist(gen);

    HPX_TEST_LTE(start, end);

    base_iterator start_it = boost::next(std::begin(c1), start);
    base_iterator end_it = boost::next(std::begin(c1), end);

    {
        hpx::future<bool> result = hpx::parallel::includes(p,
            iterator(std::begin(c1)), iterator(std::end(c1)), start_it, end_it);
        result.wait();

        bool expected =
            std::includes(std::begin(c1), std::end(c1), start_it, end_it);

        // verify values
        HPX_TEST_EQ(result.get(), expected);
    }

    {
        std::vector<std::size_t> c2;
        std::copy(start_it, end_it, std::back_inserter(c2));

        if (!c2.empty())
        {
            std::uniform_int_distribution<> dis(0, c2.size() - 1);
            ++c2[dis(gen)];    //-V104

            hpx::future<bool> result =
                hpx::parallel::includes(p, iterator(std::begin(c1)),
                    iterator(std::end(c1)), std::begin(c2), std::end(c2));
            result.wait();

            bool expected = std::includes(
                std::begin(c1), std::end(c1), std::begin(c2), std::end(c2));

            // verify values
            HPX_TEST_EQ(result.get(), expected);
        }
    }
}

template <typename IteratorTag>
void test_includes1()
{
    using namespace hpx::parallel;

    test_includes1(execution::seq, IteratorTag());
    test_includes1(execution::par, IteratorTag());
    test_includes1(execution::par_unseq, IteratorTag());

    test_includes1_async(execution::seq(execution::task), IteratorTag());
    test_includes1_async(execution::par(execution::task), IteratorTag());
}

void includes_test1()
{
    test_includes1<std::random_access_iterator_tag>();
    test_includes1<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_includes2(ExPolicy policy, IteratorTag)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);

    std::uniform_int_distribution<> dis(0, c1.size() - 1);
    std::size_t start = dis(gen);
    std::uniform_int_distribution<> dist(0, c1.size() - start - 1);
    std::size_t end = start + dist(gen);

    HPX_TEST_LTE(start, end);

    base_iterator start_it = boost::next(std::begin(c1), start);
    base_iterator end_it = boost::next(std::begin(c1), end);

    {
        bool result = hpx::parallel::includes(policy, iterator(std::begin(c1)),
            iterator(std::end(c1)), start_it, end_it, std::less<std::size_t>());

        bool expected = std::includes(std::begin(c1), std::end(c1), start_it,
            end_it, std::less<std::size_t>());

        // verify values
        HPX_TEST_EQ(result, expected);
    }

    {
        std::vector<std::size_t> c2;
        std::copy(start_it, end_it, std::back_inserter(c2));

        if (!c2.empty())
        {
            std::uniform_int_distribution<> dis(0, c2.size() - 1);
            ++c2[dis(gen)];    //-V104

            bool result = hpx::parallel::includes(policy,
                iterator(std::begin(c1)), iterator(std::end(c1)),
                std::begin(c2), std::end(c2), std::less<std::size_t>());

            bool expected = std::includes(std::begin(c1), std::end(c1),
                std::begin(c2), std::end(c2), std::less<std::size_t>());

            // verify values
            HPX_TEST_EQ(result, expected);
        }
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_includes2_async(ExPolicy p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);

    std::uniform_int_distribution<> dis(0, c1.size() - 1);
    std::size_t start = dis(gen);
    std::uniform_int_distribution<> dist(0, c1.size() - start - 1);
    std::size_t end = start + dist(gen);

    HPX_TEST_LTE(start, end);

    base_iterator start_it = boost::next(std::begin(c1), start);
    base_iterator end_it = boost::next(std::begin(c1), end);

    {
        hpx::future<bool> result = hpx::parallel::includes(p,
            iterator(std::begin(c1)), iterator(std::end(c1)), start_it, end_it,
            std::less<std::size_t>());
        result.wait();

        bool expected = std::includes(std::begin(c1), std::end(c1), start_it,
            end_it, std::less<std::size_t>());

        // verify values
        HPX_TEST_EQ(result.get(), expected);
    }

    {
        std::vector<std::size_t> c2;
        std::copy(start_it, end_it, std::back_inserter(c2));

        if (!c2.empty())
        {
            std::uniform_int_distribution<> dis(0, c2.size() - 1);
            ++c2[dis(gen)];    //-V104

            hpx::future<bool> result = hpx::parallel::includes(p,
                iterator(std::begin(c1)), iterator(std::end(c1)),
                std::begin(c2), std::end(c2), std::less<std::size_t>());
            result.wait();

            bool expected = std::includes(std::begin(c1), std::end(c1),
                std::begin(c2), std::end(c2), std::less<std::size_t>());

            // verify values
            HPX_TEST_EQ(result.get(), expected);
        }
    }
}

template <typename IteratorTag>
void test_includes2()
{
    using namespace hpx::parallel;

    test_includes2(execution::seq, IteratorTag());
    test_includes2(execution::par, IteratorTag());
    test_includes2(execution::par_unseq, IteratorTag());

    test_includes2_async(execution::seq(execution::task), IteratorTag());
    test_includes2_async(execution::par(execution::task), IteratorTag());
}

void includes_test2()
{
    test_includes2<std::random_access_iterator_tag>();
    test_includes2<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_includes_exception(ExPolicy policy, IteratorTag)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);

    std::uniform_int_distribution<> dis(0, c1.size() - 1);
    std::size_t start = dis(gen);
    std::uniform_int_distribution<> dist(0, c1.size() - start - 1);
    std::size_t end = start + dist(gen);

    HPX_TEST_LTE(start, end);

    if (start == end)
        ++end;

    HPX_TEST_LTE(end, c1.size());

    base_iterator start_it = boost::next(std::begin(c1), start);
    base_iterator end_it = boost::next(std::begin(c1), end);

    bool caught_exception = false;
    try
    {
        hpx::parallel::includes(policy, iterator(std::begin(c1)),
            iterator(std::end(c1)), start_it, end_it,
            [](std::size_t v1, std::size_t v2) {
                return throw std::runtime_error("test"), true;
            });

        HPX_TEST(false);
    }
    catch (hpx::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(policy, e);
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag>
void test_includes_exception_async(ExPolicy p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);

    std::uniform_int_distribution<> dis(0, c1.size() - 1);
    std::size_t start = dis(gen);
    std::uniform_int_distribution<> dist(0, c1.size() - start - 1);
    std::size_t end = start + dist(gen);

    HPX_TEST_LTE(start, end);

    if (start == end)
        ++end;

    HPX_TEST_LTE(end, c1.size());

    base_iterator start_it = boost::next(std::begin(c1), start);
    base_iterator end_it = boost::next(std::begin(c1), end);

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        hpx::future<bool> f = hpx::parallel::includes(p,
            iterator(std::begin(c1)), iterator(std::end(c1)), start_it, end_it,
            [](std::size_t v1, std::size_t v2) {
                return throw std::runtime_error("test"), true;
            });
        returned_from_algorithm = true;
        f.get();

        HPX_TEST(false);
    }
    catch (hpx::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(p, e);
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
    HPX_TEST(returned_from_algorithm);
}

template <typename IteratorTag>
void test_includes_exception()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_includes_exception(execution::seq, IteratorTag());
    test_includes_exception(execution::par, IteratorTag());

    test_includes_exception_async(
        execution::seq(execution::task), IteratorTag());
    test_includes_exception_async(
        execution::par(execution::task), IteratorTag());
}

void includes_exception_test()
{
    test_includes_exception<std::random_access_iterator_tag>();
    test_includes_exception<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_includes_bad_alloc(ExPolicy policy, IteratorTag)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);

    std::uniform_int_distribution<> dis(0, c1.size() - 1);
    std::size_t start = dis(gen);
    std::uniform_int_distribution<> dist(0, c1.size() - start - 1);
    std::size_t end = start + dist(gen);

    HPX_TEST_LTE(start, end);

    if (start == end)
        ++end;

    HPX_TEST_LTE(end, c1.size());

    base_iterator start_it = boost::next(std::begin(c1), start);
    base_iterator end_it = boost::next(std::begin(c1), end);

    bool caught_bad_alloc = false;
    try
    {
        hpx::parallel::includes(policy, iterator(std::begin(c1)),
            iterator(std::end(c1)), start_it, end_it,
            [](std::size_t v1, std::size_t v2) {
                return throw std::bad_alloc(), true;
            });

        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
}

template <typename ExPolicy, typename IteratorTag>
void test_includes_bad_alloc_async(ExPolicy p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);

    std::uniform_int_distribution<> dis(0, c1.size() - 1);
    std::size_t start = dis(gen);
    std::uniform_int_distribution<> dist(0, c1.size() - start - 1);
    std::size_t end = start + dist(gen);

    HPX_TEST_LTE(start, end);

    if (start == end)
        ++end;

    HPX_TEST_LTE(end, c1.size());

    base_iterator start_it = boost::next(std::begin(c1), start);
    base_iterator end_it = boost::next(std::begin(c1), end);

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;
    try
    {
        hpx::future<bool> f = hpx::parallel::includes(p,
            iterator(std::begin(c1)), iterator(std::end(c1)), start_it, end_it,
            [](std::size_t v1, std::size_t v2) {
                return throw std::bad_alloc(), true;
            });
        returned_from_algorithm = true;
        f.get();

        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
    HPX_TEST(returned_from_algorithm);
}

template <typename IteratorTag>
void test_includes_bad_alloc()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_includes_bad_alloc(execution::seq, IteratorTag());
    test_includes_bad_alloc(execution::par, IteratorTag());

    test_includes_bad_alloc_async(
        execution::seq(execution::task), IteratorTag());
    test_includes_bad_alloc_async(
        execution::par(execution::task), IteratorTag());
}

void includes_bad_alloc_test()
{
    test_includes_bad_alloc<std::random_access_iterator_tag>();
    test_includes_bad_alloc<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(hpx::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    gen.seed(seed);

    includes_test1();
    includes_test2();
    includes_exception_test();
    includes_bad_alloc_test();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace hpx::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run");

    // By default this test should run on all available cores
    std::vector<std::string> const cfg = {"hpx.os_threads=all"};

    // Initialize and run HPX
    HPX_TEST_EQ_MSG(hpx::init(desc_commandline, argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
