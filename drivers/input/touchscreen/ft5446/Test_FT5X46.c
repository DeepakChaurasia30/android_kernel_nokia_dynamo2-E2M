/************************************************************************
* Copyright (C) 2012-2015, Focaltech Systems (R)拢卢All Rights Reserved.
*
* File Name: Test_FT5X46.c
*
* Author: Software Development Team, AE
*
* Created: 2015-07-14
*
* Abstract: test item for FT5X46\FT5X46i
*
************************************************************************/

/*******************************************************************************
* Included header files
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include "Global.h"
#include "DetailThreshold.h"
#include "Test_FT5X46.h"
#include "Config_FT5X46.h"
//#include "Comm_FT5X46.h"
#include <asm/uaccess.h>
#ifdef CONFIG_FIH_PROJECT_E2M
#include <linux/mm.h>
#endif
/*******************************************************************************
* Private constant and macro definitions using #define
*******************************************************************************/
#define IC_TEST_VERSION  "Test version: V1.0.2--2015-11-27, (sync version of FT_MultipleTest: V2.7.0.3--2015-07-13)"

/////////////////////////////////////////////////Reg 
#define DEVIDE_MODE_ADDR	0x00
#define REG_LINE_NUM	0x01
#define REG_TX_NUM	0x02
#define REG_RX_NUM	0x03
#define REG_PATTERN_5422        0x53
#define REG_MAPPING_SWITCH      0x54
#define REG_TX_NOMAPPING_NUM        0x55
#define REG_RX_NOMAPPING_NUM      0x56
#define REG_NORMALIZE_TYPE      0x16
#define REG_ScCbBuf0	0x4E
#define REG_ScWorkMode	0x44
#define REG_ScCbAddrR	0x45
#define REG_RawBuf0 0x36
#define REG_WATER_CHANNEL_SELECT 0x09

/*******************************************************************************
* Private enumerations, structures and unions using typedef
*******************************************************************************/
enum WaterproofType
{
	WT_NeedProofOnTest,
	WT_NeedProofOffTest,
	WT_NeedTxOnVal,
	WT_NeedRxOnVal,
	WT_NeedTxOffVal,
	WT_NeedRxOffVal,
};
/*******************************************************************************
* Static variables
*******************************************************************************/

static int m_RawData[TX_NUM_MAX][RX_NUM_MAX] = {{0,0}};
static int m_iTempRawData[TX_NUM_MAX * RX_NUM_MAX] = {0};
static unsigned char m_ucTempData[TX_NUM_MAX * RX_NUM_MAX*2] = {0};
static bool m_bV3TP = false;
static int invalide[TX_NUM_MAX][RX_NUM_MAX] = {{0}};
static int RxLinearity[TX_NUM_MAX][RX_NUM_MAX] = {{0}};
static int TxLinearity[TX_NUM_MAX][RX_NUM_MAX] = {{0}};
static int m_DifferData[TX_NUM_MAX][RX_NUM_MAX] = {{0}};
static int m_absDifferData[TX_NUM_MAX][RX_NUM_MAX] = {{0}};

//---------------------About Store Test Dat
static char g_pStoreAllData[1024*80] = {0};
static char *g_pTmpBuff = NULL;
static char *g_pStoreMsgArea = NULL;
static int g_lenStoreMsgArea = 0;
static char *g_pMsgAreaLine2 = NULL;
static int g_lenMsgAreaLine2 = 0;
static char *g_pStoreDataArea = NULL;
static int g_lenStoreDataArea = 0;
static unsigned char m_ucTestItemCode = 0;
static int m_iStartLine = 0;
static int m_iTestDataCount = 0;

/*******************************************************************************
* Global variable or extern global variabls/functions
*******************************************************************************/


/*******************************************************************************
* Static function prototypes
*******************************************************************************/
//////////////////////////////////////////////Communication function
static int StartScan(void);
static unsigned char ReadRawData(unsigned char Freq, unsigned char LineNum, int ByteNum, int *pRevBuffer);
static unsigned char GetPanelRows(unsigned char *pPanelRows);
static unsigned char GetPanelCols(unsigned char *pPanelCols);
static unsigned char GetTxSC_CB(unsigned char index, unsigned char *pcbValue);
//////////////////////////////////////////////Common function
static unsigned char GetRawData(void);
static unsigned char GetChannelNum(void);
//////////////////////////////////////////////about Test
static void InitTest(void);
static void FinishTest(void);
static void Save_Test_Data(int iData[TX_NUM_MAX][RX_NUM_MAX], int iArrayIndex, unsigned char Row, unsigned char Col, unsigned char ItemCount);
static void InitStoreParamOfTestData(void);
static void MergeAllTestData(void);
//////////////////////////////////////////////Others 
static void AllocateMemory(void);
static void FreeMemory(void);
static void ShowRawData(void);
static boolean GetTestCondition(int iTestType, unsigned char ucChannelValue);

static unsigned char GetChannelNumNoMapping(void);
static unsigned char SwitchToNoMapping(void);


/************************************************************************
* Name: FT5X46_StartTest
* Brief:  Test entry. Determine which test item to test
* Input: none
* Output: none
* Return: Test Result, PASS or FAIL
***********************************************************************/
boolean FT5X46_StartTest()
{
	bool bTestResult = true;
	bool bTempResult = 1;
	unsigned char ReCode=0;
	unsigned char ucDevice = 0;
	int iItemCount=0;
	
	//--------------1. Init part
	InitTest();
	printk("[focal] - g_TestItemNum = %d \n", g_TestItemNum);       //show test item num	
	//--------------2. test item
	if(0 == g_TestItemNum)
		bTestResult = false;
	
	for(iItemCount = 0; iItemCount < g_TestItemNum; iItemCount++)
	{
		m_ucTestItemCode = g_stTestItem[ucDevice][iItemCount].ItemCode;

		///////////////////////////////////////////////////////FT5X22_ENTER_FACTORY_MODE
		if(Code_FT5X22_ENTER_FACTORY_MODE == g_stTestItem[ucDevice][iItemCount].ItemCode
			)
		{
			printk("[focal] Code_FT5X22_ENTER_FACTORY_MODE\n");
			ReCode = FT5X46_TestItem_EnterFactoryMode();
			if(ERROR_CODE_OK != ReCode || (!bTempResult))
			{
				bTestResult = false;
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
				break;//if this item FAIL, no longer test.				
			}
			else
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
		}

		///////////////////////////////////////////////////////FT5X22_CHANNEL_NUM_TEST
		/*if(Code_FT5X22_CHANNEL_NUM_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode
		)
		{
			printk("[focal] Code_FT5X22_CHANNEL_NUM_TEST");
		ReCode = FT5X46_TestItem_ChannelsTest(&bTempResult);
		if(ERROR_CODE_OK != ReCode || (!bTempResult))
		{
		bTestResult = false;
		}
		}*/	

		///////////////////////////////////////////////////////FT5X22_RAWDATA_TEST
		if(Code_FT5X22_RAWDATA_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode
			)
		{
			printk("[focal] Code_FT5X22_RAWDATA_TEST\n");
			ReCode = FT5X46_TestItem_RawDataTest(&bTempResult);
			if(ERROR_CODE_OK != ReCode || (!bTempResult))
			{
				bTestResult = false;
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
			}
			else
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
		}


		///////////////////////////////////////////////////////FT5X22_SCAP_CB_TEST
		if(Code_FT5X22_SCAP_CB_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode
			)
		{
			printk("[focal] Code_FT5X22_SCAP_CB_TEST\n");
			ReCode = FT5X46_TestItem_SCapCbTest(&bTempResult);
			if(ERROR_CODE_OK != ReCode || (!bTempResult))
			{
				bTestResult = false;
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
			}
			else
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
		}

		///////////////////////////////////////////////////////FT5X22_SCAP_RAWDATA_TEST
		if(Code_FT5X22_SCAP_RAWDATA_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode
			)
		{
			printk("[focal] Code_FT5X22_SCAP_RAWDATA_TEST\n");
			ReCode = FT5X46_TestItem_SCapRawDataTest(&bTempResult);
			if(ERROR_CODE_OK != ReCode || (!bTempResult))
			{
				bTestResult = false;
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
			}
			else
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
		}


		///////////////////////////////////////////////////////Code_FT5X22_UNIFORMITY_TEST
		if(Code_FT5X22_UNIFORMITY_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode
			)
		{
			printk("[focal] Code_FT5X22_UNIFORMITY_TEST \n");
			ReCode = FT5X46_TestItem_UniformityTest(&bTempResult);
			if(ERROR_CODE_OK != ReCode || (!bTempResult))
			{
				bTestResult = false;
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
			}
			else
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
		}

		///////////////////////////////////////////////////////Code_FT5X22_WEAK_SHORT_CIRCUIT_TEST
             /*
		if(Code_FT5X22_WEAK_SHORT_CIRCUIT_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode
			)
		{
			printk("[focal] Code_FT5X22_WEAK_SHORT_CIRCUIT_TEST \n");
			ReCode = FT5X46_TestItem_WeakShortTest(&bTempResult);
			if(ERROR_CODE_OK != ReCode || (!bTempResult))
			{
				bTestResult = false;
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
			}
			else
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
		}*/

		///////////////////////////////////////////////////////	Code_FT5X22_PANELDIFFER_TEST,
			if(Code_FT5X22_PANELDIFFER_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode
			)
		{
			printk("[focal] Code_FT5X22_PANELDIFFER_TEST \n");
			ReCode = FT5X46_TestItem_PanelDifferTest(&bTempResult);
			if(ERROR_CODE_OK != ReCode || (!bTempResult))
			{
				bTestResult = false;
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
			}
			else
				g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
		}
	}
	//--------------3. End Part
	FinishTest();
	//--------------4 Enter WrokMode
	for(iItemCount =0;iItemCount<3;iItemCount++) EnterWork();
	//--------------5. return result
	return bTestResult;
}
/************************************************************************
* Name: InitTest
* Brief:  Init all param before test
* Input: none
* Output: none
* Return: none
***********************************************************************/
static void InitTest(void)
{
	AllocateMemory();//Allocate pointer Memory
	
	InitStoreParamOfTestData();
	printk("[focal] %s \n", IC_TEST_VERSION);	//show lib version
}
/************************************************************************
* Name: FinishTest
* Brief:  Init all param before test
* Input: none
* Output: none
* Return: none
***********************************************************************/
static void FinishTest(void)
{
	MergeAllTestData();//Merge Test Result
	FreeMemory();//Release pointer memory
}
/************************************************************************
* Name: FT5X46_get_test_data
* Brief:  get data of test result
* Input: none
* Output: pTestData, the returned buff
* Return: the length of test data. if length > 0, got data;else ERR.
***********************************************************************/
int FT5X46_get_test_data(char *pTestData)
{
	if(NULL == pTestData)
	{
		printk("[focal] %s pTestData == NULL \n", __func__);	
		return -1;
	}
	memcpy(pTestData, g_pStoreAllData, (g_lenStoreMsgArea+g_lenStoreDataArea));
	return (g_lenStoreMsgArea+g_lenStoreDataArea);	
}

