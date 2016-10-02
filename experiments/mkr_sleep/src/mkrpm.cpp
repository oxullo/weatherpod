/*
   Weatherpod - WiFi E-ink weather widget
   Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Parts of the following code come from the Arduino library RTCZero
   https://github.com/arduino-libraries/RTCZero

 */

#include <stdint.h>

#include "mkrpm.h"

void RTC_Handler(void)
{
    RTC->MODE2.INTFLAG.reg = RTC_MODE2_INTFLAG_ALARM0; // must clear flag at end
}

MkrPM::MkrPM()
{
}

void MkrPM::begin()
{
    uint16_t tmp_reg = 0;

    PM->APBAMASK.reg |= PM_APBAMASK_RTC; // turn on digital interface clock

    // Setup clock GCLK2 with OSC32K divided by 32
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2)|GCLK_GENDIV_DIV(4);
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN |
                        //  GCLK_GENCTRL_SRC_XOSC32K |
                         GCLK_GENCTRL_SRC_OSCULP32K |
                         GCLK_GENCTRL_ID(2) |
                         GCLK_GENCTRL_DIVSEL;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    GCLK->CLKCTRL.reg = (uint32_t)((GCLK_CLKCTRL_CLKEN |
                                    GCLK_CLKCTRL_GEN_GCLK2 |
                                    (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos)));
    while (GCLK->STATUS.bit.SYNCBUSY);

    RTCdisable();
    RTCreset();

    tmp_reg |= RTC_MODE2_CTRL_MODE_CLOCK; // set clock operating mode
    tmp_reg |= RTC_MODE2_CTRL_PRESCALER_DIV1024; // set prescaler to 1024 for MODE2
    tmp_reg &= ~RTC_MODE2_CTRL_MATCHCLR; // disable clear on match

    RTC->MODE2.READREQ.reg &= ~RTC_READREQ_RCONT; // disable continuously mode

    RTC->MODE2.CTRL.reg = tmp_reg;
    while (RTCisSyncing());

    NVIC_EnableIRQ(RTC_IRQn); // enable RTC interrupt
    NVIC_SetPriority(RTC_IRQn, 0x00);

    RTC->MODE2.INTENSET.reg |= RTC_MODE2_INTENSET_ALARM0; // enable alarm interrupt
    RTC->MODE2.Mode2Alarm[0].MASK.bit.SEL = RTC_MODE2_MASK_SEL_OFF_Val; // default alarm match is off (disabled)

    while (RTCisSyncing());

    RTCenable();
    RTCresetRemove();
}

void MkrPM::sleepForMinutes(uint8_t minutes)
{
    uint8_t elapsed = 0;

    RTC->MODE2.CLOCK.bit.MINUTE = 0;
    while (RTCisSyncing());

    RTC->MODE2.CLOCK.bit.SECOND = 0;
    while (RTCisSyncing());

    RTC->MODE2.Mode2Alarm[0].ALARM.bit.MINUTE = minutes;
    while (RTCisSyncing());

    RTC->MODE2.Mode2Alarm[0].MASK.bit.SEL = RTC_MODE2_MASK_SEL_MMSS_Val;
    while (RTCisSyncing());

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __DSB();
    __WFI();    // Wait For Interrupt, going here in deep sleep

    RTC->MODE2.Mode2Alarm[0].MASK.bit.SEL = 0x00;
    while (RTCisSyncing());
}

// Wait for sync in write operations
inline bool MkrPM::RTCisSyncing()
{
    return (RTC->MODE2.STATUS.bit.SYNCBUSY);
}

void MkrPM::RTCdisable()
{
    RTC->MODE2.CTRL.reg &= ~RTC_MODE2_CTRL_ENABLE; // disable RTC
    while (RTCisSyncing());
}

void MkrPM::RTCenable()
{
    RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_ENABLE; // enable RTC
    while (RTCisSyncing());
}

void MkrPM::RTCreset()
{
    RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_SWRST; // software reset
    while (RTCisSyncing());
}

void MkrPM::RTCresetRemove()
{
    RTC->MODE2.CTRL.reg &= ~RTC_MODE2_CTRL_SWRST; // software reset remove
    while (RTCisSyncing());
}
