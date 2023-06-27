/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

/** INCLUDES *******************************************************/
#include "system.h"
    
#ifdef  __VL53L_MASTER  //++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  //__VL53L_MASTER +++++++++++++++++++++++++++++++++++++++++++++++++++++++




#include <stdint.h>
#include <stddef.h>
#include "vl53l0x_api.h"
#include "vl53l0x_def.h"
//#include "vl53l_ST.h"
#include "vl53l0x_platform.h"
#include "vl53l0x_api_strings.h"
#include <string.h>
#include "vl53l0x_tuning.h"

#define VERSION_REQUIRED_MAJOR 1 ///< Required sensor major version
#define VERSION_REQUIRED_MINOR 0 ///< Required sensor minor version
#define VERSION_REQUIRED_BUILD 4 ///< Required sensor build
/** ProtoType *******************************************************/
//void VL53_init(void);
VL53L0X_Error rangingTest(VL53L0X_Dev_t *pMyDevice);
void vl53l0x_Single_test(void);


/** 変数　 *******************************************************/
VL53L0X_Dev_t   MyDevice;

//==============================================================================
//
//==============================================================================
void print_pal_error(VL53L0X_Error Status){
    char buf[VL53L0X_MAX_STRING_LENGTH];
    VL53L0X_GetPalErrorString(Status, buf);
    printf("API Status: %i : %s\r\n", Status, buf);
}
//==============================================================================
//
//==============================================================================

void print_range_status(VL53L0X_RangingMeasurementData_t* pRangingMeasurementData){
    char buf[VL53L0X_MAX_STRING_LENGTH];
    uint8_t RangeStatus;

    /*
     * New Range Status: data is valid when pRangingMeasurementData->RangeStatus = 0
     */

    RangeStatus = pRangingMeasurementData->RangeStatus;

    VL53L0X_GetRangeStatusString(RangeStatus, buf);
    printf("Range Status: %i : %s\r\n", RangeStatus, buf);

}

/********************************************************************
 * Function:        void VL53_init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
void VL53_init(void)
{
#ifdef  __VL53L_MASTER  //++++++++++++++++++++++++++++++++++++++++++++++++++++++

    VL53L0X_Error   Status = VL53L0X_ERROR_NONE;
    VL53L0X_Dev_t   *pMyDevice = &MyDevice;
    VL53L0X_Version_t                   Version;
    VL53L0X_Version_t                  *pVersion   = &Version;


    int32_t status_int;
    //int32_t init_done = 0;
    int NecleoComStatus = 0;
    int NecleoAutoCom = 1;
    //TCHAR SerialCommStr[MAX_VALUE_NAME];

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    if (argc == 1 ) {
	   printf("Nucleo Automatic detection selected!\r\n");	
	   printf("To Specify a COM use: %s <yourCOM> \r\n", argv[0]);
    } else if (argc == 2 ) {
	   printf("Nucleo Manual COM selected!\r\n");
	   strcpy(SerialCommStr,argv[1]);	   
	   printf("You have specified %s \r\n", SerialCommStr);
	   NecleoAutoCom = 0;
    } else {
	   printf("ERROR: wrong parameters !\r\n");	
           printf("USAGE : %s <yourCOM> \r\n", argv[0]);
	   return(1);
    }
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    printf ("VL53L0X PAL Continuous Ranging example\r\n");
//    printf ("Press a Key to continue!");
//    getchar();


#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    if (NecleoAutoCom == 1) {
    // Get the number of the COM attached to the Nucleo
    // Note that the following function will look for the 
    // Nucleo with name USBSER000 so be careful if you have 2 Nucleo 
    // inserted

    printf("Detect Nucleo Board ...");
    NecleoComStatus = GetNucleoSerialComm(SerialCommStr);
    
    if ((NecleoComStatus == 0) || (strcmp(SerialCommStr, "") == 0)) {
		    printf("Error Nucleo Board not Detected!\r\n");
	    return(1);
    }
    
	    printf("Nucleo Board detected on %s\r\n", SerialCommStr);
    }
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    // Initialize Comms
    pMyDevice->I2cDevAddr      = 0x52;
    pMyDevice->comms_type      =  1;
    pMyDevice->comms_speed_khz =  400;

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    Status = VL53L0X_i2c_init(SerialCommStr, 460800);
    if (Status != VL53L0X_ERROR_NONE) {
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;
        init_done = 1;
    } else
        printf ("Init Comms\r\n");
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    /*
     * Disable VL53L0X API logging if you want to run at full speed
     */
#ifdef VL53L0X_LOG_ENABLE
    VL53L0X_trace_config("test.log", TRACE_MODULE_ALL, TRACE_LEVEL_ALL, TRACE_FUNCTION_ALL);
#endif

    /*
     *  Get the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        status_int = VL53L0X_GetVersion(pVersion);
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    /*
     *  Verify the version of the VL53L0X API running in the firmrware
     */

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    if(Status == VL53L0X_ERROR_NONE)
    {
        if( pVersion->major != VERSION_REQUIRED_MAJOR ||
            pVersion->minor != VERSION_REQUIRED_MINOR ||
            pVersion->build != VERSION_REQUIRED_BUILD )
        {
            printf("VL53L0X API Version Error: Your firmware has %d.%d.%d (revision %d). This example requires %d.%d.%d.\r\n",
                pVersion->major, pVersion->minor, pVersion->build, pVersion->revision,
                VERSION_REQUIRED_MAJOR, VERSION_REQUIRED_MINOR, VERSION_REQUIRED_BUILD);
        }
    }
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    // End of implementation specific
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_DataInit\r\n");
        Status = VL53L0X_DataInit(&MyDevice); // Data initialization
        print_pal_error(Status);
    }

#endif  //__VL53L_MASTER +++++++++++++++++++++++++++++++++++++++++++++++++++++++

}