/************************************************************************
* Name: FT5X46_TestItem_EnterFactoryMode
* Brief:  Check whether TP can enter Factory Mode, and do some thing
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_EnterFactoryMode(void)
{	
	unsigned char ReCode = ERROR_CODE_INVALID_PARAM;
	int iRedo = 5;
	int i ;
	unsigned char chPattern=0;

	SysDelay(150);
	for(i = 1; i <= iRedo; i++)
	{
		ReCode = EnterFactory();
		if(ERROR_CODE_OK != ReCode)
		{
			printk("Failed to Enter factory mode...\n");
			if(i < iRedo)
			{
				SysDelay(50);
				continue;
			}
		}
		else
		{
			break;
		}

	}
	SysDelay(300);


	if(ReCode != ERROR_CODE_OK)	
	{	
		return ReCode;
	}

	ReCode = GetChannelNum();

	//theDevice.m_cHidDev[m_NumDevice]->WriteReg(0xFB, 0);

	ReCode = ReadReg( REG_PATTERN_5422, &chPattern );
	if (chPattern == 1)
	{
		m_bV3TP = true;
	}
	else
	{
		m_bV3TP = false;
	}

	return ReCode;
}
/************************************************************************
* Name: FT5X46_TestItem_RawDataTest
* Brief:  TestItem: RawDataTest. Check if MCAP RawData is within the range.
* Input: none
* Output: bTestResult, PASS or FAIL
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_RawDataTest(bool * bTestResult)
{
	unsigned char ReCode = 0;
	bool btmpresult = true;
	int RawDataMin;
	int RawDataMax;
	unsigned char ucFre;
	unsigned char strSwitch = 0;
	unsigned char OriginValue = 0xff;
	unsigned char OriginVal = 0xff;
	int index = 0;
	int iRow, iCol;
	int iValue = 0;

    struct file *pfile = NULL;
    mm_segment_t old_fs;
    loff_t pos;
    char txt_buffer[1024];

	printk("\n\n==============================Test Item: -------- Raw Data  Test \n\n");

    //androidboot.mode=2 is FTM mode, androidboot.mode=0 is AOS mode
      if (strnstr(saved_command_line, "androidboot.mode=2", strlen(saved_command_line))) {
        if (NULL == pfile)
        pfile = filp_open("/data/touch_data.txt", O_CREAT|O_RDWR, 0600);
        if (IS_ERR(pfile)) {
                    pr_err("[focal] error occured while opening file /data/touch_data.txt.\n");
                        } else {
                        old_fs = get_fs();
                        set_fs(KERNEL_DS);
                        pos = 0;
                        sprintf(txt_buffer, "DATA=%d;", g_ScreenSetParam.iTxNum * g_ScreenSetParam.iRxNum);
                        vfs_write(pfile, txt_buffer, strlen(txt_buffer), &pos);
                        //pos += strlen(txt_buffer);
        }
      }
	ReCode = EnterFactory(); 
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("\n\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
		goto TEST_ERR;
	}

	if (m_bV3TP)
	{
		ReCode = ReadReg( REG_MAPPING_SWITCH, &strSwitch );
		if (strSwitch != 0)
		{
			ReCode = WriteReg( REG_MAPPING_SWITCH, 0 );
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
		}			
	}


	ReCode = ReadReg( REG_NORMALIZE_TYPE, &OriginValue );	
	if( ReCode != ERROR_CODE_OK )goto TEST_ERR;


	if (g_ScreenSetParam.isNormalize == Auto_Normalize)
	{
		if(OriginValue != 1)
		{
			ReCode = WriteReg( REG_NORMALIZE_TYPE, 0x01 );
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
		}

		printk( "\n=========Set Frequecy High\n" );
		ReCode = WriteReg( 0x0A, 0x81 );
		if( ReCode != ERROR_CODE_OK )goto TEST_ERR;

		printk( "\n=========FIR State: ON\n");
		ReCode = ReadReg(0xFB,&OriginVal);
		if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
		ReCode = WriteReg(0xFB, 1);
		if( ReCode != ERROR_CODE_OK )goto TEST_ERR;

		for (index = 0; index < 3; ++index )
		{
			ReCode = GetRawData();
		}

		if( ReCode != ERROR_CODE_OK )  
		{
			printk("\nGet Rawdata failed, Error Code: 0x%x",  ReCode);
			goto TEST_ERR;
		}

		ShowRawData();

		////////////////////////////////To Determine RawData if in Range or not
		for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)
		{
			for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
			{
				if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node
				RawDataMin = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Min[iRow][iCol];
				RawDataMax = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Max[iRow][iCol];
				iValue = m_RawData[iRow][iCol];
				if(iValue < RawDataMin || iValue > RawDataMax)
				{
					btmpresult = false;
					printk("rawdata test failure. Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) \n", \
						iRow+1, iCol+1, iValue, RawDataMin, RawDataMax);
				}
				if(btmpresult == false)
				    sprintf(txt_buffer, "(C,%d,%d,%d,%d,%d,F);", iRow, iCol, RawDataMax, RawDataMin, iValue);
				else
				    sprintf(txt_buffer, "(C,%d,%d,%d,%d,%d,P);", iRow, iCol, RawDataMax, RawDataMin, iValue);
				if (strnstr(saved_command_line, "androidboot.mode=2", strlen(saved_command_line))) {
				    vfs_write(pfile, txt_buffer, strlen(txt_buffer), &pos);
				    //pos += strlen(txt_buffer);
				}
			}
		}	
		if (strnstr(saved_command_line, "androidboot.mode=2", strlen(saved_command_line))) {
		    filp_close(pfile, NULL);
		    set_fs(old_fs);
		}
		//////////////////////////////Save Test Data
		ReCode = WriteReg(0xFB,OriginVal);
		if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
              if (strnstr(saved_command_line, "androidboot.mode=2", strlen(saved_command_line))) 
		Save_Test_Data(m_RawData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 2);
	}
	else
	{	
		if(OriginValue != 0)
		{
			ReCode = WriteReg( REG_NORMALIZE_TYPE, 0x00 );
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
		}

		ReCode =  ReadReg( 0x0A, &ucFre );
		if( ReCode != ERROR_CODE_OK )goto TEST_ERR;


		if(g_stCfg_FT5X22_BasicThreshold.RawDataTest_SetLowFreq)
		{
			printk("\n=========Set Frequecy Low\n");
			ReCode = WriteReg( 0x0A, 0x80 );
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;

			//FIR OFF

			printk("\n=========FIR State: OFF\n" );
			ReCode = ReadReg(0xFB,&OriginVal);
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
			ReCode = WriteReg(0xFB, 0);
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
			SysDelay(100);

			for (index = 0; index < 3; ++index )
			{
				ReCode = GetRawData();
			}

			if( ReCode != ERROR_CODE_OK )  
			{
				printk("\nGet Rawdata failed, Error Code: 0x%x",  ReCode);
				goto TEST_ERR;
			}
			ShowRawData();

			////////////////////////////////To Determine RawData if in Range or not
			for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)
			{

				for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
				{
					if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node
					RawDataMin = g_stCfg_MCap_DetailThreshold.RawDataTest_Low_Min[iRow][iCol];
					RawDataMax = g_stCfg_MCap_DetailThreshold.RawDataTest_Low_Max[iRow][iCol];
					iValue = m_RawData[iRow][iCol];
#if STDebug
					printk("[FocalTech]Low Compare Condition RawDataMin:%d RawDataMax:%d Value:%d\n",RawDataMin,RawDataMax,iValue);
#endif
					if(iValue < RawDataMin || iValue > RawDataMax)
					{
						btmpresult = false;
						printk("rawdata test failure. Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) \n", \
							iRow+1, iCol+1, iValue, RawDataMin, RawDataMax);
					}
				}
			}

			//////////////////////////////Save Test Data
			ReCode = WriteReg(0xFB,OriginVal);
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
                  if (strnstr(saved_command_line, "androidboot.mode=2", strlen(saved_command_line))) 
			Save_Test_Data(m_RawData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 1);
		}


		if ( g_stCfg_FT5X22_BasicThreshold.RawDataTest_SetHighFreq )
		{

			printk( "\n=========Set Frequecy High\n");
			ReCode = WriteReg( 0x0A, 0x81 );
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;

			printk("\n=========FIR State: OFF\n" );
			ReCode = ReadReg(0xFB,&OriginVal);
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
			ReCode = WriteReg(0xFB, 0);
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
			SysDelay(100);

			for (index = 0; index < 3; ++index )
			{
				ReCode = GetRawData();
			}

			if( ReCode != ERROR_CODE_OK )  
			{
				printk("\nGet Rawdata failed, Error Code: 0x%x",  ReCode);
				if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
			}
			ShowRawData();

			////////////////////////////////To Determine RawData if in Range or not
			for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)
			{

				for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
				{
					if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node
					RawDataMin = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Min[iRow][iCol];
					RawDataMax = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Max[iRow][iCol];
					iValue = m_RawData[iRow][iCol];
#if STDebug
					printk("[FocalTech]High Compare Condition RawDataMin:%d RawDataMax:%d Value:%d\n",RawDataMin,RawDataMax,iValue);
#endif
					if(iValue < RawDataMin || iValue > RawDataMax)
					if(iValue < RawDataMin || iValue > RawDataMax)
					{
						btmpresult = false;
						printk("rawdata test failure. Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) \n", \
							iRow+1, iCol+1, iValue, RawDataMin, RawDataMax);
					}
				}
			}

			//////////////////////////////Save Test Data
			ReCode = WriteReg(0xFB,OriginVal);
			if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
                  if (strnstr(saved_command_line, "androidboot.mode=2", strlen(saved_command_line))) 
			Save_Test_Data(m_RawData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 2);			
		}

	}



	ReCode = WriteReg( REG_NORMALIZE_TYPE, OriginValue );
	if( ReCode != ERROR_CODE_OK )goto TEST_ERR;


	if (m_bV3TP)
	{
		ReCode = WriteReg( REG_MAPPING_SWITCH, strSwitch );
		if( ReCode != ERROR_CODE_OK )goto TEST_ERR;
	}

	//-------------------------Result
	if( btmpresult )
	{
		*bTestResult = true;
		printk("\n\n//RawData Test is OK!\n");
	}
	else
	{
		* bTestResult = false;
		printk("\n\n//RawData Test is NG!\n");
	}
	return ReCode;

TEST_ERR:

	* bTestResult = false;
	printk("\n\n//RawData Test is NG!\n");
	return ReCode;

}
/************************************************************************
* Name: FT5X46_TestItem_SCapRawDataTest
* Brief:  TestItem: SCapRawDataTest. Check if SCAP RawData is within the range.
* Input: none
* Output: bTestResult, PASS or FAIL
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_SCapRawDataTest(bool * bTestResult)
{
	int i =0;
	int RawDataMin = 0;
	int RawDataMax = 0; 
	int Value = 0;
	boolean bFlag = true;
	unsigned char ReCode = 0;
	boolean btmpresult = true;
	int iMax=0;
	int iMin=0;
	int iAvg=0;
	int ByteNum=0;
	unsigned char wc_value = 0;//waterproof channel value
	unsigned char ucValue = 0;
	int iCount = 0;
	int ibiggerValue = 0;

	printk("\n\n==============================Test Item: -------- Scap RawData Test \n\n");
	//-------1.Preparatory work	
	//in Factory Mode
	ReCode = EnterFactory(); 
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("\n\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
		goto TEST_ERR;
	}

	//get waterproof channel setting, to check if Tx/Rx channel need to test
	ReCode = ReadReg( REG_WATER_CHANNEL_SELECT, &wc_value );
	if(ReCode != ERROR_CODE_OK)	goto TEST_ERR;

	//If it is V3 pattern, Get Tx/Rx Num again
	ReCode= SwitchToNoMapping();
	if(ReCode != ERROR_CODE_OK)	goto TEST_ERR;

	//-------2.Get SCap Raw Data, Step:1.Start Scanning; 2. Read Raw Data
	ReCode = StartScan();
	if(ReCode != ERROR_CODE_OK)
	{	
		printk("Failed to Scan SCap RawData! \n");
		goto TEST_ERR;
	}
	for(i = 0; i < 3; i++)
	{
		memset(m_iTempRawData, 0, sizeof(m_iTempRawData));

		//rawdata
		ByteNum = (g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum)*2;
		ReCode = ReadRawData(0, 0xAC, ByteNum, m_iTempRawData);
		if(ReCode != ERROR_CODE_OK)goto TEST_ERR;
		memcpy( m_RawData[0+g_ScreenSetParam.iTxNum], m_iTempRawData, sizeof(int)*g_ScreenSetParam.iRxNum );
		memcpy( m_RawData[1+g_ScreenSetParam.iTxNum], m_iTempRawData + g_ScreenSetParam.iRxNum, sizeof(int)*g_ScreenSetParam.iTxNum );

		//rawdata
		ByteNum = (g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum)*2;
		ReCode = ReadRawData(0, 0xAB, ByteNum, m_iTempRawData);
		if(ReCode != ERROR_CODE_OK)goto TEST_ERR;
		memcpy( m_RawData[2+g_ScreenSetParam.iTxNum], m_iTempRawData, sizeof(int)*g_ScreenSetParam.iRxNum );
		memcpy( m_RawData[3+g_ScreenSetParam.iTxNum], m_iTempRawData + g_ScreenSetParam.iRxNum, sizeof(int)*g_ScreenSetParam.iTxNum );	
	}


	//-----3. Judge

	//Waterproof ON
	bFlag=GetTestCondition(WT_NeedProofOnTest, wc_value);		
	if(g_stCfg_FT5X22_BasicThreshold.SCapRawDataTest_SetWaterproof_ON && bFlag )
	{
		iCount = 0;
		RawDataMin = g_stCfg_FT5X22_BasicThreshold.SCapRawDataTest_ON_Min;
		RawDataMax = g_stCfg_FT5X22_BasicThreshold.SCapRawDataTest_ON_Max;
		iMax = -m_RawData[0+g_ScreenSetParam.iTxNum][0];
		iMin = 2 * m_RawData[0+g_ScreenSetParam.iTxNum][0];
		iAvg = 0;
		Value = 0;

		
		bFlag=GetTestCondition(WT_NeedRxOnVal, wc_value);	
		if(bFlag)
			printk("Judge Rx in Waterproof-ON:\n");
		for( i = 0; bFlag && i < g_ScreenSetParam.iRxNum; i++ )
		{
			if( g_stCfg_MCap_DetailThreshold.InvalidNode_SC[0][i] == 0 )      continue;
			RawDataMin = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_ON_Min[0][i];
			RawDataMax = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_ON_Max[0][i];				
			Value = m_RawData[0+g_ScreenSetParam.iTxNum][i];
			iAvg += Value;
			if(iMax < Value) iMax = Value;//find the Max value
			if(iMin > Value) iMin = Value;//fine the min value
			if(Value > RawDataMax || Value < RawDataMin) 
			{
				btmpresult = false;
				printk("Failed. Num = %d, Value = %d, range = (%d, %d):\n", i+1, Value, RawDataMin, RawDataMax);
			}
			iCount++;
		}

		
		bFlag=GetTestCondition(WT_NeedTxOnVal, wc_value);
		if(bFlag)
			printk("Judge Tx in Waterproof-ON:\n");
		for(i = 0;bFlag && i < g_ScreenSetParam.iTxNum; i++)
		{
			if( g_stCfg_MCap_DetailThreshold.InvalidNode_SC[1][i] == 0 )      continue;
			RawDataMin = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_ON_Min[1][i];
			RawDataMax = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_ON_Max[1][i];				
			Value = m_RawData[1+g_ScreenSetParam.iTxNum][i];
			iAvg += Value;
			if(iMax < Value) iMax = Value;//find the Max value
			if(iMin > Value) iMin = Value;//fine the min value
			if(Value > RawDataMax || Value < RawDataMin) 
			{
				btmpresult = false;
				printk("Failed. Num = %d, Value = %d, range = (%d, %d):\n", i+1, Value, RawDataMin, RawDataMax);
			}
			iCount++;
		}
		if(0 == iCount)
		{
			iAvg = 0;
			iMax = 0;
			iMin = 0;
		}
		else				
			iAvg = iAvg/iCount;

		printk("SCap RawData in Waterproof-ON, Max : %d, Min: %d, Deviation: %d, Average: %d\n", iMax, iMin, iMax - iMin, iAvg);
		//////////////////////////////Save Test Data
		ibiggerValue = g_ScreenSetParam.iTxNum>g_ScreenSetParam.iRxNum?g_ScreenSetParam.iTxNum:g_ScreenSetParam.iRxNum;
		Save_Test_Data(m_RawData, g_ScreenSetParam.iTxNum+0, 2, ibiggerValue, 1);	
	}

	//Waterproof OFF
	bFlag=GetTestCondition(WT_NeedProofOffTest, wc_value);
	if(g_stCfg_FT5X22_BasicThreshold.SCapRawDataTest_SetWaterproof_OFF && bFlag)
	{
		iCount = 0;
		RawDataMin = g_stCfg_FT5X22_BasicThreshold.SCapRawDataTest_OFF_Min;
		RawDataMax = g_stCfg_FT5X22_BasicThreshold.SCapRawDataTest_OFF_Max;
		iMax = -m_RawData[2+g_ScreenSetParam.iTxNum][0];
		iMin = 2 * m_RawData[2+g_ScreenSetParam.iTxNum][0];
		iAvg = 0;
		Value = 0;
		
		bFlag=GetTestCondition(WT_NeedRxOffVal, wc_value);
		if(bFlag)
			printk("Judge Rx in Waterproof-OFF:\n");
		for(i = 0; bFlag && i < g_ScreenSetParam.iRxNum; i++)
		{
			if( g_stCfg_MCap_DetailThreshold.InvalidNode_SC[0][i] == 0 )      continue;
			RawDataMin = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_OFF_Min[0][i];
			RawDataMax = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_OFF_Max[0][i];
			Value = m_RawData[2+g_ScreenSetParam.iTxNum][i];
			iAvg += Value;

			//printk("zaxzax3 Value %d RawDataMin %d  RawDataMax %d  \n", Value, RawDataMin, RawDataMax);
			//strTemp += str;
			if(iMax < Value) iMax = Value;
			if(iMin > Value) iMin = Value;
			if(Value > RawDataMax || Value < RawDataMin) 
			{
				btmpresult = false;
				printk("Failed. Num = %d, Value = %d, range = (%d, %d):\n", i+1, Value, RawDataMin, RawDataMax);
			}
			iCount++;
		}
		
		bFlag=GetTestCondition(WT_NeedTxOffVal, wc_value);	
		if(bFlag)
			printk("Judge Tx in Waterproof-OFF:\n");
		for(i = 0; bFlag && i < g_ScreenSetParam.iTxNum; i++)
		{
			if( g_stCfg_MCap_DetailThreshold.InvalidNode_SC[1][i] == 0 )      continue;

			Value = m_RawData[3+g_ScreenSetParam.iTxNum][i];
			RawDataMin = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_OFF_Min[1][i];
			RawDataMax = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_OFF_Max[1][i];
			//printk("zaxzax4 Value %d RawDataMin %d  RawDataMax %d  \n", Value, RawDataMin, RawDataMax);
			iAvg += Value;
			if(iMax < Value) iMax = Value;
			if(iMin > Value) iMin = Value;
			if(Value > RawDataMax || Value < RawDataMin) 
			{
				btmpresult = false;
				printk("Failed. Num = %d, Value = %d, range = (%d, %d):\n", i+1, Value, RawDataMin, RawDataMax);
			}
			iCount++;
		}
		if(0 == iCount)
		{
			iAvg = 0;
			iMax = 0;
			iMin = 0;
		}
		else				
			iAvg = iAvg/iCount;

		printk("SCap RawData in Waterproof-OFF, Max : %d, Min: %d, Deviation: %d, Average: %d\n", iMax, iMin, iMax - iMin, iAvg);
		//////////////////////////////Save Test Data
		ibiggerValue = g_ScreenSetParam.iTxNum>g_ScreenSetParam.iRxNum?g_ScreenSetParam.iTxNum:g_ScreenSetParam.iRxNum;
		Save_Test_Data(m_RawData, g_ScreenSetParam.iTxNum+2, 2, ibiggerValue, 2);	
	}
	//-----4. post-stage work
	if(m_bV3TP)
	{
		ReCode = ReadReg( REG_MAPPING_SWITCH, &ucValue );
		if (0 !=ucValue )
		{
			ReCode = WriteReg( REG_MAPPING_SWITCH, 0 );
			SysDelay(10); 			
			if( ReCode != ERROR_CODE_OK)	
			{
				printk("Failed to switch mapping type!\n ");
				btmpresult = false;
			}
		}	

		GetChannelNum();
	}

	//-----5. Test Result
	if( btmpresult )
	{
		*bTestResult = true;
		printk("\n\n//SCap RawData Test is OK!\n");
	}
	else
	{
		* bTestResult = false;
		printk("\n\n//SCap RawData Test is NG!\n");
	}
	return ReCode;

TEST_ERR:
	* bTestResult = false;
	printk("\n\n//SCap RawData Test is NG!\n");	
	return ReCode;	
}

/************************************************************************
* Name: FT5X46_TestItem_SCapCbTest
* Brief:  TestItem: SCapCbTest. Check if SCAP Cb is within the range.
* Input: none
* Output: bTestResult, PASS or FAIL
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_SCapCbTest(bool* bTestResult)
{
	int i,/* j, iOutNum,*/index,Value,CBMin,CBMax;
	boolean bFlag = true;
	unsigned char ReCode;
	boolean btmpresult = true;
	int iMax, iMin, iAvg;
	unsigned char wc_value = 0;
	unsigned char ucValue = 0;
	int iCount = 0;
	int ibiggerValue = 0;

	printk("\n\n==============================Test Item: -----  Scap CB Test \n\n");	
	//-------1.Preparatory work	
	//in Factory Mode
	ReCode = EnterFactory(); 
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("\n\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
		goto TEST_ERR;
	}

	//get waterproof channel setting, to check if Tx/Rx channel need to test
	ReCode = ReadReg( REG_WATER_CHANNEL_SELECT, &wc_value );
	if(ReCode != ERROR_CODE_OK)	goto TEST_ERR;

	//If it is V3 pattern, Get Tx/Rx Num again
	bFlag= SwitchToNoMapping();
	if( bFlag )
	{	
		printk("Failed to SwitchToNoMapping! \n");
		goto TEST_ERR;
	}

	//-------2.Get SCap Raw Data, Step:1.Start Scanning; 2. Read Raw Data
	ReCode = StartScan();
	if(ReCode != ERROR_CODE_OK)
	{	
		printk("Failed to Scan SCap RawData! \n");
		goto TEST_ERR;
	}


	for(i = 0; i < 3; i++)
	{
		memset(m_RawData, 0, sizeof(m_RawData));
		memset(m_ucTempData, 0, sizeof(m_ucTempData));

		ReCode = WriteReg( REG_ScWorkMode, 1 );
		ReCode = StartScan();
		ReCode = WriteReg( REG_ScCbAddrR, 0 );			
		ReCode = GetTxSC_CB( g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum + 128, m_ucTempData );
		for ( index = 0; index < g_ScreenSetParam.iRxNum; ++index )
		{
			m_RawData[0 + g_ScreenSetParam.iTxNum][index]= m_ucTempData[index];
		}
		for ( index = 0; index < g_ScreenSetParam.iTxNum; ++index )
		{
			m_RawData[1 + g_ScreenSetParam.iTxNum][index] = m_ucTempData[index + g_ScreenSetParam.iRxNum];
		}

		//rawdata
		ReCode = WriteReg( REG_ScWorkMode, 0 );
		ReCode = StartScan();
		ReCode = WriteReg( REG_ScCbAddrR, 0 );
		ReCode = GetTxSC_CB( g_ScreenSetParam.iRxNum + g_ScreenSetParam.iTxNum + 128, m_ucTempData );
		for ( index = 0; index < g_ScreenSetParam.iRxNum; ++index )
		{
			m_RawData[2 + g_ScreenSetParam.iTxNum][index]= m_ucTempData[index];
		}
		for ( index = 0; index < g_ScreenSetParam.iTxNum; ++index )
		{
			m_RawData[3 + g_ScreenSetParam.iTxNum][index] = m_ucTempData[index + g_ScreenSetParam.iRxNum];
		}

		if( ReCode != ERROR_CODE_OK )	
		{
			printk("Failed to Get SCap CB!\n");
		}		
	}

	if(ReCode != ERROR_CODE_OK)	goto TEST_ERR;

	//-----3. Judge

	//Waterproof ON
	bFlag=GetTestCondition(WT_NeedProofOnTest, wc_value);			
	if(g_stCfg_FT5X22_BasicThreshold.SCapCbTest_SetWaterproof_ON && bFlag)
	{
		printk("SCapCbTest in WaterProof On Mode:  \n");

		iMax = -m_RawData[0+g_ScreenSetParam.iTxNum][0];
		iMin = 2 * m_RawData[0+g_ScreenSetParam.iTxNum][0];
		iAvg = 0;
		Value = 0;
		iCount = 0;

		
		bFlag=GetTestCondition(WT_NeedRxOnVal, wc_value);
		if(bFlag)
			printk("SCap CB_Rx:  \n");
		for( i = 0;bFlag && i < g_ScreenSetParam.iRxNum; i++ )
		{
			if( g_stCfg_MCap_DetailThreshold.InvalidNode_SC[0][i] == 0 )      continue;
			CBMin = g_stCfg_MCap_DetailThreshold.SCapCbTest_ON_Min[0][i];
			CBMax = g_stCfg_MCap_DetailThreshold.SCapCbTest_ON_Max[0][i];
			Value = m_RawData[0+g_ScreenSetParam.iTxNum][i];
			iAvg += Value;

			if(iMax < Value) iMax = Value;//find the Max Value
			if(iMin > Value) iMin = Value;//find the Min Value
			if(Value > CBMax || Value < CBMin) 
			{
				btmpresult = false;
				printk("Failed. Num = %d, Value = %d, range = (%d, %d):\n", i+1, Value, CBMin, CBMax);
			}
			iCount++;
		}

		
		bFlag=GetTestCondition(WT_NeedTxOnVal, wc_value);
		if(bFlag)
			printk("SCap CB_Tx:  \n");
		for(i = 0;bFlag &&  i < g_ScreenSetParam.iTxNum; i++)
		{
			if( g_stCfg_MCap_DetailThreshold.InvalidNode_SC[1][i] == 0 )      continue;
			CBMin = g_stCfg_MCap_DetailThreshold.SCapCbTest_ON_Min[1][i];
			CBMax = g_stCfg_MCap_DetailThreshold.SCapCbTest_ON_Max[1][i];
			Value = m_RawData[1+g_ScreenSetParam.iTxNum][i];
			iAvg += Value;
			if(iMax < Value) iMax = Value;
			if(iMin > Value) iMin = Value;
			if(Value > CBMax || Value < CBMin) 
			{
				btmpresult = false;
				printk("Failed. Num = %d, Value = %d, range = (%d, %d):\n", i+1, Value, CBMin, CBMax);
			}
			iCount++;
		}

		if(0 == iCount)
		{
			iAvg = 0;
			iMax = 0;
			iMin = 0;
		}
		else				
			iAvg = iAvg/iCount;

		printk("SCap CB in Waterproof-ON, Max : %d, Min: %d, Deviation: %d, Average: %d\n", iMax, iMin, iMax - iMin, iAvg);
		//////////////////////////////Save Test Data
		ibiggerValue = g_ScreenSetParam.iTxNum>g_ScreenSetParam.iRxNum?g_ScreenSetParam.iTxNum:g_ScreenSetParam.iRxNum;
		Save_Test_Data(m_RawData, g_ScreenSetParam.iTxNum+0, 2, ibiggerValue, 1);			
	}

	bFlag=GetTestCondition(WT_NeedProofOffTest, wc_value);
	if(g_stCfg_FT5X22_BasicThreshold.SCapCbTest_SetWaterproof_OFF && bFlag)
	{			
		printk("SCapCbTest in WaterProof OFF Mode:  \n");
		iMax = -m_RawData[2+g_ScreenSetParam.iTxNum][0];
		iMin = 2 * m_RawData[2+g_ScreenSetParam.iTxNum][0];
		iAvg = 0;
		Value = 0;
		iCount = 0;

		
		bFlag=GetTestCondition(WT_NeedRxOffVal, wc_value);
		if(bFlag)
			printk("SCap CB_Rx:  \n");
		for(i = 0;bFlag &&  i < g_ScreenSetParam.iRxNum; i++)
		{
			if( g_stCfg_MCap_DetailThreshold.InvalidNode_SC[0][i] == 0 )      continue;
			CBMin = g_stCfg_MCap_DetailThreshold.SCapCbTest_OFF_Min[0][i];
			CBMax = g_stCfg_MCap_DetailThreshold.SCapCbTest_OFF_Max[0][i];
			Value = m_RawData[2+g_ScreenSetParam.iTxNum][i];
			iAvg += Value;

			if(iMax < Value) iMax = Value;
			if(iMin > Value) iMin = Value;
			if(Value > CBMax || Value < CBMin) 
			{
				btmpresult = false;
				printk("Failed. Num = %d, Value = %d, range = (%d, %d):\n", i+1, Value, CBMin, CBMax);
			}
			iCount++;
		}

		
		bFlag=GetTestCondition(WT_NeedTxOffVal, wc_value);	
		if(bFlag)
			printk("SCap CB_Tx:  \n");
		for(i = 0; bFlag && i < g_ScreenSetParam.iTxNum; i++)
		{
			//if( m_ScapInvalide[1][i] == 0 )      continue;
			if( g_stCfg_MCap_DetailThreshold.InvalidNode_SC[1][i] == 0 )      continue;
			CBMin = g_stCfg_MCap_DetailThreshold.SCapCbTest_OFF_Min[1][i];
			CBMax = g_stCfg_MCap_DetailThreshold.SCapCbTest_OFF_Max[1][i];
			Value = m_RawData[3+g_ScreenSetParam.iTxNum][i];

			iAvg += Value;
			if(iMax < Value) iMax = Value;
			if(iMin > Value) iMin = Value;
			if(Value > CBMax || Value < CBMin) 
			{
				btmpresult = false;
				printk("Failed. Num = %d, Value = %d, range = (%d, %d):\n", i+1, Value, CBMin, CBMax);
			}
			iCount++;
		}

		if(0 == iCount)
		{
			iAvg = 0;
			iMax = 0;
			iMin = 0;
		}
		else				
			iAvg = iAvg/iCount;

		printk("SCap CB in Waterproof-OFF, Max : %d, Min: %d, Deviation: %d, Average: %d\n", iMax, iMin, iMax - iMin, iAvg);
		//////////////////////////////Save Test Data
		ibiggerValue = g_ScreenSetParam.iTxNum>g_ScreenSetParam.iRxNum?g_ScreenSetParam.iTxNum:g_ScreenSetParam.iRxNum;
		Save_Test_Data(m_RawData, g_ScreenSetParam.iTxNum+2, 2, ibiggerValue, 2);	
	}
	//-----4. post-stage work
	if(m_bV3TP)
	{
		ReCode = ReadReg( REG_MAPPING_SWITCH, &ucValue );
		if (0 != ucValue )
		{
			ReCode = WriteReg( REG_MAPPING_SWITCH, 0 );
			SysDelay(10); 			
			if( ReCode != ERROR_CODE_OK)	
			{
				printk("Failed to switch mapping type!\n ");
				btmpresult = false;
			}
		}	


		GetChannelNum();
	}

	//-----5. Test Result

	if( btmpresult )
	{
		*bTestResult = true;
		printk("\n\n//SCap CB Test Test is OK!\n");
	}
	else
	{
		* bTestResult = false;
		printk("\n\n//SCap CB Test Test is NG!\n");
	}
	return ReCode;

