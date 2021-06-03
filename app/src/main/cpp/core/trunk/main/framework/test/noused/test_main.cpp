// test_boost_suit2.cpp : 定义控制台应用程序的入口点。
 
#define BOOST_TEST_INCLUDE
 
#include <iostream>
#include <vector>
using namespace std;
 
#define BOOST_TEST_MODULE maintest
 
#include <boost/test/unit_test.hpp>
#include <boost/assign.hpp>
using namespace boost;
 
//全局的测试夹具类
struct global_fixture
{
    global_fixture(){cout<<"global setup"<<endl;}
    ~global_fixture(){cout<<"global teardown"<<endl;}
};
 
//定义全局夹具
BOOST_GLOBAL_FIXTURE(global_fixture);
 
//测试套件夹具类
struct assign_fixture
{
    assign_fixture()
    {
        cout<<"suit setup"<<endl;
    }
    ~assign_fixture()
    {
        cout<<"suit tear down"<<endl;
    }
    vector<int> v;
};
 
//测试定义套件级别的夹具
BOOST_FIXTURE_TEST_SUITE(s_assign, assign_fixture)
 
BOOST_AUTO_TEST_CASE(t_assign1)
{
    using namespace boost::assign;
 
    v += 1, 2, 3, 4;
    BOOST_CHECK_EQUAL(v.size(), 4);
    BOOST_CHECK_EQUAL(v[2], 3);
    BOOST_CHECK_EQUAL(v[2], 4);
}
 
BOOST_AUTO_TEST_CASE(t_assign2)
{
    using namespace boost::assign;
 
    push_back(v) (10) (20) (30);
 
    BOOST_CHECK_EQUAL(v.empty(), false);
    BOOST_CHECK_LT(v[0], v[1]);
    BOOST_CHECK_GT(v[0], v[1]);
}
 
BOOST_AUTO_TEST_SUITE_END()