VL53L0X_Error WaitStopCompleted(VL53L0X_DEV Dev) {
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    uint32_t StopCompleted=0;
    uint32_t LoopNb;

    // Wait until it finished
    // use timeout to avoid deadlock
    if (Status == VL53L0X_ERROR_NONE) {
        LoopNb = 0;
        do {
            Status = VL53L0X_GetStopCompletedStatus(Dev, &StopCompleted);
            if ((StopCompleted == 0x00) || Status != VL53L0X_ERROR_NONE) {
                break;
            }
            LoopNb = LoopNb + 1;
            VL53L0X_PollingDelay(Dev);
        } while (LoopNb < VL53L0X_DEFAULT_MAX_LOOP);

        if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
            Status = VL53L0X_ERROR_TIME_OUT;
        }
	
    }

    return Status;
}
    

VL53L0X_Error WaitMeasurementDataReady(VL53L0X_DEV Dev) {
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    uint8_t NewDatReady=0;
    uint32_t LoopNb;

    vl53_LogDisp("WaitMeasurementDataReady() START", Status);
    // Wait until it finished
    // use timeout to avoid deadlock
    if (Status == VL53L0X_ERROR_NONE) {
        LoopNb = 0;
        do {
            Status = VL53L0X_GetMeasurementDataReady(Dev, &NewDatReady);
            if ((NewDatReady == 0x01) || Status != VL53L0X_ERROR_NONE) {
                break;
            }
            LoopNb = LoopNb + 1;
            VL53L0X_PollingDelay(Dev);
        } while (LoopNb < VL53L0X_DEFAULT_MAX_LOOP);

        if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
            Status = VL53L0X_ERROR_TIME_OUT;
        }
    }

    vl53_LogDisp("WaitMeasurementDataReady() END", Status);
    return Status;
}

void vl53_LogDisp(char *string,int8_t status)
{
#ifdef ___VL53_LOG_DISP 
    printf("%s ++++++++++++++++++++++++++++++ STATUS = %d\r\n",string, status);
#endif
}


VL53L0X_Error rangingTest(VL53L0X_Dev_t *pMyDevice)
{
    VL53L0X_RangingMeasurementData_t    RangingMeasurementData;
    VL53L0X_RangingMeasurementData_t   *pRangingMeasurementData    = &RangingMeasurementData;
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_StaticInit\r\n");
        Status = VL53L0X_StaticInit(pMyDevice); // Device Initialization
        // StaticInit will set interrupt by default
        print_pal_error(Status);
    }
    vl53_LogDisp("010 ",Status);
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefCalibration\r\n");
        Status = VL53L0X_PerformRefCalibration(pMyDevice,
        		&VhvSettings, &PhaseCal); // Device Initialization
        print_pal_error(Status);
    }
    vl53_LogDisp("011 ",Status);

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefSpadManagement\r\n");
        Status = VL53L0X_PerformRefSpadManagement(pMyDevice, &refSpadCount, &isApertureSpads); // Device Initialization
        print_pal_error(Status);
    }
    vl53_LogDisp("012 ",Status);

    if(Status == VL53L0X_ERROR_NONE)
    {

        printf ("Call of VL53L0X_SetDeviceMode\r\n");
        Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING); // Setup in single ranging mode
        print_pal_error(Status);
    }
    vl53_LogDisp("013 ",Status);
    
    if(Status == VL53L0X_ERROR_NONE)
    {
		printf ("Call of VL53L0X_StartMeasurement\r\n");
		Status = VL53L0X_StartMeasurement(pMyDevice);
		print_pal_error(Status);
    }
    vl53_LogDisp("014 ",Status);

    if(Status == VL53L0X_ERROR_NONE)
    {
        uint32_t measurement;
        uint32_t no_of_measurements = 32;

        uint16_t* pResults = (uint16_t*)malloc(sizeof(uint16_t) * no_of_measurements);

        for(measurement=0; measurement<no_of_measurements; measurement++)
        {

            Status = WaitMeasurementDataReady(pMyDevice);

            if(Status == VL53L0X_ERROR_NONE)
            {
                Status = VL53L0X_GetRangingMeasurementData(pMyDevice, pRangingMeasurementData);

                *(pResults + measurement) = pRangingMeasurementData->RangeMilliMeter;
                printf("In loop measurement %d: %d\r\n", measurement, pRangingMeasurementData->RangeMilliMeter);

                // Clear the interrupt
                VL53L0X_ClearInterruptMask(pMyDevice, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
                VL53L0X_PollingDelay(pMyDevice);
            } else {
                break;
            }
        }
        vl53_LogDisp("015 ",Status);

        if(Status == VL53L0X_ERROR_NONE)
        {
            for(measurement=0; measurement<no_of_measurements; measurement++)
            {
                printf("measurement %d: %d\r\n", measurement, *(pResults + measurement));
            }
        }

        free(pResults);
    }
    vl53_LogDisp("016 ",Status);

    
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_StopMeasurement\r\n");
        Status = VL53L0X_StopMeasurement(pMyDevice);
    }
    vl53_LogDisp("017 ",Status);

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Wait Stop to be competed\r\n");
        Status = WaitStopCompleted(pMyDevice);
    }
    vl53_LogDisp("018 ",Status);

    if(Status == VL53L0X_ERROR_NONE)
	Status = VL53L0X_ClearInterruptMask(pMyDevice,
		VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);

    return Status;
}


