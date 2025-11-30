/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "oled.h"
#include "utils.h"
#include "nbiot.h"
#include "sht3x.h"
#ifdef KY_005_IR_ENISSION
#include "irsnd.h"
#endif

#ifdef KY_022_IR_RECEIVER
#include "irmp.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIGHT_DARK_THRESHOLD            (200U)
#define SOUND_NOTICE_THRESHOLD          (600U)
#define SOUND_SNORE_THRESHOLD           (1200U)
#define SENSOR_POLL_INTERVAL_MS         (1000U)
#define OLED_REFRESH_INTERVAL_MS        (500U)
#define ALARM_DURATION_MS               (5000U)
#define ALARM_DEFAULT_HOUR              (7U)
#define ALARM_DEFAULT_MIN               (0U)
#define SLEEP_CLOCK_START_HOUR          (0U)
#define ALARM_BOOT_DELAY_MS             (30000U)
#define LED_FLASH_INTERVAL_MS           (1000U)
#define MANUAL_ALARM_DELAY_MS           (5000U)
#define TIP_DISPLAY_DURATION_MS         (4000U)
#define CHARGE_VOLTAGE_THRESHOLD_MV     (4100U)
#define MELODY_REST_MS                  (30U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern unsigned char key1, key2;

typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} SleepClock_t;

static uint8_t gLedPowerOn = 0U;      // LED 总开关状态 (0:关, 1:开)
static uint8_t gHw483LastState = 0U;  // 按钮上一次的电平状态

static SleepClock_t gSleepClock = {SLEEP_CLOCK_START_HOUR, 0U, 0U};
static uint32_t gLastClockTickMs = 0U;
static uint32_t gLastSensorPollMs = 0U;
static uint32_t gLastOledRefreshMs = 0U;
static uint32_t gAlarmStartMs = 0U;
static uint32_t gBootTickMs = 0U;
static uint8_t gAlarmLatched = 0U;
static uint8_t gLedModuleEnabled = 0U;
static uint8_t gOledEnabled = 1U;
static uint8_t gAlarmActive = 0U;
static uint8_t gLedMode = 0U; /* LED_MODE_OFF */
static uint8_t gLedFlashIndex = 0U;
static uint32_t gLastLedFlashTick = 0U;
static float gLastTemp = 0.0f;
static float gLastHumi = 0.0f;
static uint16_t gLastLight = 0U;
static uint16_t gLastSound = 0U;
static uint16_t gLastVoltage = 0U;
static uint8_t gCurrentPage = 0U;
static char gPageTipBuf[32] = {0};
static uint8_t gTipPageActive = 0U;
static uint8_t gTipReturnPage = 0U;
static uint8_t gTipPrevPage = 0U;
static uint8_t gManualAlarmCountdownActive = 0U;
static uint32_t gManualAlarmTargetTick = 0U;
static uint8_t gLastRenderedPage = 0xFFU;
static uint8_t gOledNeedsFullRefresh = 1U;
static uint8_t gMelodyPlaying = 0U;
static size_t gMelodyIndex = 0U;
static uint32_t gMelodyStepEndMs = 0U;
static uint8_t gMelodyInPause = 0U;

enum
{
	LED_MODE_OFF = 0,
	LED_MODE_WHITE,
	LED_MODE_BLUE,
	LED_MODE_RED,
	LED_MODE_GREEN,
	LED_MODE_FLASH,
	LED_MODE_COUNT
};