TEST_ERR:

	* bTestResult = false;
	printk("\n\n//SCap CB Test Test is NG!\n");
	return ReCode;	
}

/************************************************************************
* Name: GetPanelRows(Same function name as FT_MultipleTest)
* Brief:  Get row of TP
* Input: none
* Output: pPanelRows
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetPanelRows(unsigned char *pPanelRows)
{
	return ReadReg(REG_TX_NUM, pPanelRows);
}

/************************************************************************
* Name: GetPanelCols(Same function name as FT_MultipleTest)
* Brief:  get column of TP
* Input: none
* Output: pPanelCols
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetPanelCols(unsigned char *pPanelCols)
{
	return ReadReg(REG_RX_NUM, pPanelCols);
}
/************************************************************************
* Name: StartScan(Same function name as FT_MultipleTest)
* Brief:  Scan TP, do it before read Raw Data
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static int StartScan(void)
{
	unsigned char RegVal = 0;
	unsigned char times = 0;
	const unsigned char MaxTimes = 20;
	unsigned char ReCode = ERROR_CODE_COMM_ERROR;

	ReCode = ReadReg(DEVIDE_MODE_ADDR, &RegVal);
	if(ReCode == ERROR_CODE_OK)
	{
		RegVal |= 0x80;
		ReCode = WriteReg(DEVIDE_MODE_ADDR, RegVal);
		if(ReCode == ERROR_CODE_OK)
		{
			while(times++ < MaxTimes)
			{
				SysDelay(8);	//8ms
				ReCode = ReadReg(DEVIDE_MODE_ADDR, &RegVal);
				if(ReCode == ERROR_CODE_OK)
				{
					if((RegVal>>7) == 0)	break;
				}
				else
				{
					break;
				}
			}
			if(times < MaxTimes)	ReCode = ERROR_CODE_OK;
			else ReCode = ERROR_CODE_COMM_ERROR;
		}
	}
	return ReCode;

}	
/************************************************************************
* Name: ReadRawData(Same function name as FT_MultipleTest)
* Brief:  read Raw Data
* Input: Freq(No longer used, reserved), LineNum, ByteNum
* Output: pRevBuffer
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char ReadRawData(unsigned char Freq, unsigned char LineNum, int ByteNum, int *pRevBuffer)
{
	unsigned char ReCode=ERROR_CODE_COMM_ERROR;
	unsigned char I2C_wBuffer[3];
	int i, iReadNum;
	unsigned short BytesNumInTestMode1=0;

	//unsigned short BytesNumInTestMode3=0, BytesNumInTestMode2=0,BytesNumInTestMode1=0;
	//unsigned short BytesNumInTestMode6=0, BytesNumInTestMode5=0,BytesNumInTestMode4=0;

	iReadNum=ByteNum/342;

	if(0 != (ByteNum%342)) iReadNum++;

	if(ByteNum <= 342)
	{
		BytesNumInTestMode1 = ByteNum;		
	}
	else
	{
		BytesNumInTestMode1 = 342;
	}

	ReCode = WriteReg(REG_LINE_NUM, LineNum);//Set row addr;


	//***********************************************************Read raw data		
	I2C_wBuffer[0] = REG_RawBuf0;	//set begin address
	if(ReCode == ERROR_CODE_OK)
	{
		focal_msleep(10);
		ReCode = Comm_Base_IIC_IO(I2C_wBuffer, 1, m_ucTempData, BytesNumInTestMode1);
	}

	for(i=1; i<iReadNum; i++)
	{
		if(ReCode != ERROR_CODE_OK) break;

		if(i==iReadNum-1)//last packet
		{
			focal_msleep(10);
			ReCode = Comm_Base_IIC_IO(NULL, 0, m_ucTempData+342*i, ByteNum-342*i);
		}
		else
		{
			focal_msleep(10);
			ReCode = Comm_Base_IIC_IO(NULL, 0, m_ucTempData+342*i, 342);	
		}

	}

	if(ReCode == ERROR_CODE_OK)
	{
		for(i=0; i<(ByteNum>>1); i++)
		{
			pRevBuffer[i] = (m_ucTempData[i<<1]<<8)+m_ucTempData[(i<<1)+1];
			//if(pRevBuffer[i] & 0x8000)
			//{
			//	pRevBuffer[i] -= 0xffff + 1;
			//}
		}
	}

	return ReCode;

}
/************************************************************************
* Name: GetTxSC_CB(Same function name as FT_MultipleTest)
* Brief:  get CB of Tx SCap
* Input: index
* Output: pcbValue
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char GetTxSC_CB(unsigned char index, unsigned char *pcbValue)
{
	unsigned char ReCode = ERROR_CODE_OK;
	unsigned char wBuffer[4];

	if(index<128)
	{	
		*pcbValue = 0;
		WriteReg(REG_ScCbAddrR, index);
		ReCode = ReadReg(REG_ScCbBuf0, pcbValue);
	}
	else//index-128
	{
		WriteReg(REG_ScCbAddrR, 0);
		wBuffer[0] = REG_ScCbBuf0;	
		ReCode = Comm_Base_IIC_IO(wBuffer, 1, pcbValue, index-128);

	}	

	return ReCode;
}


//////////////////////////////////////////////
/************************************************************************
* Name: AllocateMemory
* Brief:  Allocate pointer Memory
* Input: none
* Output: none
* Return: none
***********************************************************************/
static void AllocateMemory(void)
{
	//New buff
	g_pStoreMsgArea =NULL;	
	if(NULL == g_pStoreMsgArea)
		g_pStoreMsgArea = kmalloc(1024*80, GFP_ATOMIC);
	if (g_pStoreMsgArea == NULL){
		g_pStoreMsgArea = vmalloc(1024*80);
		pr_err("[focal] %s() - ERROR: g_pStoreMsgArea kmalloc fail, do vmalloc\n", __func__);
	}
	g_pMsgAreaLine2 =NULL;	
	if(NULL == g_pMsgAreaLine2)
		g_pMsgAreaLine2 = kmalloc(1024*80, GFP_ATOMIC);
	if (g_pMsgAreaLine2 == NULL){
		g_pMsgAreaLine2 = vmalloc(1024*80);
		pr_err("[focal] %s() - ERROR: g_pMsgAreaLine2 kmalloc fail, do vmalloc\n", __func__);
	}
	g_pStoreDataArea =NULL;	
	if(NULL == g_pStoreDataArea)
		g_pStoreDataArea = kmalloc(1024*80, GFP_ATOMIC);
	if (g_pStoreDataArea == NULL){
		g_pStoreDataArea = vmalloc(1024*80);
		pr_err("[focal] %s() - ERROR: g_pStoreDataArea kmalloc fail, do vmalloc\n", __func__);
	}
	/*g_pStoreAllData =NULL;	
	if(NULL == g_pStoreAllData)
		g_pStoreAllData = kmalloc(1024*8, GFP_ATOMIC);
	g_pTmpBuff =NULL;*/	
	if(NULL == g_pTmpBuff)
		g_pTmpBuff = kmalloc(1024*16, GFP_ATOMIC);
	if (g_pTmpBuff == NULL){
		g_pTmpBuff = vmalloc(1024*16);
		pr_err("[focal] %s() - ERROR: g_pTmpBuff kmalloc fail, do vmalloc\n", __func__);
	}

}
/************************************************************************
* Name: FreeMemory
* Brief:  Release pointer memory
* Input: none
* Output: none
* Return: none
***********************************************************************/
static void FreeMemory(void)
{
	//Release buff
	/*
	if(NULL == g_pStoreMsgArea)
		kfree(g_pStoreMsgArea);

	if(NULL == g_pMsgAreaLine2)
		kfree(g_pMsgAreaLine2);

	if(NULL == g_pStoreDataArea)
		kfree(g_pStoreDataArea);

	//if(NULL == g_pStoreAllData)
	//	kfree(g_pStoreAllData);

	if(NULL == g_pTmpBuff)
		kfree(g_pTmpBuff);
		*/

	if(NULL == g_pStoreMsgArea){
		if (is_vmalloc_addr(g_pStoreMsgArea)){
		    printk("[focal] %s  vfree(g_pStoreMsgArea) \n", __func__);
		    vfree(g_pStoreMsgArea);
		}
		else{
		    printk("[focal] %s  kfree(g_pStoreMsgArea) \n", __func__);
		    kfree(g_pStoreMsgArea);
		}
	}

	if(NULL == g_pMsgAreaLine2){
		if (is_vmalloc_addr(g_pMsgAreaLine2)){
		    printk("[focal] %s  vfree(g_pMsgAreaLine2) \n", __func__);
		    vfree(g_pMsgAreaLine2);
		}
		else{
		    printk("[focal] %s  kfree(g_pMsgAreaLine2) \n", __func__);
		    kfree(g_pMsgAreaLine2);
		}
	}

	if(NULL == g_pStoreDataArea){
		if (is_vmalloc_addr(g_pStoreDataArea)){
		    printk("[focal] %s vfree(g_pStoreDataArea) \n", __func__);
		    vfree(g_pStoreDataArea);
		}
		else{
		    printk("[focal] %s kfree(g_pStoreDataArea) \n", __func__);
		    kfree(g_pStoreDataArea);
		}
	}

	if(NULL == g_pTmpBuff){
		if (is_vmalloc_addr(g_pTmpBuff)){
		    printk("[focal] %s vfree(g_pTmpBuff) \n", __func__);
		    vfree(g_pTmpBuff);
		}
		else{
		    printk("[focal] %s  kfree(g_pTmpBuff) \n", __func__);
		    kfree(g_pTmpBuff);
		}
	}

}