void vl53l0x_test(void)
{
    #ifdef  __VL53L_MASTER  //++++++++++++++++++++++++++++++++++++++++++++++++++++++

    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    //VL53L0X_Dev_t MyDevice;
    VL53L0X_Dev_t *pMyDevice = &MyDevice;
    VL53L0X_Version_t                   Version;
    VL53L0X_Version_t                  *pVersion   = &Version;
    VL53L0X_DeviceInfo_t                DeviceInfo;
    



    int32_t status_int;
    //int32_t init_done = 0;
    int NecleoComStatus = 0;
    int NecleoAutoCom = 1;
    //TCHAR SerialCommStr[MAX_VALUE_NAME];

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    if (argc == 1 ) {
	   printf("Nucleo Automatic detection selected!\r\n");	
	   printf("To Specify a COM use: %s <yourCOM> \r\n", argv[0]);
    } else if (argc == 2 ) {
	   printf("Nucleo Manual COM selected!\r\n");
	   strcpy(SerialCommStr,argv[1]);	   
	   printf("You have specified %s \r\n", SerialCommStr);
	   NecleoAutoCom = 0;
    } else {
	   printf("ERROR: wrong parameters !\r\n");	
           printf("USAGE : %s <yourCOM> \r\n", argv[0]);
	   return(1);
    }
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    printf ("VL53L0X PAL Continuous Ranging example\r\n");
//    printf ("Press a Key to continue!");
//    getchar();


#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    if (NecleoAutoCom == 1) {
    // Get the number of the COM attached to the Nucleo
    // Note that the following function will look for the 
    // Nucleo with name USBSER000 so be careful if you have 2 Nucleo 
    // inserted

    printf("Detect Nucleo Board ...");
    NecleoComStatus = GetNucleoSerialComm(SerialCommStr);
    
    if ((NecleoComStatus == 0) || (strcmp(SerialCommStr, "") == 0)) {
		    printf("Error Nucleo Board not Detected!\r\n");
	    return(1);
    }
    
	    printf("Nucleo Board detected on %s\r\n", SerialCommStr);
    }
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    // Initialize Comms
    pMyDevice->I2cDevAddr      = 0x52;
    pMyDevice->comms_type      =  1;
    pMyDevice->comms_speed_khz =  400;

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    Status = VL53L0X_i2c_init(SerialCommStr, 460800);
    if (Status != VL53L0X_ERROR_NONE) {
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;
        init_done = 1;
    } else
        printf ("Init Comms\r\n");
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    /*
     * Disable VL53L0X API logging if you want to run at full speed
     */
#ifdef VL53L0X_LOG_ENABLE
    VL53L0X_trace_config("test.log", TRACE_MODULE_ALL, TRACE_LEVEL_ALL, TRACE_FUNCTION_ALL);
#endif

    /*
     *  Get the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        status_int = VL53L0X_GetVersion(pVersion);
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    /*
     *  Verify the version of the VL53L0X API running in the firmrware
     */

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    if(Status == VL53L0X_ERROR_NONE)
    {
        if( pVersion->major != VERSION_REQUIRED_MAJOR ||
            pVersion->minor != VERSION_REQUIRED_MINOR ||
            pVersion->build != VERSION_REQUIRED_BUILD )
        {
            printf("VL53L0X API Version Error: Your firmware has %d.%d.%d (revision %d). This example requires %d.%d.%d.\r\n",
                pVersion->major, pVersion->minor, pVersion->build, pVersion->revision,
                VERSION_REQUIRED_MAJOR, VERSION_REQUIRED_MINOR, VERSION_REQUIRED_BUILD);
        }
    }

    // End of implementation specific
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_DataInit\r\n");
        Status = VL53L0X_DataInit(&MyDevice); // Data initialization
        print_pal_error(Status);
    }

#endif  //__VL53L_MASTER +++++++++++++++++++++++++++++++++++++++++++++++++++++++

    
    
    printf("\r\n");
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = VL53L0X_GetDeviceInfo(&MyDevice, &DeviceInfo);
        vl53_LogDisp("VL53L0X_GetDeviceInfo()", Status);
    }
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf("VL53L0X_GetDeviceInfo:\r\n");
        printf("Device Name : %s\r\n", DeviceInfo.Name);
        printf("Device Type : %s\r\n", DeviceInfo.Type);
        printf("Device ID : %s\r\n", DeviceInfo.ProductId);
        printf("ProductRevisionMajor : %d\r\n", DeviceInfo.ProductRevisionMajor);
        printf("ProductRevisionMinor : %d\r\n", DeviceInfo.ProductRevisionMinor);

        if ((DeviceInfo.ProductRevisionMinor != 1) && (DeviceInfo.ProductRevisionMinor != 1)) {
        	printf("Error expected cut 1.1 but found cut %d.%d\r\n",
        			DeviceInfo.ProductRevisionMajor, DeviceInfo.ProductRevisionMinor);
        	Status = VL53L0X_ERROR_NOT_SUPPORTED;
        }
    }
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = rangingTest(pMyDevice);
    }

    print_pal_error(Status);
    
    // Implementation specific

    /*
     *  Disconnect comms - part of VL53L0X_platform.c
     */
#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    if(init_done == 0)
    {
        printf ("Close Comms\r\n");
        status_int = VL53L0X_comms_close();
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    print_pal_error(Status);
    // End of implementation specific
    
//    printf ("Press a Key to continue!");
//    getchar();
    

} 