enum
{
	OLED_PAGE_SENSORS_1 = 0,
	OLED_PAGE_SENSORS_2,
	OLED_PAGE_LED,
	OLED_PAGE_ALARM,
	OLED_PAGE_COUNT
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void SleepClock_Tick(void);
static void SleepClock_Format(char *pcBuf, size_t bufLen);
static bool SleepEnvironment_Comfort(float temp, float humi);
static uint8_t SleepComfort_Score(float temp, float humi, uint16_t soundLevel, uint16_t lightLevel);
static const char *SleepComfort_Tip(float temp, float humi, uint16_t soundLevel, uint16_t lightLevel);
static void SleepTracker_HandleKeys(void);
static void SleepTracker_UpdateSensors(void);
static void SleepTracker_UpdateAlarm(void);
static void SleepTracker_ShowBootScreen(void);
static uint32_t SleepTracker_GetTim2ClockHz(void);
static void SleepTracker_BuzzerStartTone(uint16_t freq);
static void SleepTracker_BuzzerStopTone(void);
static void SleepTracker_SetRgb(uint8_t rOn, uint8_t gOn, uint8_t bOn);
static void SleepTracker_DrawPageSensors1(void);
static void SleepTracker_DrawPageSensors2(void);
static void SleepTracker_DrawPageLed(void);
static void SleepTracker_DrawPageAlarm(void);
static void SleepTracker_DrawPageTip(void);
static void SleepTracker_HandleAlarmToggle(void);
static const char *SleepTracker_LedModeName(uint8_t mode);
static uint16_t SleepTracker_BatteryMv(uint16_t raw);
static void SleepTracker_MelodyStart(void);
static void SleepTracker_MelodyStop(void);
static void SleepTracker_MelodyTask(void);

typedef struct {
	uint16_t freq;
	uint16_t durationMs;
} ToneStep_t;

static const ToneStep_t gJingleBells[] = {
		{523, 150}, {0, 50}, {523, 150}, {0, 50}, // Do, Do
		    {659, 150}, {0, 50}, {659, 150}, {0, 50}, // Mi, Mi
		    {783, 400}, {0, 100},                     // Sol
		    {659, 200}, {783, 200}, {1046, 600},      // Mi Sol Do(High)
		    {0, 400},                                 // 休止
		    {1046, 200}, {783, 200}, {659, 200}, {523, 600} // Do(H) Sol Mi Do
};
static const size_t gJingleBellCount = sizeof(gJingleBells)/sizeof(gJingleBells[0]);

static void SleepTracker_HandleAlarmToggle(void)
{
	uint32_t now = HAL_GetTick();
	if(gAlarmActive) {
		gAlarmActive = 0U;
		Beep_Switch(0);
		return;
	}
	if(gManualAlarmCountdownActive) {
		gManualAlarmCountdownActive = 0U;
		return;
	}
	gManualAlarmCountdownActive = 1U;
	gManualAlarmTargetTick = now + MANUAL_ALARM_DELAY_MS;
}

static const char *SleepTracker_LedModeName(uint8_t mode)
{
	switch(mode) {
		case LED_MODE_OFF:   return "Off";
		case LED_MODE_WHITE: return "White";
		case LED_MODE_BLUE:  return "Green";
		case LED_MODE_RED:   return "Blue";
		case LED_MODE_GREEN: return "Red";
		case LED_MODE_FLASH: return "Flash";
		default: return "Unknown";
	}
}

static void SleepTracker_DrawPageSensors1(void)
{
	char buf[24] = {0};
	char timeBuf[12] = {0};
	SleepClock_Format(timeBuf, sizeof(timeBuf));
	snprintf(buf, sizeof(buf), "Time %s", timeBuf);
	OLED_ShowString(0, 0, (uint8_t *)buf, 8);
	snprintf(buf, sizeof(buf), "Temp:%4.1fC", gLastTemp);
	OLED_ShowString(0, 1, (uint8_t *)buf, 8);
	snprintf(buf, sizeof(buf), "Humi:%4.1f%%", gLastHumi);
	OLED_ShowString(0, 2, (uint8_t *)buf, 8);
	snprintf(buf, sizeof(buf), "Light:%04u", gLastLight);
	OLED_ShowString(0, 3, (uint8_t *)buf, 8);
	snprintf(buf, sizeof(buf), "Sound:%04u", gLastSound);
	OLED_ShowString(0, 4, (uint8_t *)buf, 8);
}

static void SleepTracker_DrawPageSensors2(void)
{
	char buf[24] = {0};
	uint8_t score = SleepComfort_Score(gLastTemp, gLastHumi, gLastSound, gLastLight);
	uint16_t batteryMv = SleepTracker_BatteryMv(gLastVoltage);
	bool charging = (batteryMv >= CHARGE_VOLTAGE_THRESHOLD_MV);
	snprintf(buf, sizeof(buf), "Volt:%04umV", batteryMv);
	OLED_ShowString(0, 0, (uint8_t *)buf, 8);
	snprintf(buf, sizeof(buf), "Charge:%s", charging ? "Yes" : "No");
	OLED_ShowString(0, 1, (uint8_t *)buf, 8);
	snprintf(buf, sizeof(buf), "Env:%3u%%", score);
	OLED_ShowString(0, 2, (uint8_t *)buf, 8);
	snprintf(buf, sizeof(buf), "Noise:%04u", gLastSound);
	OLED_ShowString(0, 3, (uint8_t *)buf, 8);
	OLED_ShowString(0, 4, (uint8_t *)"K2:Sleep tips", 8);
}

static void SleepTracker_DrawPageLed(void)
{
	char buf[24] = {0};
	snprintf(buf, sizeof(buf), "LED:%s", SleepTracker_LedModeName(gLedMode));
	OLED_ShowString(0, 0, (uint8_t *)buf, 8);
	OLED_ShowString(0, 1, (uint8_t *)"Seq:Off-W-G-B-R-F", 4);
	snprintf(buf, sizeof(buf), "Flash:%s", (LED_MODE_FLASH == gLedMode) ? "On" : "Off");
	OLED_ShowString(0, 2, (uint8_t *)buf, 8);
	OLED_ShowString(0, 3, (uint8_t *)"K2 cycle mode", 8);
}

static void SleepTracker_DrawPageAlarm(void)
{
	char buf[24] = {0};
	uint32_t now = HAL_GetTick();
	uint32_t remainingMs = 0U;
	if(gManualAlarmCountdownActive && gManualAlarmTargetTick > now) {
		remainingMs = gManualAlarmTargetTick - now;
	}
	snprintf(buf, sizeof(buf), "Alarm:%s", gAlarmActive ? "Ringing" : (gManualAlarmCountdownActive ? "Pending" : "Idle"));
	OLED_ShowString(0, 0, (uint8_t *)buf, 8);
	if(gManualAlarmCountdownActive) {
		snprintf(buf, sizeof(buf), "Countdown:%lus", (unsigned long)((remainingMs + 999U) / 1000U));
	} else {
		strncpy(buf, "Countdown:--", sizeof(buf) - 1U);
	}
	OLED_ShowString(0, 1, (uint8_t *)buf, 8);
	OLED_ShowString(0, 2, (uint8_t *)"K2 start/stop", 8);
	OLED_ShowString(0, 3, (uint8_t *)"2nd tap cancels", 8);
}

static void SleepTracker_DrawPageTip(void)
{
	OLED_ShowString(0, 0, (uint8_t *)"Sleep Tip", 8);
	OLED_ShowString(0, 2, (uint8_t *)gPageTipBuf, 8);
	OLED_ShowString(0, 3, (uint8_t *)(gPageTipBuf + 16), 8);
	OLED_ShowString(0, 5, (uint8_t *)"K2 to return", 8);
}

static void SleepClock_Tick(void)
{
	uint32_t nowMs = HAL_GetTick();
	if(0U == gLastClockTickMs) {
		gLastClockTickMs = nowMs;
	}
	if(nowMs < gLastClockTickMs + 1000U) {
		return;
	}
	uint32_t elapsed = (nowMs - gLastClockTickMs) / 1000U;
	gLastClockTickMs += elapsed * 1000U;
	while(elapsed--) {
		gSleepClock.second++;
		if(gSleepClock.second >= 60U) {
			gSleepClock.second = 0U;
			gSleepClock.minute++;
			if(gSleepClock.minute >= 60U) {
				gSleepClock.minute = 0U;
				gSleepClock.hour++;
				if(gSleepClock.hour >= 24U) {
					gSleepClock.hour = 0U;
				}
			}
		}
	}
}

static void SleepClock_Format(char *pcBuf, size_t bufLen)
{
	snprintf(pcBuf, bufLen, "%02u:%02u:%02u", gSleepClock.hour, gSleepClock.minute, gSleepClock.second);
}

static bool SleepEnvironment_Comfort(float temp, float humi)
{
	return (temp >= 18.0f && temp <= 26.5f) && (humi >= 40.0f && humi <= 65.0f);
}

static uint8_t SleepComfort_Score(float temp, float humi, uint16_t soundLevel, uint16_t lightLevel)
{
	uint8_t score = 100U;
	if(temp < 18.0f) {
		score -= (uint8_t)((18.0f - temp) * 2.0f);
	} else if(temp > 26.5f) {
		score -= (uint8_t)((temp - 26.5f) * 2.0f);
	}
	if(humi < 40.0f) {
		score -= (uint8_t)((40.0f - humi));
	} else if(humi > 65.0f) {
		score -= (uint8_t)((humi - 65.0f));
	}
	if(soundLevel >= SOUND_SNORE_THRESHOLD) {
		score -= 40U;
	} else if(soundLevel >= SOUND_NOTICE_THRESHOLD) {
		score -= 20U;
	}
	if(lightLevel > LIGHT_DARK_THRESHOLD) {
		score -= 15U;
	}
	if(score > 100U) {
		score = 0U;
	}
	return score;
}

static const char *SleepComfort_Tip(float temp, float humi, uint16_t soundLevel, uint16_t lightLevel)
{
	if(soundLevel >= SOUND_SNORE_THRESHOLD) {
		return "Tip:Noise very high";
	}
	if(soundLevel >= SOUND_NOTICE_THRESHOLD) {
		return "Tip:Noise slightly high";
	}
	if(lightLevel > LIGHT_DARK_THRESHOLD) {
		return "Tip:Dim the lights";
	}
	if(temp < 18.0f) {
		return "Tip:Room too cold";
	}
	if(temp > 26.5f) {
		return "Tip:Lower temp";
	}
	if(humi < 40.0f) {
		return "Tip:Add humidity";
	}
	if(humi > 65.0f) {
		return "Tip:Dry the air";
	}
	return "Tip:Environment ok";
}

static void SleepTracker_HandleKeys(void)
{
	if(1 == key1) {
		key1 = 0;
		if(gTipPageActive) {
			gTipPageActive = 0U;
			gCurrentPage = (gTipPrevPage + 1U) % OLED_PAGE_COUNT;
		} else {
			gCurrentPage = (gCurrentPage + 1U) % OLED_PAGE_COUNT;
		}
		memset(gPageTipBuf, 0, sizeof(gPageTipBuf));
		gOledNeedsFullRefresh = 1U;
	}
	if(1 == key2) {
		key2 = 0;
		if(gAlarmActive) {
			gAlarmActive = 0U;
			gManualAlarmCountdownActive = 0U;
			SleepTracker_MelodyStop();
			gOledNeedsFullRefresh = 1U;
			return;
		}
		if(gTipPageActive) {
			gTipPageActive = 0U;
			gCurrentPage = gTipReturnPage % OLED_PAGE_COUNT;
			gOledNeedsFullRefresh = 1U;
			return;
		}
		switch(gCurrentPage) {
			case OLED_PAGE_SENSORS_2:
			{
				const char *tip = SleepComfort_Tip(gLastTemp, gLastHumi, gLastSound, gLastLight);
				strncpy(gPageTipBuf, tip, sizeof(gPageTipBuf)-1U);
				gPageTipBuf[sizeof(gPageTipBuf)-1U] = '\0';
				gTipPrevPage = gCurrentPage;
				gTipReturnPage = gCurrentPage;
				gTipPageActive = 1U;
				gOledNeedsFullRefresh = 1U;
				break;
			}
			case OLED_PAGE_LED:
//				gLedMode = (gLedMode + 1U) % LED_MODE_COUNT;
//				gLedModuleEnabled = (gLedMode != LED_MODE_OFF);
//				if(LED_MODE_FLASH == gLedMode) {
//					gLedFlashIndex = 0U;
//					gLastLedFlashTick = 0U;
//				}
//				gOledNeedsFullRefresh = 1U;
//				break;
				gLedMode++;
				                if (gLedMode >= LED_MODE_COUNT) {
				                    gLedMode = LED_MODE_WHITE; // 跳过 0，回到 1
				                }

				                // 确保总开关是开的，否则用户调颜色时看不到效果
				                if (!gLedPowerOn) {
				                    gLedPowerOn = 1U;
				                }

				                if(LED_MODE_FLASH == gLedMode) {
				                    gLedFlashIndex = 0U;
				                    gLastLedFlashTick = 0U;
				                }
				                gOledNeedsFullRefresh = 1U;
				                break;
			case OLED_PAGE_ALARM:
				SleepTracker_HandleAlarmToggle();
				gOledNeedsFullRefresh = 1U;
				break;
			default:
				break;
		}
	}
}

static void SleepTracker_UpdateSensors(void)
{
	uint32_t nowMs = HAL_GetTick();
	if((nowMs - gLastSensorPollMs) < SENSOR_POLL_INTERVAL_MS) {
		return;
	}
	gLastSensorPollMs = nowMs;
	SHT3X_GetTempAndHumi(&gLastTemp, &gLastHumi, REPEATAB_HIGH, MODE_CLKSTRETCH, 6);
	KE1_ADC_Senser_Get(&gLastLight, &gLastSound, &gLastVoltage);
}

static uint32_t SleepTracker_GetTim2ClockHz(void)
{
	uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
	if((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1) {
		pclk1 *= 2U;
	}
	return pclk1;
}

static void SleepTracker_BuzzerStartTone(uint16_t freq)
{
	if(0U == freq) {
		SleepTracker_BuzzerStopTone();
		return;
	}
	uint32_t timerClk = SleepTracker_GetTim2ClockHz();
	uint32_t period = timerClk / (((uint32_t)htim2.Init.Prescaler + 1U) * (uint32_t)freq);
	if(period == 0U) {
		period = 1U;
	}
	__HAL_TIM_SET_AUTORELOAD(&htim2, period - 1U);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (period - 1U)/2U);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}

static void SleepTracker_BuzzerStopTone(void)
{
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
}

static void SleepTracker_MelodyStart(void)
{
	gMelodyPlaying = 1U;
	gMelodyIndex = 0U;
	gMelodyStepEndMs = 0U;
	gMelodyInPause = 1U;
	Beep_Switch(1);
}

static void SleepTracker_MelodyStop(void)
{
	if(!gMelodyPlaying && gAlarmActive == 0U) {
		SleepTracker_BuzzerStopTone();
		Beep_Switch(0);
		return;
	}
	gMelodyPlaying = 0U;
	gMelodyIndex = 0U;
	gMelodyStepEndMs = 0U;
	gMelodyInPause = 0U;
	SleepTracker_BuzzerStopTone();
	Beep_Switch(0);
}


// HW483
static void SleepTracker_PollExternalButton(void)
{
    // 读取 PA7 状态
    uint8_t currentState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);

