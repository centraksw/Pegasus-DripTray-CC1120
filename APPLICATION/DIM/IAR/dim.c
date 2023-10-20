#include "defines.h"
#include "dim.h"
#include "timer_drv.h"
#include "i2c_drv.h"
#include "flash_app.h"
#include "motion.h"
#include <string.h>

static BYTE MeasurementCounter;
static WORD ProximityValue;
static WORD ProximityArray1[MAX_PROXIMITY_DATA];
static BYTE I2C_Command[2];
static BYTE ProximityCount;
static BYTE TriggerSource;

static WORD MeasureProximityData(BYTE count)
{
    DWORD Total = 0;
    WORD I2CVal = 0;
    BYTE idx, data, SuccessCnt = 0, retry;

    for(idx=0; idx<count; idx++)
    {
        /* Start a proximity measurement and wait until reading is avaialble */
        I2C_Command[0] = READ_COMMAND;
        I2C_Command[1] = 0x8;
        I2CMST_writeBlock(SLAVE_ADDRESS, 2, I2C_Command);
        TIMER_CCR_Delay(5);

        for(retry=0; retry<10; retry++) {
            I2C_Command[0] = READ_COMMAND;
            I2CMST_writeBlock(SLAVE_ADDRESS, 1, I2C_Command);
            TIMER_CCR_Delay(5);
            I2CMST_readBlock(SLAVE_ADDRESS, 1, &data);
            TIMER_CCR_Delay(5);
            if (data == 0xA0)
            {
                /* Read the proximity value */
                I2C_Command[0] = IR_DATA_MSB;
                I2CMST_writeBlock(SLAVE_ADDRESS, 1, I2C_Command);
                TIMER_CCR_Delay(5);
                I2CMST_readBlock(SLAVE_ADDRESS, 1, &data);
                TIMER_CCR_Delay(5);
                I2CVal = data;
                I2C_Command[0] = IR_DATA_LSB;
                I2CMST_writeBlock(SLAVE_ADDRESS, 1, I2C_Command);
                TIMER_CCR_Delay(5);
                I2CMST_readBlock(SLAVE_ADDRESS, 1, &data);
                TIMER_CCR_Delay(5);
                I2CVal <<= 8;
                I2CVal |= data;
                Total += I2CVal;
                SuccessCnt += 1;
                break;
            }
        }
    }

    if( SuccessCnt > 0 )
        I2CVal = Total / SuccessCnt;

    return I2CVal;
}

static WORD CalculateRunningAverage(WORD *proxyValue, BYTE Count)
{
    DWORD runningAverage = 0;
    BYTE idx;

    for(idx=0; idx<Count; idx++)
        runningAverage += proxyValue[idx];

    runningAverage = runningAverage / Count;

    return runningAverage;
}

static WORD CalculateRunningMedian(WORD *proxyValue, BYTE Count)
{
    WORD retVal = 0;
    BYTE i, j;

    for(i=0; i<Count-1; i++)
    {
        for(j=i+1; j<Count; j++)
        {
            if( proxyValue[i] > proxyValue[j] )
            {
                retVal = proxyValue[i];
                proxyValue[i] = proxyValue[j];
                proxyValue[j] = retVal;
            }
        }
    }

    if( Count & 1 )
        retVal = proxyValue[(Count/2)];
    else
    {
        Count /= 2;
        retVal = (proxyValue[Count-1] + proxyValue[Count])/2;
    }

    return retVal;
}