VL53L0X_Error ShinglerangingTest(VL53L0X_Dev_t *pMyDevice)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    VL53L0X_RangingMeasurementData_t    RangingMeasurementData;
    int i;
    FixPoint1616_t LimitCheckCurrent;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;

    
    
    
    
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_StaticInit\r\n");
        Status = VL53L0X_StaticInit(pMyDevice); // Device Initialization
        print_pal_error(Status);
    }
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefCalibration\r\n");
        Status = VL53L0X_PerformRefCalibration(pMyDevice,
        		&VhvSettings, &PhaseCal); // Device Initialization
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefSpadManagement\r\n");
        Status = VL53L0X_PerformRefSpadManagement(pMyDevice,
        		&refSpadCount, &isApertureSpads); // Device Initialization
        printf ("refSpadCount = %d, isApertureSpads = %d\r\n", refSpadCount, isApertureSpads);
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {

        // no need to do this when we use VL53L0X_PerformSingleRangingMeasurement
        printf ("Call of VL53L0X_SetDeviceMode\r\n");
        Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
        print_pal_error(Status);
    }

    // Enable/Disable Sigma and Signal check
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    }
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }

    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
    }

    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckValue(pMyDevice,
        		VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
        		(FixPoint1616_t)(1.5*0.023*65536));
    }


    /*
     *  Step  4 : Test ranging mode
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        for(i=0;i<10;i++){
            printf ("Call of VL53L0X_PerformSingleRangingMeasurement\r\n");
            Status = VL53L0X_PerformSingleRangingMeasurement(pMyDevice, &RangingMeasurementData);

            print_pal_error(Status);
            print_range_status(&RangingMeasurementData);

            VL53L0X_GetLimitCheckCurrent(pMyDevice, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, &LimitCheckCurrent);

            printf("RANGE IGNORE THRESHOLD: %f\r\n", (float)LimitCheckCurrent/65536.0);


            if (Status != VL53L0X_ERROR_NONE) break;

            printf("Measured distance: %i\r\n", RangingMeasurementData.RangeMilliMeter);


        }
    }
    return Status;
}

void vl53l0x_Single_test(void)
{
    //int32_t init_done = 0;

    
        VL53L0X_Error Status = VL53L0X_ERROR_NONE;
//    VL53L0X_Dev_t MyDevice;
    VL53L0X_Dev_t *pMyDevice = &MyDevice;
    VL53L0X_Version_t                   Version;
    VL53L0X_Version_t                  *pVersion   = &Version;
    VL53L0X_DeviceInfo_t                DeviceInfo;

    int32_t status_int;
    int32_t init_done = 0;
    int NecleoComStatus = 0;
    int NecleoAutoCom = 1;
    //TCHAR SerialCommStr[MAX_VALUE_NAME];

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    if (argc == 1 ) {
	   printf("Nucleo Automatic detection selected!\r\n");	
	   printf("To Specify a COM use: %s <yourCOM> \n", argv[0]);
    } else if (argc == 2 ) {
	   printf("Nucleo Manual COM selected!\r\n");
	   strcpy(SerialCommStr,argv[1]);	   
	   printf("You have specified %s \n", SerialCommStr);
	   NecleoAutoCom = 0;
    } else {
	   printf("ERROR: wrong parameters !\r\n");	
           printf("USAGE : %s <yourCOM> \n", argv[0]);
	   return(1);
    }
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    
    
    printf ("VL53L0X API Simple Ranging example\r\n\n");
//    printf ("Press a Key to continue!\r\n\n");
//    getchar();

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    if (NecleoAutoCom == 1) {
	    // Get the number of the COM attached to the Nucleo
	    // Note that the following function will look for the 
	    // Nucleo with name USBSER000 so be careful if you have 2 Nucleo 
	    // inserted

	    printf("Detect Nucleo Board ...");
	    NecleoComStatus = GetNucleoSerialComm(SerialCommStr);
	    
	    if ((NecleoComStatus == 0) || (strcmp(SerialCommStr, "") == 0)) {
		    printf("Error Nucleo Board not Detected!\r\n");
		    return(1);
	    }
	    
	    printf("Nucleo Board detected on %s\r\n\n", SerialCommStr);
    }
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
    
    // Initialize Comms
    pMyDevice->I2cDevAddr      = 0x52;
    pMyDevice->comms_type      =  1;
    pMyDevice->comms_speed_khz =  400;

#ifdef ___NOP   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    Status = VL53L0X_i2c_init(SerialCommStr, 460800);
    if (Status != VL53L0X_ERROR_NONE) {
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;
        init_done = 1;
    } else
        printf ("Init Comms\r\n");
#endif // ___NOP  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    

    /*
     * Disable VL53L0X API logging if you want to run at full speed
     */
#ifdef VL53L0X_LOG_ENABLE
    VL53L0X_trace_config("test.log", TRACE_MODULE_ALL, TRACE_LEVEL_ALL, TRACE_FUNCTION_ALL);
#endif

    /*
     *  Get the version of the VL53L0X API running in the firmware
     */

    vl53_LogDisp("_Single_test() 01",Status);
    if(Status == VL53L0X_ERROR_NONE)
    {
        status_int = VL53L0X_GetVersion(pVersion);
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }
    vl53_LogDisp("_Single_test() 02",Status);

    /*
     *  Verify the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        if( pVersion->major != VERSION_REQUIRED_MAJOR ||
            pVersion->minor != VERSION_REQUIRED_MINOR ||
            pVersion->build != VERSION_REQUIRED_BUILD )
        {
            printf("VL53L0X API Version Error:\r\n Your firmware has %d.%d.%d (revision %x). This example requires %d.%d.%d.\r\n",
                pVersion->major, pVersion->minor, pVersion->build, pVersion->revision,
                VERSION_REQUIRED_MAJOR, VERSION_REQUIRED_MINOR, VERSION_REQUIRED_BUILD);
        }
    }


    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_DataInit\r\n");
        Status = VL53L0X_DataInit(&MyDevice); // Data initialization
        print_pal_error(Status);
    }
    
    vl53_LogDisp("_Single_test() 03",Status);

    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = VL53L0X_GetDeviceInfo(&MyDevice, &DeviceInfo);
    
        if(Status == VL53L0X_ERROR_NONE)
        {
            printf("VL53L0X_GetDeviceInfo:\r\n");
            printf("Device Name : %s\r\n", DeviceInfo.Name);
            printf("Device Type : %s\r\n", DeviceInfo.Type);
            printf("Device ID : %s\r\n", DeviceInfo.ProductId);
            printf("ProductRevisionMajor : %d\r\n", DeviceInfo.ProductRevisionMajor);
            printf("ProductRevisionMinor : %d\r\n", DeviceInfo.ProductRevisionMinor);

        if ((DeviceInfo.ProductRevisionMinor != 1) && (DeviceInfo.ProductRevisionMinor != 1)) {
            printf("Error expected cut 1.1 but found cut %d.%d\r\n\n", DeviceInfo.ProductRevisionMajor, DeviceInfo.ProductRevisionMinor);
                Status = VL53L0X_ERROR_NOT_SUPPORTED;
                vl53_LogDisp("_Single_test() 03-1",Status);
            }
        }
        print_pal_error(Status);
        vl53_LogDisp("_Single_test() 04",Status);
    }


    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = ShinglerangingTest(pMyDevice);
    }
    vl53_LogDisp("_Single_test() 05",Status);

    print_pal_error(Status);
    
    // Implementation specific

    /*
     *  Disconnect comms - part of VL53L0X_platform.c
     */