    // 检测上升沿 (从 0 变 1)
    if(currentState == 1 && gHw483LastState == 0) {
        // 切换 LED 总电源状态
        gLedPowerOn = !gLedPowerOn;

        // 如果开启了电源且当前 LED 模式是 OFF，自动设为白色，避免开启后没反应
        if(gLedPowerOn && gLedMode == LED_MODE_OFF) {
            gLedMode = LED_MODE_WHITE;
        }

        // 触发屏幕刷新以显示状态
        gOledNeedsFullRefresh = 1U;

    }
    gHw483LastState = currentState;
}

static void SleepTracker_MelodyTask(void)
{
	if(!gMelodyPlaying || 0U == gAlarmActive) {
		if(gMelodyPlaying) {
			SleepTracker_MelodyStop();
		}
		return;
	}
	uint32_t now = HAL_GetTick();
	if(gMelodyStepEndMs != 0U && now < gMelodyStepEndMs) {
		return;
	}
	if(gMelodyInPause) {
		if(gMelodyIndex >= gJingleBellCount) {
			gMelodyIndex = 0U;
		}
		const ToneStep_t *step = &gJingleBells[gMelodyIndex++];
		SleepTracker_BuzzerStartTone(step->freq);
		gMelodyStepEndMs = now + step->durationMs;
		gMelodyInPause = 0U;
		return;
	}
	SleepTracker_BuzzerStopTone();
	gMelodyInPause = 1U;
	gMelodyStepEndMs = now + MELODY_REST_MS;
}