/************************************************************************
* Name: InitStoreParamOfTestData
* Brief:  Init store param of test data
* Input: none
* Output: none
* Return: none
***********************************************************************/
static void InitStoreParamOfTestData(void)
{


	g_lenStoreMsgArea = 0;
	//Msg Area, Add Line1
	g_lenStoreMsgArea += sprintf(g_pStoreMsgArea,"ECC, 85, 170, IC Name, %s, IC Code, %x\n", g_strIcName,  g_ScreenSetParam.iSelectedIC);
	//Line2
	//g_pMsgAreaLine2 = NULL;
	g_lenMsgAreaLine2 = 0;

	//Data Area
	//g_pStoreDataArea = NULL;
	g_lenStoreDataArea = 0;
	m_iStartLine = 11;//The Start Line of Data Area is 11

	m_iTestDataCount = 0;	
}
/************************************************************************
* Name: MergeAllTestData
* Brief:  Merge All Data of test result
* Input: none
* Output: none
* Return: none
***********************************************************************/
static void MergeAllTestData(void)
{
	int iLen = 0;

	//Add the head part of Line2
	iLen= sprintf(g_pTmpBuff,"TestItem, %d, ", m_iTestDataCount);
	memcpy(g_pStoreMsgArea+g_lenStoreMsgArea, g_pTmpBuff, iLen);
	g_lenStoreMsgArea+=iLen;

	//Add other part of Line2, except for "\n"
	memcpy(g_pStoreMsgArea+g_lenStoreMsgArea, g_pMsgAreaLine2, g_lenMsgAreaLine2);
	g_lenStoreMsgArea+=g_lenMsgAreaLine2;	

	//Add Line3 ~ Line10
	iLen= sprintf(g_pTmpBuff,"\n\n\n\n\n\n\n\n\n");
	memcpy(g_pStoreMsgArea+g_lenStoreMsgArea, g_pTmpBuff, iLen);
	g_lenStoreMsgArea+=iLen;

	///1.Add Msg Area
	memcpy(g_pStoreAllData, g_pStoreMsgArea, g_lenStoreMsgArea);

	///2.Add Data Area
	if(0!= g_lenStoreDataArea)
	{
		memcpy(g_pStoreAllData+g_lenStoreMsgArea, g_pStoreDataArea, g_lenStoreDataArea);
	}

	printk("[focal] %s lenStoreMsgArea=%d,  lenStoreDataArea = %d\n", __func__, g_lenStoreMsgArea, g_lenStoreDataArea);
}