#ifdef ___NOP
    if(init_done == 0)
    {
        printf ("Close Comms\r\n");
        status_int = VL53L0X_comms_close();
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }
#endif  // ___NOP

    print_pal_error(Status);
	
//    printf ("\nPress a Key to continue!");
//    getchar();
    
}

VL53L0X_Error HArangingTest(VL53L0X_Dev_t *pMyDevice)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    VL53L0X_RangingMeasurementData_t    RangingMeasurementData;
    int i;
    FixPoint1616_t LimitCheckCurrent;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_StaticInit\r\n");
        Status = VL53L0X_StaticInit(pMyDevice); // Device Initialization
        print_pal_error(Status);
    }
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefCalibration\r\n");
        Status = VL53L0X_PerformRefCalibration(pMyDevice,
        		&VhvSettings, &PhaseCal); // Device Initialization
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE) // needed if a coverglass is used and no calibration has been performed
    {
        printf ("Call of VL53L0X_PerformRefSpadManagement\r\n");
        Status = VL53L0X_PerformRefSpadManagement(pMyDevice,
        		&refSpadCount, &isApertureSpads); // Device Initialization
        printf ("refSpadCount = %d, isApertureSpads = %d\r\n", refSpadCount, isApertureSpads);
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {

        // no need to do this when we use VL53L0X_PerformSingleRangingMeasurement
        printf ("Call of VL53L0X_SetDeviceMode\r\n");
        Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
        print_pal_error(Status);
    }

    // Enable/Disable Sigma and Signal check
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    }
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }
				
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckValue(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
        		(FixPoint1616_t)(0.25*65536));
	}			
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckValue(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
        		(FixPoint1616_t)(18*65536));			
    }
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pMyDevice,
        		200000);
    }
    /*
     *  Step  4 : Test ranging mode
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        for(i=0;i<10;i++){
            printf ("Call of VL53L0X_PerformSingleRangingMeasurement\r\n");
            Status = VL53L0X_PerformSingleRangingMeasurement(pMyDevice,
            		&RangingMeasurementData);

            print_pal_error(Status);
            print_range_status(&RangingMeasurementData);

            VL53L0X_GetLimitCheckCurrent(pMyDevice,
            		VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, &LimitCheckCurrent);

            printf("RANGE IGNORE THRESHOLD: %f\r\n", (float)LimitCheckCurrent/65536.0);


            if (Status != VL53L0X_ERROR_NONE) break;

            printf("Measured distance: %i\r\n", RangingMeasurementData.RangeMilliMeter);


        }
    }
    return Status;
}

void vl53l0x_SingleHA_test(void)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
//    VL53L0X_Dev_t MyDevice;
    VL53L0X_Dev_t *pMyDevice = &MyDevice;
    VL53L0X_Version_t                   Version;
    VL53L0X_Version_t                  *pVersion   = &Version;
    VL53L0X_DeviceInfo_t                DeviceInfo;

    int32_t status_int;
    int32_t init_done = 0;
    int NecleoComStatus = 0;
    int NecleoAutoCom = 1;
//    TCHAR SerialCommStr[MAX_VALUE_NAME];

#ifdef ___NOP    
    if (argc == 1 ) {
	   printf("Nucleo Automatic detection selected!\n");	
	   printf("To Specify a COM use: %s <yourCOM> \n", argv[0]);
    } else if (argc == 2 ) {
	   printf("Nucleo Manual COM selected!\n");
	   strcpy(SerialCommStr,argv[1]);	   
	   printf("You have specified %s \n", SerialCommStr);
	   NecleoAutoCom = 0;
    } else {
	   printf("ERROR: wrong parameters !\n");	
           printf("USAGE : %s <yourCOM> \n", argv[0]);
	   return(1);
    }
#endif    

    
    
    printf ("VL53L0X API Simple Ranging example\r\n\n");
//    printf ("Press a Key to continue!\n\n");
//    getchar();

#ifdef ___NOP    

    if (NecleoAutoCom == 1) {
    // Get the number of the COM attached to the Nucleo
    // Note that the following function will look for the 
    // Nucleo with name USBSER000 so be careful if you have 2 Nucleo 
    // inserted

    printf("Detect Nucleo Board ...");
    NecleoComStatus = GetNucleoSerialComm(SerialCommStr);
    
    if ((NecleoComStatus == 0) || (strcmp(SerialCommStr, "") == 0)) {
		    printf("Error Nucleo Board not Detected!\n");
	    return(1);
    }
    
	    printf("Nucleo Board detected on %s\n\n", SerialCommStr);
    }
#endif    

    // Initialize Comms
    pMyDevice->I2cDevAddr      = 0x52;
    pMyDevice->comms_type      =  1;
    pMyDevice->comms_speed_khz =  400;

#ifdef ___NOP    
    Status = VL53L0X_i2c_init(SerialCommStr, 460800);
    if (Status != VL53L0X_ERROR_NONE) {
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;
        init_done = 1;
    } else
        printf ("Init Comms\n");
#endif    

    /*
     * Disable VL53L0X API logging if you want to run at full speed
     */