static uint16_t SleepTracker_BatteryMv(uint16_t raw)
{
	uint32_t mv = ((uint32_t)raw * 3300U * 2U) / 4095U;
	if(mv > 5000U) {
		mv = 5000U;
	}
	return (uint16_t)mv;
}

static void SleepTracker_ShowBootScreen(void)
{
	float bootTemp = 0.0f, bootHumi = 0.0f;
	uint16_t bootLight = 0U, bootSound = 0U, bootVoltage = 0U;
	SHT3X_GetTempAndHumi(&bootTemp, &bootHumi, REPEATAB_HIGH, MODE_CLKSTRETCH, 6);
	KE1_ADC_Senser_Get(&bootLight, &bootSound, &bootVoltage);
	OLED_Clear();
	OLED_ShowString(0, 0, (uint8_t *)" HELLO", 16);
	OLED_ShowString(0, 2, (uint8_t *)" WORLD", 16);
	static const char *bootTree[] = {
		"  *  ",
		" *** ",
		"*****",
		"  I  "
	};
	for(uint8_t i = 0; i < (sizeof(bootTree)/sizeof(bootTree[0])); ++i) {
		OLED_ShowString(70, i, (uint8_t *)bootTree[i], 8);
	}
	HAL_Delay(1500);
	OLED_Clear();
	char lineBuf[22] = {0};
	snprintf(lineBuf, sizeof(lineBuf), "Temp:%4.1fC", bootTemp);
	OLED_ShowString(0, 0, (uint8_t *)lineBuf, 12);
	snprintf(lineBuf, sizeof(lineBuf), "Humi:%4.1f%%", bootHumi);
	OLED_ShowString(0, 2, (uint8_t *)lineBuf, 12);
	snprintf(lineBuf, sizeof(lineBuf), "Light:%04u", bootLight);
	OLED_ShowString(0, 4, (uint8_t *)lineBuf, 12);
	snprintf(lineBuf, sizeof(lineBuf), "Sound:%04u", bootSound);
	OLED_ShowString(0, 6, (uint8_t *)lineBuf, 12);
	snprintf(lineBuf, sizeof(lineBuf), "Volt:%04umV", bootVoltage);
	OLED_ShowString(0, 7, (uint8_t *)lineBuf, 12);
	HAL_Delay(1500);
	// SleepTracker_PlayJingleBell();
}