/************************************************************************
* Name: Save_Test_Data
* Brief:  Storage format of test data
* Input: int iData[TX_NUM_MAX][RX_NUM_MAX], int iArrayIndex, unsigned char Row, unsigned char Col, unsigned char ItemCount
* Output: none
* Return: none
***********************************************************************/
static void Save_Test_Data(int iData[TX_NUM_MAX][RX_NUM_MAX], int iArrayIndex, unsigned char Row, unsigned char Col, unsigned char ItemCount)
{
	int iLen = 0;
	int i = 0, j = 0;

	//Save  Msg (ItemCode is enough, ItemName is not necessary, so set it to "NA".)
	iLen= sprintf(g_pTmpBuff,"NA, %d, %d, %d, %d, %d, ", \
		m_ucTestItemCode, Row, Col, m_iStartLine, ItemCount);
	memcpy(g_pMsgAreaLine2+g_lenMsgAreaLine2, g_pTmpBuff, iLen);
	g_lenMsgAreaLine2 += iLen;

	m_iStartLine += Row;
	m_iTestDataCount++;

	//Save Data 
	for(i = 0+iArrayIndex; i < Row+iArrayIndex; i++)
	{
		for(j = 0; j < Col; j++)
		{
			if(j == (Col -1))//The Last Data of the Row, add "\n"
				iLen= sprintf(g_pTmpBuff,"%d, \n", iData[i][j]);	
			else
				iLen= sprintf(g_pTmpBuff,"%d, ", iData[i][j]);	

			memcpy(g_pStoreDataArea+g_lenStoreDataArea, g_pTmpBuff, iLen);
			g_lenStoreDataArea += iLen;		
		}
	}

}

/************************************************************************
* Name: GetChannelNum
* Brief:  Get Channel Num(Tx and Rx)
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetChannelNum(void)
{
	unsigned char ReCode;
	unsigned char rBuffer[1]; //= new unsigned char;

	//m_strCurrentTestMsg = "Get Tx Num...";
	ReCode = GetPanelRows(rBuffer);
	if(ReCode == ERROR_CODE_OK)
	{
		g_ScreenSetParam.iTxNum = rBuffer[0];	
		if(g_ScreenSetParam.iTxNum > g_ScreenSetParam.iUsedMaxTxNum)
		{
			printk("Failed to get Tx number, Get num = %d, UsedMaxNum = %d\n",
				g_ScreenSetParam.iTxNum, g_ScreenSetParam.iUsedMaxTxNum);
			return ERROR_CODE_INVALID_PARAM;
		}
	}
	else
	{
		printk("Failed to get Tx number\n");
	}

	///////////////m_strCurrentTestMsg = "Get Rx Num...";

	ReCode = GetPanelCols(rBuffer);
	if(ReCode == ERROR_CODE_OK)
	{
		g_ScreenSetParam.iRxNum = rBuffer[0];
		if(g_ScreenSetParam.iRxNum > g_ScreenSetParam.iUsedMaxRxNum)
		{
			printk("Failed to get Rx number, Get num = %d, UsedMaxNum = %d\n",
				g_ScreenSetParam.iRxNum, g_ScreenSetParam.iUsedMaxRxNum);
			return ERROR_CODE_INVALID_PARAM;
		}		
	}
	else
	{
		printk("Failed to get Rx number\n");
	}

	return ReCode;

}
/************************************************************************
* Name: GetRawData
* Brief:  Get Raw Data of MCAP
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetRawData(void)
{
	unsigned char ReCode = ERROR_CODE_OK;
	int iRow = 0;
	int iCol = 0;		

	//--------------------------------------------Enter Factory Mode
	ReCode = EnterFactory();	
	if( ERROR_CODE_OK != ReCode ) 
	{
		printk("Failed to Enter Factory Mode...\n");
		return ReCode;
	}


	//--------------------------------------------Check Num of Channel 
	if(0 == (g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum)) 
	{
		ReCode = GetChannelNum();
		if( ERROR_CODE_OK != ReCode ) 
		{
			printk("Error Channel Num...\n");
			return ERROR_CODE_INVALID_PARAM;
		}
	}

#if STDebug
	printk("[FocalTech] Tx:%d,Rx:%d\n",g_ScreenSetParam.iTxNum,g_ScreenSetParam.iRxNum);
#endif

	//--------------------------------------------Start Scanning
	printk("Start Scan ...\n");
	ReCode = StartScan();
	if(ERROR_CODE_OK != ReCode) 
	{
		printk("Failed to Scan ...\n");
		return ReCode;
	}

	//--------------------------------------------Read RawData, Only MCAP
	memset(m_RawData, 0, sizeof(m_RawData));
	ReCode = ReadRawData( 1, 0xAA, ( g_ScreenSetParam.iTxNum * g_ScreenSetParam.iRxNum )*2, m_iTempRawData );
	for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++)
	{
		for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
		{
			m_RawData[iRow][iCol] = m_iTempRawData[iRow*g_ScreenSetParam.iRxNum + iCol];
		}
	}
	return ReCode;
}
/************************************************************************
* Name: ShowRawData
* Brief:  Show RawData
* Input: none
* Output: none
* Return: none.
***********************************************************************/
static void ShowRawData(void)
{
	int iRow, iCol;
	//----------------------------------------------------------Show RawData
	for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++)
	{
		printk("\nTx%2d:  ", iRow+1);
		for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
		{
			printk("%5d    ", m_RawData[iRow][iCol]);
		}
	}
}

/************************************************************************
* Name: GetChannelNumNoMapping
* Brief:  get Tx&Rx num from other Register
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetChannelNumNoMapping(void)
{
	unsigned char ReCode;
	unsigned char rBuffer[1]; //= new unsigned char;


	printk("Get Tx Num...\n");
	ReCode =ReadReg( REG_TX_NOMAPPING_NUM,  rBuffer);
	if(ReCode == ERROR_CODE_OK)
	{
		g_ScreenSetParam.iTxNum= rBuffer[0];	
	}
	else
	{
		printk("Failed to get Tx number\n");
	}


	printk("Get Rx Num...\n");
	ReCode = ReadReg( REG_RX_NOMAPPING_NUM,  rBuffer);
	if(ReCode == ERROR_CODE_OK)
	{
		g_ScreenSetParam.iRxNum = rBuffer[0];
	}
	else
	{
		printk("Failed to get Rx number\n");
	}

	return ReCode;
}
/************************************************************************
* Name: SwitchToNoMapping
* Brief:  If it is V3 pattern, Get Tx/Rx Num again
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char SwitchToNoMapping(void)
{
	unsigned char chPattern = -1;
	unsigned char ReCode = ERROR_CODE_OK;
	unsigned char RegData = -1;
	ReCode = ReadReg( REG_PATTERN_5422, &chPattern );//

	if(1 == chPattern)// 1: V3 Pattern
	{	
		RegData = -1;
		ReCode =ReadReg( REG_MAPPING_SWITCH, &RegData );
		if( 1 != RegData ) 
		{
			ReCode = WriteReg( REG_MAPPING_SWITCH, 1 );  //0-mapping 1-no mampping
			focal_msleep(20);
			GetChannelNumNoMapping();
		}
	}

	if( ReCode != ERROR_CODE_OK )
	{
		printk("Switch To NoMapping Failed!\n");
	}
	return ReCode;
}
/************************************************************************
* Name: GetTestCondition
* Brief:  Check whether Rx or TX need to test, in Waterproof ON/OFF Mode.
* Input: none
* Output: none
* Return: true: need to test; false: Not tested.
***********************************************************************/
static boolean GetTestCondition(int iTestType, unsigned char ucChannelValue)
{
	boolean bIsNeeded = false;
	switch(iTestType)
	{
	case WT_NeedProofOnTest://Bit5:  0; 1
		bIsNeeded = !( ucChannelValue & 0x20 );
		break;
	case WT_NeedProofOffTest://Bit7: 0; 1
		bIsNeeded = !( ucChannelValue & 0x80 );
		break;
	case WT_NeedTxOnVal:
		//Bit6:  0 :Rx+Tx ; 1
		//Bit2:  0: Tx;  1: Rx
		bIsNeeded = !( ucChannelValue & 0x40 ) || !( ucChannelValue & 0x04 );
		break;			
	case WT_NeedRxOnVal:
		//Bit6:  0 : Rx+Tx; 1
		//Bit2:  0: Tx;  1:Rx
		bIsNeeded = !( ucChannelValue & 0x40 ) || ( ucChannelValue & 0x04 );
		break;			
	case WT_NeedTxOffVal://Bit1,Bit0:  00: Tx; 10: Rx+Tx 
		bIsNeeded = (0x00 == (ucChannelValue & 0x03)) || (0x02 == ( ucChannelValue & 0x03 ));
		break;			
	case WT_NeedRxOffVal://Bit1,Bit0:  01: Rx;    10: Rx+Tx
		bIsNeeded = (0x01 == (ucChannelValue & 0x03)) || (0x02 == ( ucChannelValue & 0x03 ));
		break;
	default:break;
	}
	return bIsNeeded;
}