#ifdef VL53L0X_LOG_ENABLE
    VL53L0X_trace_config("test.log", TRACE_MODULE_ALL, TRACE_LEVEL_ALL, TRACE_FUNCTION_ALL);
#endif

    /*
     *  Get the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        status_int = VL53L0X_GetVersion(pVersion);
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    /*
     *  Verify the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        if( pVersion->major != VERSION_REQUIRED_MAJOR ||
            pVersion->minor != VERSION_REQUIRED_MINOR ||
            pVersion->build != VERSION_REQUIRED_BUILD )
        {
            printf("VL53L0X API Version Error:\r\n Your firmware has %d.%d.%d (revision %d). This example requires %d.%d.%d.\r\n",
                pVersion->major, pVersion->minor, pVersion->build, pVersion->revision,
                VERSION_REQUIRED_MAJOR, VERSION_REQUIRED_MINOR, VERSION_REQUIRED_BUILD);
        }
    }


    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_DataInit\r\n");
        Status = VL53L0X_DataInit(&MyDevice); // Data initialization
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = VL53L0X_GetDeviceInfo(&MyDevice, &DeviceInfo);
        
    
        if(Status == VL53L0X_ERROR_NONE)
        {
            printf("VL53L0X_GetDeviceInfo:\r\n");
            printf("Device Name : %s\r\n", DeviceInfo.Name);
            printf("Device Type : %s\r\n", DeviceInfo.Type);
            printf("Device ID : %s\r\n", DeviceInfo.ProductId);
            printf("ProductRevisionMajor : %d\r\n", DeviceInfo.ProductRevisionMajor);
        printf("ProductRevisionMinor : %d\r\n", DeviceInfo.ProductRevisionMinor);

        if ((DeviceInfo.ProductRevisionMinor != 1) && (DeviceInfo.ProductRevisionMinor != 1)) {
            printf("Error expected cut 1.1 but found cut %d.%d\r\n",
                       DeviceInfo.ProductRevisionMajor, DeviceInfo.ProductRevisionMinor);
                Status = VL53L0X_ERROR_NOT_SUPPORTED;
            }
        }
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = HArangingTest(pMyDevice);
    }

    print_pal_error(Status);
    
    // Implementation specific

    /*
     *  Disconnect comms - part of VL53L0X_platform.c
     */

#ifdef ___NOP
    if(init_done == 0)
    {
        printf ("Close Comms\r\n");
        status_int = VL53L0X_comms_close();
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }
#endif
    print_pal_error(Status);
	
//    printf ("\nPress a Key to continue!");
//    getchar();
    
}


VL53L0X_Error HSrangingTest(VL53L0X_Dev_t *pMyDevice)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    VL53L0X_RangingMeasurementData_t    RangingMeasurementData;
    int i;
    FixPoint1616_t LimitCheckCurrent;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_StaticInit\r\n");
        Status = VL53L0X_StaticInit(pMyDevice); // Device Initialization
        print_pal_error(Status);
    }
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefCalibration\r\n");
        Status = VL53L0X_PerformRefCalibration(pMyDevice,
        		&VhvSettings, &PhaseCal); // Device Initialization
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefSpadManagement\r\n");
        Status = VL53L0X_PerformRefSpadManagement(pMyDevice,
        		&refSpadCount, &isApertureSpads); // Device Initialization
        printf ("refSpadCount = %d, isApertureSpads = %d\r\n", refSpadCount, isApertureSpads);
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {

        // no need to do this when we use VL53L0X_PerformSingleRangingMeasurement
        printf ("Call of VL53L0X_SetDeviceMode\r\n");
        Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
        print_pal_error(Status);
    }

    // Enable/Disable Sigma and Signal check
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    }
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }

    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckValue(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
        		(FixPoint1616_t)(0.25*65536));
	}			
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckValue(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
        		(FixPoint1616_t)(32*65536));			
    }
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pMyDevice,
        		30000);
    }	
	

    /*
     *  Step  4 : Test ranging mode
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        for(i=0;i<10;i++){
            printf ("Call of VL53L0X_PerformSingleRangingMeasurement\r\n");
            Status = VL53L0X_PerformSingleRangingMeasurement(pMyDevice,
            		&RangingMeasurementData);

            print_pal_error(Status);
            print_range_status(&RangingMeasurementData);

            VL53L0X_GetLimitCheckCurrent(pMyDevice,
            		VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, &LimitCheckCurrent);

            printf("RANGE IGNORE THRESHOLD: %f\r\n", (float)LimitCheckCurrent/65536.0);


            if (Status != VL53L0X_ERROR_NONE) break;

            printf("Measured distance: %i\r\n", RangingMeasurementData.RangeMilliMeter);


        }
    }
    return Status;
}

void vl53l0x_SingleHS_test(void)
{

    

        VL53L0X_Error Status = VL53L0X_ERROR_NONE;
//    VL53L0X_Dev_t MyDevice;
    VL53L0X_Dev_t *pMyDevice = &MyDevice;
    VL53L0X_Version_t                   Version;
    VL53L0X_Version_t                  *pVersion   = &Version;
    VL53L0X_DeviceInfo_t                DeviceInfo;

    int32_t status_int;
    int32_t init_done = 0;
    int NecleoComStatus = 0;
    int NecleoAutoCom = 1;

#ifdef ___NOP
    TCHAR SerialCommStr[MAX_VALUE_NAME];

    if (argc == 1 ) {
	   printf("Nucleo Automatic detection selected!\n");	
	   printf("To Specify a COM use: %s <yourCOM> \n", argv[0]);
    } else if (argc == 2 ) {
	   printf("Nucleo Manual COM selected!\n");
	   strcpy(SerialCommStr,argv[1]);	   
	   printf("You have specified %s \n", SerialCommStr);
	   NecleoAutoCom = 0;
    } else {
	   printf("ERROR: wrong parameters !\n");	
           printf("USAGE : %s <yourCOM> \n", argv[0]);
	   return(1);
    }

#endif
    
    
    printf ("VL53L0X API Simple Ranging example\r\n\n");
//    printf ("Press a Key to continue!\n\n");
//    getchar();
#ifdef ___NOP

    if (NecleoAutoCom == 1) {
    // Get the number of the COM attached to the Nucleo
    // Note that the following function will look for the 
    // Nucleo with name USBSER000 so be careful if you have 2 Nucleo 
    // inserted

    printf("Detect Nucleo Board ...");
    NecleoComStatus = GetNucleoSerialComm(SerialCommStr);
    
    if ((NecleoComStatus == 0) || (strcmp(SerialCommStr, "") == 0)) {
		    printf("Error Nucleo Board not Detected!\n");
	    return(1);
    }
    
	    printf("Nucleo Board detected on %s\n\n", SerialCommStr);
    }
#endif

    // Initialize Comms
    pMyDevice->I2cDevAddr      = 0x52;
    pMyDevice->comms_type      =  1;
    pMyDevice->comms_speed_khz =  400;

#ifdef ___NOP
    Status = VL53L0X_i2c_init(SerialCommStr, 460800);
    if (Status != VL53L0X_ERROR_NONE) {
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;
        init_done = 1;
    } else
        printf ("Init Comms\n");
#endif

    /*
     * Disable VL53L0X API logging if you want to run at full speed
     */