static void SleepTracker_UpdateAlarm(void)
{
	uint32_t now = HAL_GetTick();
	if(0U == gBootTickMs) {
		gBootTickMs = now;
	}
	bool allowAutoAlarm = ((now - gBootTickMs) >= ALARM_BOOT_DELAY_MS);
	if(allowAutoAlarm) {
		if(0U == gAlarmLatched && gSleepClock.hour == ALARM_DEFAULT_HOUR && gSleepClock.minute == ALARM_DEFAULT_MIN) {
			gAlarmLatched = 1U;
			gAlarmActive = 1U;
			gAlarmStartMs = now;
			SleepTracker_MelodyStart();
			SleepTracker_MelodyTask();
		}
		if(gSleepClock.hour != ALARM_DEFAULT_HOUR || gSleepClock.minute != ALARM_DEFAULT_MIN) {
			gAlarmLatched = 0U;
		}
	}
	if(gManualAlarmCountdownActive && !gAlarmActive) {
		if(now >= gManualAlarmTargetTick) {
			gManualAlarmCountdownActive = 0U;
			gAlarmActive = 1U;
			gAlarmStartMs = now;
			SleepTracker_MelodyStart();
			SleepTracker_MelodyTask();
		}
	}
	if(gAlarmActive) {
		if((now - gAlarmStartMs) >= ALARM_DURATION_MS) {
			gAlarmActive = 0U;
			SleepTracker_MelodyStop();
		}
	}
	SleepTracker_MelodyTask();
}