unsigned char FT5X46_TestItem_UniformityTest(bool * bTestResult)
{
	unsigned char ReCode = ERROR_CODE_COMM_ERROR;
	bool btmpresult = true;
	unsigned char strSwitch = -1;
	unsigned char FirValue = 1;

	int index = 0;

//	int TxLinearity[TX_NUM_MAX][RX_NUM_MAX] = {0};

	int iRow = 0;
	int iCol = 1;
	int iDeviation = 0;
	int iMax = 0;

//	int minHole[TX_NUM_MAX][RX_NUM_MAX] = {0};
//	int maxHole[TX_NUM_MAX][RX_NUM_MAX] = {0};

//	int shieldNode[TX_NUM_MAX][RX_NUM_MAX] = {0};

	int iMin = 0;	
	int iUniform = 0;

	//bool bResult = false;
	int iValue = 0;					

	printk("\r\n\r\n==============================Test Item: -------- RawData Uniformity Test\r\n" );
	ReCode = EnterFactory(); 
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("\r\n\r\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}

	printk("\n" );

	//先判断是否为v3屏体，然后读取0x54的值，并判断与设定的mapping类型是否一致，不一致写入数据
	//rawdata test mapping后，mapping前：0x54=1;mapping后：0x54=0;
	if (m_bV3TP)
	{
		printk("m_bV3TP \n" );

		ReCode = ReadReg( REG_MAPPING_SWITCH, &strSwitch );
		if(ReCode != ERROR_CODE_OK)		
		{
			printk("\n Read REG_MAPPING_SWITCH error. Error Code: %d\n", ReCode);
			goto TEST_ERR;
		}
		
		if (strSwitch != 0)
		{
			ReCode = WriteReg( REG_MAPPING_SWITCH, 0 );
			if(ReCode != ERROR_CODE_OK)		
			{
				printk("\n Write REG_MAPPING_SWITCH error. Error Code: %d\n", ReCode);
				goto TEST_ERR;
			}
			
			ReCode = GetChannelNum();
			if(ReCode != ERROR_CODE_OK)		
			{
				printk("\n GetChannelNum error. Error Code: %d",  ReCode);
				goto TEST_ERR;
			}
		}			
	}

	printk("\n" );

	ReCode = ReadReg(0xFB, &FirValue);
	if (ReCode != ERROR_CODE_OK)
	{
		printk("\r\nRead fir Reg Failed. error:%d. \n", ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}

	ReCode = WriteReg( 0x0A, 0x81 );
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("\n Write 0x0A error. Error Code: %d\n", ReCode);
		goto TEST_ERR;
	}
	
	SysDelay(10);	
	if (g_ScreenSetParam.isNormalize == Auto_Normalize)
	{
		printk("[focal] Auto_Normalize");
		ReCode = WriteReg(0xFB, 1);
		if(ReCode != ERROR_CODE_OK)		
		{
			printk("\n Write 0xFB error. Error Code: %d\n", ReCode);
			goto TEST_ERR;
		}

		ReCode = WriteReg( 0x0A, 0x81 );
		if(ReCode != ERROR_CODE_OK)		
		{
			printk("\n Write 0x0A error. Error Code: %d\n", ReCode);
			goto TEST_ERR;
		}
	}else
	{
		printk("[focal] NOT Auto_Normalize");
		ReCode = WriteReg(0xFB, 0);
		if(ReCode != ERROR_CODE_OK)		
		{
			printk("[focal] \n Write 0xFB error. Error Code: %d\n", ReCode);
			goto TEST_ERR;
		}

	}

	printk("\n" );
		
	SysDelay(10);

	for ( index = 0; index < 3; ++index )
	{
		ReCode = GetRawData();
	}
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("[focal] GetRawData error. Error Code: %d\n", ReCode);
		goto TEST_ERR;
	}

	ShowRawData();

	printk("\n" );

	if( g_stCfg_FT5X22_BasicThreshold.Uniformity_CheckTx )
	{
	        printk("\r\n=========Check Tx Linearity \r\n");
		for ( iRow = 0; iRow < g_ScreenSetParam.iTxNum; ++iRow )
		{
			for ( iCol = 1; iCol <  g_ScreenSetParam.iRxNum; ++iCol )
			{
				iDeviation = abs( m_RawData[iRow][iCol] - m_RawData[iRow][iCol-1] );
				iMax = max( m_RawData[iRow][iCol], m_RawData[iRow][iCol-1] );
				iMax = iMax ? iMax : 1;
				TxLinearity[iRow][iCol] = 100 * iDeviation / iMax;

			}
		}
		printk("[focal]");
	
		{
		//	NodeVal nodeOutRange;
		//	AnalyzeInfo info( g_ScreenSetParam.iTxNum,  g_ScreenSetParam.iRxNum, false );
			
		//	memset( minHole, 0/*MIN_HOLE_LEVEL*/, sizeof(minHole) );
  		//	          memcpy_s( invalide, sizeof(invalide), g_stCfg_MCap_DetailThreshold.InvalidNode, sizeof(g_stCfg_MCap_DetailThreshold.InvalidNode) );
		//		memcpy_s( maxHole, sizeof(maxHole), g_stCfg_MCap_DetailThreshold.TxLinearityTest_Max, sizeof(g_stCfg_MCap_DetailThreshold.TxLinearityTest_Max) );

			for( iRow = 0; iRow < g_ScreenSetParam.iTxNum; ++iRow )
			{
				for( iCol = 0; iCol <  g_ScreenSetParam.iRxNum; ++iCol )
					if( 0 == iCol ) invalide[iRow][iCol] = 0;
			}
		printk("[focal]");

			/*	// need
			bResult = AnalyzeTestResultMCap( TxLinearity, minHole, maxHole,
				invalide, info, textTemp, nodeOutRange );

			TestResultInfo( textTemp );
			
			if( !bResult )
			{
				printk("\r\n Tx Linearity Out Of Range:\r\n");
				btmpresult = false;
			}
			*/

			////////////////////////////////To show value
		#if 1
			printk(" Tx Linearity:\n");
			for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)
			{
				printk("\nTx%2d:    ", iRow+1);
				for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
				{
				//	if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node

					iValue = TxLinearity[iRow][iCol];
					printk("%4d,  ", iValue);
				}
				printk("\n" );
			}
			printk("\n" );
		#endif
			
			////////////////////////////////To Determine  if in Range or not
			for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)
			{
			//	printk("Tx%2d:  		"  , iRow);
				for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
				{
					if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node
				//	RawDataMin = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Min[iRow][iCol];
				//	RawDataMax = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Max[iRow][iCol];

					iMin = 0 ; // minHole[iRow][iCol];
					iMax = g_stCfg_MCap_DetailThreshold.TxLinearityTest_Max[iRow][iCol];

					iValue = TxLinearity[iRow][iCol];
					if(iValue < iMin || iValue > iMax)
					{
						btmpresult = false;
						printk("Tx Linearity Out Of Range.  Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) \n", \
							iRow+1, iCol+1, iValue, iMin, iMax);						
					}
				}
			}

		}

	//	SaveTestData( GetSaveMatrixData( TxLinearity, 0, g_ScreenSetParam.iTxNum, 1,  g_ScreenSetParam.iRxNum ), g_ScreenSetParam.iTxNum,  g_ScreenSetParam.iRxNum-1, 1 );
			//////////////////////////////Save Test Data
		//		Save_Test_Data(m_RawData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 2);			
		Save_Test_Data(TxLinearity, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 1);			
	}
	if( g_stCfg_FT5X22_BasicThreshold.Uniformity_CheckRx )
	{
		printk("\r\n=========Check Rx Linearity \r\n");
	//	NodeVal* pNodeRoot = new NodeVal;
		for ( iRow = 1; iRow < g_ScreenSetParam.iTxNum; ++iRow )
		{
			for ( iCol = 0; iCol < g_ScreenSetParam.iRxNum; ++iCol )
			{
        			iDeviation = abs( m_RawData[iRow][iCol] - m_RawData[iRow-1][iCol] );
				iMax = max( m_RawData[iRow][iCol], m_RawData[iRow-1][iCol] );
				iMax = iMax ? iMax : 1;
				RxLinearity[iRow][iCol] = 100 * iDeviation / iMax;
			}
		}

		{
		//	memset( minHole, 0/*MIN_HOLE_LEVEL*/, sizeof(minHole) );
		//	memcpy_s( invalide, sizeof(invalide), g_stCfg_MCap_DetailThreshold.InvalidNode, sizeof(g_stCfg_MCap_DetailThreshold.InvalidNode) );
		//	memcpy_s( maxHole, sizeof(maxHole), g_stCfg_MCap_DetailThreshold.RxLinearityTest_Max, sizeof(g_stCfg_MCap_DetailThreshold.RxLinearityTest_Max) );

			for(  iRow = 0; iRow < g_ScreenSetParam.iTxNum; ++iRow )
			{
				for(  iCol = 0; iCol <  g_ScreenSetParam.iRxNum; ++iCol )
					if( 0 == iRow ) invalide[iRow][iCol] = 0;
			}
			/*
			bool bResult = AnalyzeTestResultMCap( RxLinearity, minHole, maxHole,
				invalide, info, textTemp, nodeOutRange );

			TestResultInfo( textTemp );
			*/

			////////////////////////////////To show value
		#if 1
			printk("  Rx Linearity:\n");
			for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)
			{
				printk("\nTx%2d:    ", iRow+1);
				for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
				{
				//	if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node

					iValue = RxLinearity[iRow][iCol];
					printk("%4d,  ", iValue);
				}
				printk("\n" );
			}
			printk("\n" );
		#endif

			////////////////////////////////To Determine  if in Range or not
			for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)	//	iRow = 1 ???
			{
				for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
				{
					if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node

					iMin = 0 ; // minHole[iRow][iCol];
					iMax = g_stCfg_MCap_DetailThreshold.RxLinearityTest_Max[iRow][iCol];

					iValue = RxLinearity[iRow][iCol];
					if(iValue < iMin || iValue > iMax)
					{
						btmpresult = false;
						printk("Rx Linearity Out Of Range.  Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) \n", \
							iRow+1, iCol+1, iValue, iMin, iMax);						
					}
				}
			}
			////	end determine

		}

	//	SaveTestData( GetSaveMatrixData( RxLinearity, 1, g_ScreenSetParam.iTxNum, 0,  g_ScreenSetParam.iRxNum ), g_ScreenSetParam.iTxNum-1,  g_ScreenSetParam.iRxNum, 2 );
		Save_Test_Data(RxLinearity, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 1);			
	}
	if( g_stCfg_FT5X22_BasicThreshold.Uniformity_CheckMinMax )
	{
        	printk("\r\n=========Check Min/Max \r\n") ;
		iMin = 100000;
		iMax = -100000;
		for (  iRow = 0; iRow < g_ScreenSetParam.iTxNum; ++iRow )
		{
			for (  iCol = 0; iCol < g_ScreenSetParam.iRxNum; ++iCol )
			{
				if( 0 == g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] ){
					continue;
				}
				if( 2 == g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] ){
					continue;
				}
				iMin = min( iMin, m_RawData[iRow][iCol] );
				iMax = max( iMax, m_RawData[iRow][iCol] );
			}
		}
		iMax = !iMax ? 1 : iMax;
		iUniform = 100 * abs(iMin) / abs(iMax);

		printk("\r\n Min: %d, Max: %d, , Get Value of Min/Max: %d", iMin, iMax, iUniform );

		if( iUniform < g_stCfg_FT5X22_BasicThreshold.Uniformity_MinMax_Hole )
		{
			btmpresult = false;
			printk("\r\n MinMax Out Of Range, Set Value: %d", g_stCfg_FT5X22_BasicThreshold.Uniformity_MinMax_Hole  );
		}
		printk("iuniform:%d,",iUniform);
	//	SaveTestData(strText , 1, 1, 3 );

	}

	ReCode = WriteReg(0xFB, FirValue);
	if (ReCode != ERROR_CODE_OK)
	{
		printk("\r\nWrite fir Reg 0xFB Failed. error:%d. \n", ReCode);
		goto TEST_ERR;
	}

	//恢复v3屏体的mapping值
	if (m_bV3TP)
	{
		ReCode = WriteReg( REG_MAPPING_SWITCH, strSwitch );
		if( ReCode != ERROR_CODE_OK)	
		{
			printk("\r\nFailed to restore mapping type!\r\n ");
			btmpresult = false;
		}
	}

	if( btmpresult && ReCode == ERROR_CODE_OK )
	{
		*bTestResult = true;
		printk("Uniformity Test is OK.");
	}
	else
	{
		*bTestResult = false;
		printk("Uniformity Test is NG.");
	}

	printk("Uniformity Test END. \n\n\n\n");
	return ReCode;

TEST_ERR:
	* bTestResult = false;
	printk("\n\n//Uniformity Test is NG!\n\n\n\n");
	return ReCode;
}