#ifdef VL53L0X_LOG_ENABLE
    VL53L0X_trace_config("test.log", TRACE_MODULE_ALL, TRACE_LEVEL_ALL, TRACE_FUNCTION_ALL);
#endif


    /*
     *  Get the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        status_int = VL53L0X_GetVersion(pVersion);
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    /*
     *  Verify the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        if( pVersion->major != VERSION_REQUIRED_MAJOR ||
            pVersion->minor != VERSION_REQUIRED_MINOR ||
            pVersion->build != VERSION_REQUIRED_BUILD )
        {
            printf("VL53L0X API Version Error:\r\n Your firmware has %d.%d.%d (revision %d). This example requires %d.%d.%d.\r\n",
                pVersion->major, pVersion->minor, pVersion->build, pVersion->revision,
                VERSION_REQUIRED_MAJOR, VERSION_REQUIRED_MINOR, VERSION_REQUIRED_BUILD);
        }
    }


    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_DataInit\r\n");
        Status = VL53L0X_DataInit(&MyDevice); // Data initialization
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = VL53L0X_GetDeviceInfo(&MyDevice, &DeviceInfo);
        
    
        if(Status == VL53L0X_ERROR_NONE)
        {
            printf("VL53L0X_GetDeviceInfo:\r\n");
            printf("Device Name : %s\r\n", DeviceInfo.Name);
            printf("Device Type : %s\r\n", DeviceInfo.Type);
            printf("Device ID : %s\r\n", DeviceInfo.ProductId);
            printf("ProductRevisionMajor : %d\r\n", DeviceInfo.ProductRevisionMajor);
        printf("ProductRevisionMinor : %d\r\n", DeviceInfo.ProductRevisionMinor);

        if ((DeviceInfo.ProductRevisionMinor != 1) && (DeviceInfo.ProductRevisionMinor != 1)) {
            printf("Error expected cut 1.1 but found cut %d.%d\r\n",
                       DeviceInfo.ProductRevisionMajor, DeviceInfo.ProductRevisionMinor);
                Status = VL53L0X_ERROR_NOT_SUPPORTED;
            }
        }
        print_pal_error(Status);
    }
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = HSrangingTest(pMyDevice);
    }

    print_pal_error(Status);
    
    // Implementation specific

    /*
     *  Disconnect comms - part of VL53L0X_platform.c
     */

#ifdef ___NOP
    if(init_done == 0)
    {
        printf ("Close Comms\r\n");
        status_int = VL53L0X_comms_close();
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }
#endif

    print_pal_error(Status);
	
//    printf ("\nPress a Key to continue!");
//    getchar();
    
    
    
}