static void SleepTracker_SetRgb(uint8_t rOn, uint8_t gOn, uint8_t bOn)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, rOn ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, gOn ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, bOn ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void SleepTracker_UpdateLed(bool envComfortable)
{
	(void)envComfortable;
//	if(LED_MODE_OFF == gLedMode) {
//		SleepTracker_SetRgb(0U, 0U, 0U);
//		gLedModuleEnabled = 0U;
//		return;
//	}

	if(0U == gLedPowerOn || LED_MODE_OFF == gLedMode) {
	        SleepTracker_SetRgb(0U, 0U, 0U);
	        gLedModuleEnabled = 0U;
	        return;
	    }

	gLedModuleEnabled = 1U;
	if(LED_MODE_FLASH == gLedMode) {
		uint32_t now = HAL_GetTick();
		if((now - gLastLedFlashTick) >= LED_FLASH_INTERVAL_MS) {
			gLastLedFlashTick = now;
			gLedFlashIndex = (gLedFlashIndex + 1U) % 3U;
		}
		switch(gLedFlashIndex) {
			case 0: SleepTracker_SetRgb(1U, 0U, 0U); break;
			case 1: SleepTracker_SetRgb(0U, 1U, 0U); break;
			default: SleepTracker_SetRgb(0U, 0U, 1U); break;
		}
		return;
	}
	switch(gLedMode) {
		case LED_MODE_WHITE:
			SleepTracker_SetRgb(1U, 1U, 1U);
			break;
		case LED_MODE_BLUE:
			SleepTracker_SetRgb(0U, 0U, 1U);
			break;
		case LED_MODE_RED:
			SleepTracker_SetRgb(1U, 0U, 0U);
			break;
		case LED_MODE_GREEN:
			SleepTracker_SetRgb(0U, 1U, 0U);
			break;
		default:
			SleepTracker_SetRgb(0U, 0U, 0U);
			break;
	}
}