VOID DIM_ProxymityInit()
{
    I2CMST_init();

    I2CMST_start();
    TIMER_CCR_Delay(10);
    I2CMST_stop();
    TIMER_CCR_Delay(10);

    /* Set Proximity IR LED current */
    I2C_Command[0] = IR_LED_CURRENT;
    I2C_Command[1] = 2;
    I2CMST_writeBlock(SLAVE_ADDRESS, 2, I2C_Command);
    TIMER_CCR_Delay(5);

    /* Set PROXIMITY MEASUREMENT SIGNAL FREQUENCY */
    I2C_Command[0] = PROXIMITY_SIGNAL_FREQ;
    I2C_Command[1] = 0x03;
    I2CMST_writeBlock(SLAVE_ADDRESS, 2, I2C_Command);
    TIMER_CCR_Delay(5);

    /* Set PROXIMITY MODULATOR TIMING ADJUSTMENT Delay time = 4, Dead Time = 1*/
    I2C_Command[0] = PROXIMITY_MODULATOR_TIMING;
    I2C_Command[1] = 0x81;
    I2CMST_writeBlock(SLAVE_ADDRESS, 2, I2C_Command);
    TIMER_CCR_Delay(5);
}

BOOL DIM_IsProximityTriggered(BYTE Profile)
{
    BOOL blnProximitySet = FALSE;
    BYTE idx;
    WORD runningValue, measuredValue;
    WORD tempProxyValue[MAX_PROXIMITY_DATA];

    //Return false if Trigger on Proximity is not enabled in Profile.
    if( Profile <= TRIGGER_MOTION_SENSOR )
        return FALSE;

    ++MeasurementCounter;
    if( MeasurementCounter >= flash_settings.MeasurementRate )
    {
        MeasurementCounter = 0;
        ProximityValue = MeasureProximityData(flash_settings.SampleCount);

        if( ProximityCount >= flash_settings.MaxProximityValue )
        {
            if( flash_settings.calculatingType )
            {
                memcpy(tempProxyValue, ProximityArray1, ProximityCount * sizeof(WORD));
                runningValue = CalculateRunningMedian(tempProxyValue, flash_settings.MaxProximityValue);
            }
            else
                runningValue = CalculateRunningAverage(ProximityArray1, flash_settings.MaxProximityValue);

            if( ProximityValue > runningValue && (ProximityValue - runningValue) > flash_settings.Threshold )
            {
                for(idx=0; idx<flash_settings.MaxProximityValue; idx++)
                {
                    tempProxyValue[idx] = MeasureProximityData(flash_settings.SampleCount);
                    TIMER_DelayMS(2);
                }

                if( flash_settings.calculatingType )
                    measuredValue = CalculateRunningMedian(tempProxyValue, flash_settings.MaxProximityValue);
                else
                    measuredValue = CalculateRunningAverage(tempProxyValue, flash_settings.MaxProximityValue);

                if( measuredValue > runningValue && (measuredValue - runningValue) > flash_settings.Threshold )
                {
                    blnProximitySet = TRUE;
                    TriggerSource |= TRIGGER_PROXIMITY_SENSOR;
                    ProximityCount = 0;
                }
            }
	    	if( ProximityCount >= flash_settings.MaxProximityValue )
			{
	    ProximityCount = flash_settings.MaxProximityValue-1;
            memcpy(ProximityArray1+0, ProximityArray1+1, ProximityCount * sizeof(WORD));
			}
        }
        ProximityArray1[ProximityCount++] = ProximityValue;
    }
    return blnProximitySet;
}

VOID DIM_ResetProxymityCount()
{
    ProximityCount = 0;
}

BOOL DIM_CheckIsTriggered(BYTE Profile)
{
    BOOL blnRes = FALSE;

    if( MD_Triggered() )
        TriggerSource |= TRIGGER_MOTION_SENSOR;

    switch(Profile)
    {
    case TRIGGER_NONE:
        blnRes = FALSE;
        break;
    case TRIGGER_MOTION_SENSOR:
        blnRes = ((TriggerSource & 1) == 1);
        break;
    case TRIGGER_PROXIMITY_SENSOR:
        blnRes = ((TriggerSource & 2) == 2);
        break;
    case TRIGGER_MOTION_OR_PROXIMITY_SENSOR:
        blnRes = (TriggerSource > 0);
        break;
    case TRIGGER_MOTION_AND_PROXIMITY_SENSOR:
        blnRes = ((TriggerSource & 3) == 3);
        break;
    }

    TriggerSource = 0;

    return blnRes;
}