VL53L0X_Error LRrangingTest(VL53L0X_Dev_t *pMyDevice)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    VL53L0X_RangingMeasurementData_t    RangingMeasurementData;
    int i;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_StaticInit\r\n");
        Status = VL53L0X_StaticInit(pMyDevice); // Device Initialization
        print_pal_error(Status);
    }
    
    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefCalibration\r\n");
        Status = VL53L0X_PerformRefCalibration(pMyDevice,
        		&VhvSettings, &PhaseCal); // Device Initialization
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_PerformRefSpadManagement\r\n");
        Status = VL53L0X_PerformRefSpadManagement(pMyDevice,
        		&refSpadCount, &isApertureSpads); // Device Initialization
        printf ("refSpadCount = %d, isApertureSpads = %d\r\n", refSpadCount, isApertureSpads);
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {

        // no need to do this when we use VL53L0X_PerformSingleRangingMeasurement
        printf ("Call of VL53L0X_SetDeviceMode\r\n");
        Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
        print_pal_error(Status);
    }

    // Enable/Disable Sigma and Signal check
	
 /*   if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetSequenceStepEnable(pMyDevice,VL53L0X_SEQUENCESTEP_DSS, 1);
    }*/	
	
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    }
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }
				
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckValue(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
        		(FixPoint1616_t)(0.1*65536));
	}			
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckValue(pMyDevice,
        		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
        		(FixPoint1616_t)(60*65536));			
    }
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pMyDevice,
        		33000);
	}
	
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetVcselPulsePeriod(pMyDevice, 
		        VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
    }
    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetVcselPulsePeriod(pMyDevice, 
		        VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
    }


    /*
     *  Step  4 : Test ranging mode
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        for(i=0;i<50;i++){
            printf ("Call of VL53L0X_PerformSingleRangingMeasurement\r\n");
            Status = VL53L0X_PerformSingleRangingMeasurement(pMyDevice,
            		&RangingMeasurementData);

            print_pal_error(Status);
            print_range_status(&RangingMeasurementData);

           
            if (Status != VL53L0X_ERROR_NONE) break;

            printf("Measured distance: %i\r\n", RangingMeasurementData.RangeMilliMeter);


        }
    }
    return Status;
}
void vl53l0x_SingleLR_test(void)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
//    VL53L0X_Dev_t MyDevice;
    VL53L0X_Dev_t *pMyDevice = &MyDevice;
    VL53L0X_Version_t                   Version;
    VL53L0X_Version_t                  *pVersion   = &Version;
    VL53L0X_DeviceInfo_t                DeviceInfo;

    int32_t status_int;
    int32_t init_done = 0;
    int NecleoComStatus = 0;
    int NecleoAutoCom = 1;
 
#ifdef ___NOP    
    TCHAR SerialCommStr[MAX_VALUE_NAME];

    if (argc == 1 ) {
	   printf("Nucleo Automatic detection selected!\n");	
	   printf("To Specify a COM use: %s <yourCOM> \n", argv[0]);
    } else if (argc == 2 ) {
	   printf("Nucleo Manual COM selected!\n");
	   strcpy(SerialCommStr,argv[1]);	   
	   printf("You have specified %s \n", SerialCommStr);
	   NecleoAutoCom = 0;
    } else {
	   printf("ERROR: wrong parameters !\n");	
           printf("USAGE : %s <yourCOM> \n", argv[0]);
	   return(1);
    }
#endif// ___NOP    


    
    
    printf ("VL53L0X API Simple Ranging example\n\n");
//    printf ("Press a Key to continue!\n\n");
//    getchar();

#ifdef ___NOP    
    if (NecleoAutoCom == 1) {
    // Get the number of the COM attached to the Nucleo
    // Note that the following function will look for the 
    // Nucleo with name USBSER000 so be careful if you have 2 Nucleo 
    // inserted

    printf("Detect Nucleo Board ...");
    NecleoComStatus = GetNucleoSerialComm(SerialCommStr);
    
    if ((NecleoComStatus == 0) || (strcmp(SerialCommStr, "") == 0)) {
		    printf("Error Nucleo Board not Detected!\n");
	    return(1);
    }
    
	    printf("Nucleo Board detected on %s\n\n", SerialCommStr);
    }
#endif// ___NOP    

    // Initialize Comms
    pMyDevice->I2cDevAddr      = 0x52;
    pMyDevice->comms_type      =  1;
    pMyDevice->comms_speed_khz =  400;

#ifdef ___NOP    
    Status = VL53L0X_i2c_init(SerialCommStr, 460800);
    if (Status != VL53L0X_ERROR_NONE) {
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;
        init_done = 1;
    } else
        printf ("Init Comms\n");
#endif// ___NOP    

    /*
     * Disable VL53L0X API logging if you want to run at full speed
     */
#ifdef VL53L0X_LOG_ENABLE
    VL53L0X_trace_config("test.log", TRACE_MODULE_ALL, TRACE_LEVEL_ALL, TRACE_FUNCTION_ALL);
#endif

    /*
     *  Get the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        status_int = VL53L0X_GetVersion(pVersion);
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    /*
     *  Verify the version of the VL53L0X API running in the firmware
     */

    if(Status == VL53L0X_ERROR_NONE)
    {
        if( pVersion->major != VERSION_REQUIRED_MAJOR ||
            pVersion->minor != VERSION_REQUIRED_MINOR ||
            pVersion->build != VERSION_REQUIRED_BUILD )
        {
            printf("VL53L0X API Version Error: Your firmware has %d.%d.%d (revision %d). This example requires %d.%d.%d.\n",
                pVersion->major, pVersion->minor, pVersion->build, pVersion->revision,
                VERSION_REQUIRED_MAJOR, VERSION_REQUIRED_MINOR, VERSION_REQUIRED_BUILD);
        }
    }


    if(Status == VL53L0X_ERROR_NONE)
    {
        printf ("Call of VL53L0X_DataInit\n");
        Status = VL53L0X_DataInit(&MyDevice); // Data initialization
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = VL53L0X_GetDeviceInfo(&MyDevice, &DeviceInfo);
        
    
        if(Status == VL53L0X_ERROR_NONE)
        {
            printf("VL53L0X_GetDeviceInfo:\r\n");
            printf("Device Name : %s\r\n", DeviceInfo.Name);
            printf("Device Type : %s\r\n", DeviceInfo.Type);
            printf("Device ID : %s\r\n", DeviceInfo.ProductId);
            printf("ProductRevisionMajor : %d\r\n", DeviceInfo.ProductRevisionMajor);
        printf("ProductRevisionMinor : %d\r\n", DeviceInfo.ProductRevisionMinor);

        if ((DeviceInfo.ProductRevisionMinor != 1) && (DeviceInfo.ProductRevisionMinor != 1)) {
        	printf("Error expected cut 1.1 but found cut %d.%d\r\n",
                       DeviceInfo.ProductRevisionMajor, DeviceInfo.ProductRevisionMinor);
                Status = VL53L0X_ERROR_NOT_SUPPORTED;
            }
        }
        print_pal_error(Status);
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
        Status = LRrangingTest(pMyDevice);
    }

    print_pal_error(Status);
    
    // Implementation specific

    /*
     *  Disconnect comms - part of VL53L0X_platform.c
     */

    
#ifdef ___NOP
    if(init_done == 0)
    {
        printf ("Close Comms\r\n");
        status_int = VL53L0X_comms_close();
        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }
#endif

    print_pal_error(Status);
	
//    printf ("\nPress a Key to continue!");
//    getchar();
    

}

/*******************************************************************************
 End of File
*/