static void SleepTracker_UpdateOled(bool envComfortable)
{
	if(!gOledEnabled) {
		return;
	}
	uint32_t nowMs = HAL_GetTick();
	if((nowMs - gLastOledRefreshMs) < OLED_REFRESH_INTERVAL_MS) {
		return;
	}
	gLastOledRefreshMs = nowMs;
	uint8_t currentDisplayPage = gTipPageActive ? 0xFEU : gCurrentPage;
	bool pageChanged = (currentDisplayPage != gLastRenderedPage);
	if(pageChanged || gOledNeedsFullRefresh) {
		OLED_Clear();
		gOledNeedsFullRefresh = 0U;
	}
	if(gTipPageActive) {
		SleepTracker_DrawPageTip();
	} else {
		switch(gCurrentPage) {
			case OLED_PAGE_SENSORS_1:
				SleepTracker_DrawPageSensors1();
				break;
			case OLED_PAGE_SENSORS_2:
				SleepTracker_DrawPageSensors2();
				break;
			case OLED_PAGE_LED:
				SleepTracker_DrawPageLed();
				break;
			case OLED_PAGE_ALARM:
				SleepTracker_DrawPageAlarm();
				break;
			default:
				SleepTracker_DrawPageSensors1();
				break;
		}
	}
	gLastRenderedPage = currentDisplayPage;
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_TIM15_Init();
  /* USER CODE BEGIN 2 */
	Beep_Switch(0);
	UART_Enable_Receive_IT();
	OLED_Init();
	SHT3X_SetI2cAdr(0x44);

	// HW483
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE(); // 确保 GPIOA 时钟开启
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN; // 假设按下输出高电平，平时下拉
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	SleepTracker_ShowBootScreen();
	gLastClockTickMs = HAL_GetTick();
	gBootTickMs = gLastClockTickMs;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		SleepTracker_PollExternalButton();
    SleepClock_Tick();
    SleepTracker_HandleKeys();
    SleepTracker_UpdateSensors();
    bool envComfortable = SleepEnvironment_Comfort(gLastTemp, gLastHumi);
    SleepTracker_UpdateLed(envComfortable);
    SleepTracker_UpdateAlarm();
    SleepTracker_UpdateOled(envComfortable);
    HAL_Delay(50);
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 16;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
