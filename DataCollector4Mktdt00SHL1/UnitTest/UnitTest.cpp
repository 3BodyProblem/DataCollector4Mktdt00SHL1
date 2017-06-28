#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>


///< --------------------- 单元测试类定义 --------------------------------





///< ---------------------- 单元测试初始化类定义 -------------------------




///< ---------------- 单元测试导出函数定义 -------------------------------


void UnitTestEnv::SetUp()
{
}

void UnitTestEnv::TearDown()
{
}


void ExecuteTestCase()
{
	static	bool	s_bInit = false;

	if( false == s_bInit )	{
		int			nArgc = 1;
		char*		pszArgv[32] = { "DataCollector4CTP.dll", };

		s_bInit = true;
		testing::AddGlobalTestEnvironment( new UnitTestEnv() );
		testing::InitGoogleTest( &nArgc, pszArgv );
		RUN_ALL_TESTS();
	}
}