#if 0
unsigned char FT5X46_TestItem_WeakShortTest( bool* bTestResult )
{
	unsigned char ReCode = ERROR_CODE_COMM_ERROR;

	//unsigned char Id[2];
	//int i, j, iOutThresholdNum;
	int i=0;
	//bool bFlag = true;
	bool btmpresult = true;
	//int iMax, iMin, iAvg;

	int iAllAdcDataNum = 63;	
	int iMaxTx = 35;
	unsigned char iTxNum, iRxNum, iChannelNum;
	int iClbData_Ground, iClbData_Mutual, iOffset,iRsen,iCCRsen;
	unsigned char IcValue = 0;
	unsigned char strSwitch=1;
	bool  bCapShortTest = false;  

	int *iAdcData  = NULL;

	int fKcal = 0;	//	float fKcal = 0;
	int *fMShortResistance = NULL, *fGShortResistance = NULL;	//	loat *fMShortResistance, *fGShortResistance;

	int iDoffset = 0, iDsen = 0, iDrefn = 0;

	int iMin_CG = 0;

	int iCount = 0;

	int iMin_CC = 0;
	int iDCal = 0;
	int iMa =0;

	printk("\n");

	printk("\n\n\n\n==============================Test Item: -----  Weak Short-Circuit Test \r\n\r\n");

	//enter work mode,read 0xB1 value

	// read ICValue
	/*
	if( IsPramModeTest() ){
		ReCode = EnterFactory(); 
		SysDelay(200);
		//read 0xb1, <=0x05||==0xff:E版本     =0x06 || 其他值:F版本
		ReCode = ReadReg(0x4B, &IcValue);//Get IC type
	}
	else{
		ReCode = EnterWork(); 
		SysDelay(200);
		//read 0xb1, <=0x05||==0xff:E版本     =0x06 || 其他值:F版本
		ReCode = ReadReg(0xB1, &IcValue);//Get IC type
	}
	*/
	ReCode = EnterWork(); 	//	
	if(ReCode != ERROR_CODE_OK)		
	{
		printk(" EnterWork failed.. Error Code: %d", ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}
	SysDelay(200);

	//read 0xb1, <=0x05||==0xff:E版本     =0x06 || 其他值:F版本
	ReCode = ReadReg(0xB1, &IcValue);//Get IC type
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("\n Read 0xB1 IcValue error. Error Code: %d\n", ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}
	else
		printk(" IcValue:0x%02x.  \n", IcValue);

	iRsen = 57;
	//新添加的测试项，只针对互短
	iCCRsen = g_stCfg_FT5X22_BasicThreshold.WeakShortTest_CC_Rsen;
	bCapShortTest =	g_stCfg_FT5X22_BasicThreshold.WeakShortTest_CapShortTest;
	printk(" iCCRsen:%d.  \n", iCCRsen);
	printk(" bCapShortTest:%d.  \n", bCapShortTest);

	ReCode = EnterFactory();
	SysDelay(100);
	if(ReCode != ERROR_CODE_OK)		
	{
		printk(" EnterFactory failed.. Error Code: %d", ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}
	
	//先判断是否为v3屏体，然后读取0x54的值，并判断与设定的mapping类型是否一致，不一致写入数据
	//weakshort test mapping前，mapping前：0x54=1;mapping后：0x54=0;
	if (m_bV3TP)
	{
		ReCode = ReadReg( REG_MAPPING_SWITCH, &strSwitch );
		if (strSwitch != 1)
		{
			ReCode = WriteReg( REG_MAPPING_SWITCH, 1 );
			SysDelay(20);
			if( ReCode != ERROR_CODE_OK)	
			{
				printk("\r\nFailed to restore mapping type!\r\n ");
				btmpresult = false;
			}
			GetChannelNumNoMapping();

			iTxNum = g_ScreenSetParam.iTxNum;
			iRxNum = g_ScreenSetParam.iRxNum;
		}			
	}else
	{
		//TxNum寄存器：0x02(Read Only)
		//RxNum寄存器：0x03(Read Only)		
		ReCode = ReadReg(0x02, &iTxNum);//Get Tx
		ReCode = ReadReg(0x03, &iRxNum);//Get Rx
		printk("Newly acquired TxNum:%d, RxNum:%d",iTxNum,iRxNum);		
	} 	

	if(ReCode != ERROR_CODE_OK)		
	{
		printk("ReCode  error. Error Code: %d\n", ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}

	iChannelNum = iTxNum + iRxNum;
	iMaxTx = iTxNum;
	iAllAdcDataNum = 1 + (1 + iTxNum + iRxNum)*2;//总通道数 + 对地校准数据 + 通道间校准数据 + Offset

//	ReCode = StartScan();
	/*
	if( ReCode != ERROR_CODE_OK )
	{
		printk("StartScan Failed!\n");
		btmpresult = false;
		goto TEST_ERR;
	}
	*/

	for( i = 0; i < 5; i++)
	{
		ReCode = StartScan();
		if( ReCode != ERROR_CODE_OK )
		{
			printk("StartScan Failed!\n");
			btmpresult = false;
			SysDelay(100);
		}
		else
		{
			printk("StartScan OK!\n");
			break;
		}
	}
	if(i >= 5)
	{
		printk("StartScan Failed for several times.!\n");
		btmpresult = false;
		goto TEST_ERR;
	}


//	iAdcData = new int[iAllAdcDataNum];
	iAdcData = kmalloc(iAllAdcDataNum*sizeof(int), GFP_ATOMIC);
	memset(iAdcData, 0, sizeof(iAdcData));
	for( i = 0; i < 1; i++)
	{
	//	ReCode = WeakShort_GetAdcData( iAllAdcDataNum*2, iAdcData, theDevice.m_cHidDev[m_NumDevice]->iCommMode );
		ReCode = WeakShort_GetAdcData( iAllAdcDataNum*2, iAdcData);		
		SysDelay(50);		
		if(ReCode != ERROR_CODE_OK)
		{
			btmpresult = false;
			goto TEST_ERR;
		}
	}

	printk("");
	
	iOffset = iAdcData[0];
	iClbData_Ground = iAdcData[1];
	iClbData_Mutual = iAdcData[2 + iChannelNum];

	//	show value.
#if 1

//	strAdc = "\r\n对地数据:\r\n";
//	strTemp = "\r\n\r\n通道间数据：\r\n";
	for(i = 0; i < iAllAdcDataNum/*iChannelNum*/; i++)
	{
		if(i <= (iChannelNum + 1))
		{
			if(i == 0)
				printk("\n\n\nOffset %02d: %4d,	\r\n", i, iAdcData[i]);
			else if(i == 1)/*if(i <= iMaxTx)*/
				printk("Ground %02d: %4d,	\r\n", i, iAdcData[i]);
			else if(i <= (iMaxTx + 1) )
				printk("Tx%02d: %4d,	", i-1, iAdcData[i]);
			else  if(i <= (iChannelNum + 1)	)
				printk("Rx%02d: %4d,	", i - iMaxTx-1, iAdcData[i]);

			if(i % 10 == 0)
				printk("\n");
		//	else
		//		strAdc += str;
		}else
		{
			if(i == (iChannelNum + 2)	)
				printk("\n\n\nMultual %02d: %4d,	\n", i, iAdcData[i]);
			else if(i <= (iMaxTx)+(iChannelNum + 2))
				printk("Tx%02d: %4d,	", i - (iChannelNum + 2), iAdcData[i]);
			else  if(i < iAllAdcDataNum)
				printk("Rx%02d: %4d,	", i - iMaxTx - (iChannelNum + 2), iAdcData[i]);

			if(i % 10 == 0)
				printk("\r\n");
		//	else
		//		strTemp += str;
		}
	}
	printk("\r\n");

#endif

	printk("\n");

	fMShortResistance = kmalloc(iChannelNum*sizeof(int),  GFP_ATOMIC);		//	fMShortResistance = new float[iChannelNum];
	fGShortResistance =  kmalloc(iChannelNum*sizeof(int),  GFP_ATOMIC);	//		fGShortResistance = new float[iChannelNum];

	iMin_CG = g_stCfg_FT5X22_BasicThreshold.WeakShortTest_CG;

	iDoffset = iOffset - 1024;
	iDrefn = iClbData_Ground;

	/*
	strMsg = "\r\n\r\nShort Circuit (Channel and Ground):\r\n";
	str.Format("Drefp=Offset - 1024:    %5d	\r\n",iDrefn);
	strMsg +=  str;
	str.Format("Doffset=Ground:  %5d	\r\n",iDoffset);
	strMsg +=  str;// + "Dsen: ";
	strDsen = "Dsen:";
	strRes = "\r\nRshort(Ground):";
	*/
//	printk("\r\n\r\nShort Circuit (Channel and Ground):\r\n");
	printk("Drefp:    %5d	\r\n",iDrefn);
//	strMsg +=  str;
	printk("Doffset:  %5d	\r\n",iDoffset);
//	strMsg +=  str;// + "Dsen: ";
//	strDsen = "Dsen:";
	printk("Rshort(Ground): \n\n\n");

	fKcal = 1; 	// 1.0;
	printk("Short Circuit (Channel and Ground):\r\n");

	for(i = 0; i < iChannelNum; i++)
	{
		iDsen = iAdcData[i+2];
	//	strTemp.Format("%5d	",iDsen);
		printk("%5d	",iDsen);
	//	strDsen +=  strTemp;
		if (i+1 == iMaxTx)
		{
		//	strDsen += "\r\n";
			printk("\n");
		}

		if((2047+iDoffset) - iDsen <= 0)
		{
			continue;
		}

		if (i == iMaxTx)
		{
		//	strRes += "\r\n";
			printk("\n");

		}

		if(IcValue <= 0x05 || IcValue == 0xff)
		{
		//	fGShortResistance[i] = (float)( iDsen - iDoffset + 410 ) * 25.1 * fKcal / ( 2047 + iDoffset - iDsen ) - 3;
			fGShortResistance[i] = ( iDsen - iDoffset + 410 ) * 25 * fKcal / ( 2047 + iDoffset - iDsen ) - 3;
		}else
		{
			if (iDrefn - iDsen <= 0)
			{
				fGShortResistance[i] = iMin_CG;
			//	strTemp.Format("%.02f  ",fGShortResistance[i]);
				printk("%.02d  ",fGShortResistance[i]);
			//	strRes +=  strTemp;
				continue;
			}
		//	fGShortResistance[i] = (float)(((float)(iDsen - iDoffset + 384) / (float)(iDrefn - iDsen) * 57) - 1.2);//( ( iDsen - iDoffset + 384 ) * iRsen / (/*temp*/iDrefn - iDsen) ) * fKcal - 1.2;
			fGShortResistance[i] = (((iDsen - iDoffset + 384) / (iDrefn - iDsen) * 57) - 1);//( ( iDsen - iDoffset + 384 ) * iRsen / (/*temp*/iDrefn - iDsen) ) * fKcal - 1.2;			
		}
		if(fGShortResistance[i] < 0) fGShortResistance[i] = 0;
	//	strTemp.Format("%.02f  ",fGShortResistance[i]);
		printk(".%02d  ",fGShortResistance[i]);
	//	strRes +=  strTemp;
		
		if((iMin_CG > fGShortResistance[i]) || (iDsen - iDoffset < 0))
		{
			iCount++;
			if(i+1 <= iMaxTx)
				printk("Tx%02d: .%02d (kΩ),	", i+1, fGShortResistance[i]);
			else
				printk("Rx%02d: .%02d (kΩ),	", i+1 - iMaxTx, fGShortResistance[i]);
			if(iCount % 10 == 0)
				printk("\n");
		}
		
	}
	printk("\n");


	//TestResultInfo(strMsg);
	if(iCount > 0)
	{
	//	TestResultInfo(strAdc);
		btmpresult = false;
	}

	iMin_CC = g_stCfg_FT5X22_BasicThreshold.WeakShortTest_CC;

	if ((IcValue == 0x06 || IcValue < 0xff) && iRsen != iCCRsen)
	{
		iRsen = iCCRsen;
	}
	iDoffset = iOffset - 1024;
	iDrefn = iClbData_Mutual;	
	fKcal = 1.0;
//	strAdc = "\r\n\r\nShort Circuit (Channel and Channel):\r\n";
	printk("\n\nShort Circuit (Channel and Channel):\n");
	iCount = 0;

	/*
	strMsg = "\r\n\r\nShort Circuit (Channel and Channel):\r\n";
	str.Format("Drefp=Mutual:    %5d	\r\n",iDrefn);
	strMsg +=  str;
	str.Format("Doffset=Offset - 1024:  %5d	\r\n",iDoffset);
	strMsg +=  str  + "Dsen:";
	strRes = "\r\nRshort(Channel):";
	*/
	printk("Drefp:    %5d	\r\n",iDrefn);
	printk("Doffset:  %5d	\r\n",iDoffset);
	printk("Rshort(Channel):");
	iDCal = max( iDrefn, 116 + iDoffset );

	for( i = 0; i < iChannelNum; i++)
	{
		iDsen = iAdcData[i+iChannelNum + 3];
	//	strTemp.Format("%5d	",iDsen);
	//	strMsg +=  strTemp;
		printk("%5d   ",iDsen);
		if (i+1 == iMaxTx)
		{
		//	strMsg += "\r\n";
			printk("\n");
		}
		if(IcValue <= 0x05 || IcValue == 0xff)
		{
			if(iDsen - iDrefn < 0)  continue;
		}		

		if (i == iMaxTx)
		{
		//	strRes += "\r\n";
			printk("\n");
		}


		if(IcValue <= 0x05 || IcValue == 0xff)
		{
			iMa = iDsen - iDCal;
			iMa = iMa ? iMa : 1;
			fMShortResistance[i] = ( ( 2047 + iDoffset - iDCal ) * 24 / iMa - 27 ) * fKcal - 6;
		}
		else
		{
			if (iDrefn - iDsen <= 0)
			{
				fMShortResistance[i] = iMin_CC;
			//	strTemp.Format("%.02f  ",fMShortResistance[i]);
			//	strRes +=  strTemp;
				printk(".%02d  ",fMShortResistance[i]);
				continue;
			}
		//	fMShortResistance[i] = (float)( iDsen - iDoffset - 123 ) * iRsen * fKcal / (iDrefn - iDsen /*temp*/ ) - 2;
			fMShortResistance[i] = ( iDsen - iDoffset - 123 ) * iRsen * fKcal / (iDrefn - iDsen /*temp*/ ) - 2;
		}

	//	strTemp.Format("%.02f  ",fMShortResistance[i]);
	//	strRes +=  strTemp;
		printk(".%02d  ",fMShortResistance[i]);

		if(fMShortResistance[i] < 0 && fMShortResistance[i] >= -240 ) fMShortResistance[i] = 0;
		else if( fMShortResistance[i] < -240 )  continue;
		
		if( fMShortResistance[i] <= 0  || fMShortResistance[i] < iMin_CC )
		{
			iCount++;
			if(i+1 <= iMaxTx)
				printk("Tx%02d: %.02d(kΩ),	", i+1, fMShortResistance[i]);		//	str.Format("Tx%02d: %.02f(kΩ),	", i+1, fMShortResistance[i]);
			else
				printk("Rx%02d: %.02d(kΩ),	", i+1 - iMaxTx, fMShortResistance[i]);		//	str.Format("Rx%02d: %.02f(kΩ),	", i+1 - iMaxTx, fMShortResistance[i]);

			if(iCount % 10 == 0)
				printk("\n");	//	strAdc += "\r\n" + str;
		//	else
			//	strAdc += str;
		}
		
	}
	printk("\n");


	//TestResultInfo(strRes);
	if(iCount > 0 && !bCapShortTest)
	{
	//	TestResultInfo(strAdc);
		btmpresult = false;
	}

	memset(fMShortResistance,0,sizeof(fMShortResistance));

	if (bCapShortTest && iCount)
	{
		printk(" bCapShortTest && iCount.  need to add ......");
	}

	//恢复v3屏体的mapping值
	if (m_bV3TP)
	{
		ReCode = WriteReg( REG_MAPPING_SWITCH, strSwitch );
		SysDelay(50);
		if( ReCode != ERROR_CODE_OK)	
		{
			printk("Failed to restore mapping type!\r\n ");
			btmpresult = false;
		} 		
		ReCode = GetChannelNum();
		if(ReCode != ERROR_CODE_OK)		
		{
			printk("\n GetChannelNum error. Error Code: %d",  ReCode);
			goto TEST_ERR;
		}
		
		ReCode = GetRawData();
	} 	

TEST_ERR:

	if(NULL != iAdcData)
	{
		kfree(iAdcData);
		iAdcData = NULL;
	}

	if(NULL != fMShortResistance)
	{
		kfree(fMShortResistance);
		fMShortResistance = NULL;
	}

	if(NULL != fGShortResistance)
	{
		kfree(fGShortResistance);
		fGShortResistance = NULL;
	}

	if( btmpresult )
	{
		printk("\r\n\r\n//Weak Short Test is OK.");
		* bTestResult = true;
	}
	else
	{
		printk("\r\n\r\n//Weak Short Test is NG.");
		* bTestResult = false;
	}

	printk("END. \n\n\n\n");
	return ReCode;
}


//unsigned char WeakShort_GetAdcData( int AllAdcDataLen, int *pRevBuffer, int iCommMode,unsigned char chCmd/* = 0x01*/ )
static unsigned char WeakShort_GetAdcData( int AllAdcDataLen, int *pRevBuffer  )
{
	unsigned char ReCode = ERROR_CODE_OK;
	int iReadDataLen = AllAdcDataLen;//Offset*2 + (ClbData + TxNum + RxNum)*2*2
	unsigned char *pDataSend = NULL;

	unsigned char Data = 0xff;
	int i = 0;

	printk("\n");

	pDataSend = kmalloc(iReadDataLen + 1, GFP_ATOMIC);	
	if(pDataSend == NULL)	return ERROR_CODE_ALLOCATE_BUFFER_ERROR;
	memset( pDataSend, 0, sizeof( iReadDataLen + 1 ) );

	ReCode = WriteReg(0x07, 0x01);
	if(ReCode != ERROR_CODE_OK)
	{
		printk("WriteReg error. \n");
		return ReCode;
	}

	SysDelay(100);
	
	for(i = 0; i < 100; i++)
	{
		SysDelay(10);
		ReCode = ReadReg(0x07, &Data);
		if(ReCode == ERROR_CODE_OK)
		{
			if(Data == 0)break;
		}
	}

	pDataSend[0] = 0xF4;

//	if( 0 == iCommMode )        ReCode = HY_IIC_IO(hDevice, pDataSend, 1, pDataSend + 1, iReadDataLen);
       ReCode = Comm_Base_IIC_IO(pDataSend, 1, pDataSend + 1, iReadDataLen);
	if(ReCode == ERROR_CODE_OK)
	{
		printk("\n");
		for(i = 0; i < iReadDataLen/2; i++)
		{
			pRevBuffer[i] = (pDataSend[1 + 2*i]<<8) + pDataSend[1 + 2*i + 1];
			printk("pRevBuffer[%d]:%d,    ", i, pRevBuffer[i]);
		}
		printk("\n");
	}
	else
	{
		printk("Comm_Base_IIC_IO error. error:%d. \n", ReCode);
	}

	if(pDataSend != NULL)
	{
		kfree(pDataSend);
		pDataSend = NULL;
	}

	printk(" END.\n");

	return ReCode;
}
#endif
unsigned char FT5X46_TestItem_PanelDifferTest(bool * bTestResult)
{
	int index = 0;
	int iRow = 0, iCol = 0;
	int iValue = 0;
	unsigned char ReCode = 0,strSwitch = -1;
	bool btmpresult = true;
	int iMax, iMin;	//, iAvg;
	int maxValue=0;
	int minValue=32767;
	int AvgValue = 0;
	int InvalidNum=0;
	int i = 0,  j = 0;
	
	unsigned char OriginRawDataType = 0xff;
	unsigned char OriginFrequecy = 0xff;
	unsigned char OriginFirState = 0xff;

	printk("[focal]");
	printk("\r\n\r\n\r\n==============================Test Item: -------- Panel Differ Test  \r\n\r\n");

	ReCode = EnterFactory(); 
	SysDelay(20);
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("\n\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
		goto TEST_ERR;
	}

	//先判断是否为v3屏体，然后读取0x54的值，并判断与设定的mapping类型是否一致，不一致写入数据
	//rawdata test mapping后，mapping前：0x54=1;mapping后：0x54=0;
	if (m_bV3TP)
	{
		ReCode = ReadReg( REG_MAPPING_SWITCH, &strSwitch );
		if(ReCode != ERROR_CODE_OK)		
		{
			printk("\n Read REG_MAPPING_SWITCH error. Error Code: %d",  ReCode);
			goto TEST_ERR;
		}
		
		if (strSwitch != 0)
		{
			ReCode = WriteReg( REG_MAPPING_SWITCH, 0 );
			if(ReCode != ERROR_CODE_OK)		
			{
				printk("\n Write REG_MAPPING_SWITCH error. Error Code: %d",  ReCode);
				goto TEST_ERR;
			}
			//	MUST get channel number again after write reg mapping switch.
			ReCode = GetChannelNum();
			if(ReCode != ERROR_CODE_OK)		
			{
				printk("\n GetChannelNum error. Error Code: %d",  ReCode);
				goto TEST_ERR;
			}
			
			ReCode = GetRawData();		
		}			
	}

	///////////
	printk("\r\n=========Set Auto Equalization:\r\n");
	ReCode = ReadReg( REG_NORMALIZE_TYPE, &OriginRawDataType );
	if(ReCode != ERROR_CODE_OK)		
	{
		printk("Read  REG_NORMALIZE_TYPE error. Error Code: %d",  ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}
	
	if (OriginRawDataType != 0)
	{
		ReCode =WriteReg( REG_NORMALIZE_TYPE, 0x00 );
		SysDelay(50);
		if( ReCode != ERROR_CODE_OK )
		{
			btmpresult = false;
			printk("Write reg REG_NORMALIZE_TYPE Failed." );
			goto TEST_ERR;
		}
	}

	printk("=========Set Frequecy High" );
	ReCode =ReadReg( 0x0A, &OriginFrequecy);
	if( ReCode != ERROR_CODE_OK )
	{
		printk("Read reg 0x0A error. Error Code: %d",  ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}

	ReCode = WriteReg( 0x0A, 0x81);
	SysDelay(10);
	if( ReCode != ERROR_CODE_OK )
	{
		btmpresult = false;
		printk("Write reg 0x0A Failed." );
		goto TEST_ERR;
	}

	printk("=========FIR State: OFF" );
	ReCode = ReadReg( 0xFB, &OriginFirState);
	if( ReCode != ERROR_CODE_OK )
	{
		printk("Read reg 0xFB error. Error Code: %d",  ReCode);
		btmpresult = false;
		goto TEST_ERR;
	}
	ReCode = WriteReg( 0xFB, 0);	
	SysDelay(50);
	if( ReCode != ERROR_CODE_OK )
	{
		printk("Write reg 0xFB Failed." );
		btmpresult = false;
		goto TEST_ERR;
	}
	ReCode = GetRawData();

	for ( index = 0; index < 4; ++index )
	{
		ReCode = GetRawData();
		if( ReCode != ERROR_CODE_OK )
		{
			printk("GetRawData Failed." );
			btmpresult = false;
			goto TEST_ERR;
		}
	}

	////Differ值为RawData的1/10
	for(i = 0; i < g_ScreenSetParam.iTxNum; i++)
	{
		for( j= 0; j < g_ScreenSetParam.iRxNum; j++)
		{
			m_DifferData[i][j] = m_RawData[i][j]/10;
		}
	}

	////////////////////////////////

	/*
	{
		NodeVal nodeOutRange;
		AnalyzeInfo info( m_iTxNum, m_iRxNum, true );
		bool bResult = AnalyzeTestResultMCap( m_DifferData, g_stCfg_MCap_DetailThreshold.PanelDifferTest_Min, g_stCfg_MCap_DetailThreshold.PanelDifferTest_Max, 
			g_stCfg_MCap_DetailThreshold.InvalidNode, info, textTemp, nodeOutRange );

		if( !bResult )
		{
			CString strHead = _T("\r\n//========= Out of Threshold in PannelDiffer Test: \r\n");
			btmpresult = false;
		}
	}
	*/
	////////////////////

	////////////////////////////////To show value
#if 1
	printk("PannelDiffer :\n");
	for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)
	{
		printk("\nRow%2d:    ", iRow+1);
		for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
		{
		//	if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node

			iValue = m_DifferData[iRow][iCol];
			printk("%4d,  ", iValue);
		}
		printk("\n" );
	}
	printk("\n" );
#endif


	////////////////////////////////To Determine  if in Range or not
	iMin =  g_stCfg_FT5X22_BasicThreshold.PanelDifferTest_Min;		//	g_stCfg_FT5X22_BasicThreshold.PanelDifferTest_Min[iRow][iCol];
	iMax = g_stCfg_FT5X22_BasicThreshold.PanelDifferTest_Max;

	for(iRow = 0; iRow<g_ScreenSetParam.iTxNum; iRow++)	//	iRow = 1 ???
	{
		for(iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++)
		{
			if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node

			iValue = m_DifferData[iRow][iCol];
			if(iValue < iMin || iValue > iMax)
			{
				btmpresult = false;
				printk("Out Of Range.  Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) \n", \
					iRow+1, iCol+1, iValue, iMin, iMax);						
			}
		}
	}
	///////////////////////////	end determine
	
	printk("PannelDiffer ABS:\n");
	for( i = 0; i <  g_ScreenSetParam.iTxNum; i++)
	{		
	//	strTemp += "\r\n";
		printk("\n");
		for( j = 0; j <  g_ScreenSetParam.iRxNum; j++)
		{	
		//	str.Format("%d,", abs(m_DifferData[i][j]));
		//	strTemp += str;
			printk("%ld,", abs(m_DifferData[i][j]));
			m_absDifferData[i][j] = abs(m_DifferData[i][j]); 

			if( NODE_AST_TYPE == g_stCfg_MCap_DetailThreshold.InvalidNode[i][j] || NODE_INVALID_TYPE == g_stCfg_MCap_DetailThreshold.InvalidNode[i][j])
			{
				InvalidNum++;
				continue;
			}
			maxValue = max(maxValue,m_DifferData[i][j]);
			minValue = min(minValue,m_DifferData[i][j]);
			AvgValue += m_DifferData[i][j];
		}
	}
	printk("\n");
//	SaveTestData(strTemp, m_iTxNum, m_iRxNum);
	Save_Test_Data(m_absDifferData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 1);		


	AvgValue = AvgValue/( g_ScreenSetParam.iTxNum*g_ScreenSetParam.iRxNum - InvalidNum); 
	printk("PanelDiffer:Max: %d, Min: %d, Avg: %d ", maxValue, minValue,AvgValue);

	ReCode = WriteReg( REG_NORMALIZE_TYPE, OriginRawDataType );
	ReCode = WriteReg( 0x0A, OriginFrequecy );
	ReCode = WriteReg( 0xFB, OriginFirState );
	
	//恢复v3屏体的mapping值
	if (m_bV3TP)
	{
		ReCode = WriteReg( REG_MAPPING_SWITCH, strSwitch );
		if( ReCode != ERROR_CODE_OK)	
		{
			printk("Failed to restore mapping type!");
			btmpresult = false;
		}
	}
	/////////////

	///////////////////////////-------------------------Result
	if( btmpresult )
	{
		*bTestResult = true;
		printk("		//Panel Differ Test is OK!\n");
	}
	else
	{
		* bTestResult = false;
		printk("		//Panel Differ Test is NG!\n");
	}
	return ReCode;

TEST_ERR:

	* bTestResult = false;
	printk("		//Panel Differ Test is NG!\n");
	return ReCode;
}
