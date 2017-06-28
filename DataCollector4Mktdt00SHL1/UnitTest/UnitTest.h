#ifndef __QLX_UNIT_TEST_H__
#define __QLX_UNIT_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "gtest/gtest.h"


///< --------------------- 单元测试类定义 --------------------------------




///< ------------ 单元测试初始化类定义 ------------------------------------


/**
 * @class					UnitTestEnv
 * @brief					全局事件(初始化引擎)
 * @author					barry
 */
class UnitTestEnv : public testing::Environment
{
public:
	void					SetUp();
	void					TearDown();

private:
};


/**
 * @brief					运行所有的测试例程
 * @author					barry
 */
void ExecuteTestCase();



#endif





