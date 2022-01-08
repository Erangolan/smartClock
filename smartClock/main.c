/*
 * File:   LabC2.c
 * Author: Amit
 *
 * Created on April 11, 2021, 16:17
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "System/system.h"
#include "System/delay.h"
#include "oledDriver/oledC.h"
#include "oledDriver/oledC_colors.h"
#include "oledDriver/oledC_shapes.h"




typedef unsigned char BOOL;
#define FALSE 0
#define TRUE  !FALSE

typedef struct Time {
    int seconds;
    int minutes;
    int houres;
} time;

typedef struct Date {
    int day;
    int month;
} date;

time t;
time alarm;
date d;
unsigned int counter = 0;
uint8_t lightsCounter = 20;
BOOL lights = 0;
BOOL flag = 0;

void displayDate(){
    char str1[10], str2[10];
    sprintf(str1, "%02d/", d.day);
    sprintf(str2, "%02d", d.month);
    strcat(str1, str2);
    oledC_DrawString(63, 80, 1, 1, str1, OLEDC_COLOR_BLACK);
}

// display analog clock
void display_analog(time* t)
{
    
}


//initiLIE both time and date default value
void initDate(){
    d.day = 10;
    d.month = 8;
    t.seconds = 24;
    t.minutes = 18;
    t.houres = 20;
}

void digitalView() {
    if (t.seconds % 6 == 0){
        char str1[10], str2[10], str3[10];
        sprintf(str1, "%02d:", t.houres);
        sprintf(str2, "%02d:", t.minutes);
        sprintf(str3, "%02d", t.seconds);

        strcat(str1, str2);
        strcat(str1, str3);

        flag ? oledC_DrawString(45, 5, 1, 1, str1, OLEDC_COLOR_BLACK) : oledC_DrawString(4, 40, 2, 2, str1, OLEDC_COLOR_BLACK);
    } else if (t.seconds % 5 == 0)
        flag ? oledC_DrawRectangle(45, 0, 95, 25, OLEDC_COLOR_WHITE) : oledC_DrawRectangle(0, 40, 90, 55, OLEDC_COLOR_WHITE);

    if (t.seconds == 59) {
        t.seconds = 0;
        t.minutes++;

        if (t.minutes == 60) {
            t.minutes = 0;
            t.houres++;
        }
    } else
        t.seconds++;
}

void __attribute__((__interrupt__)) _T1Interrupt (void) {    
    counter = LATAbits.LATA8 ? counter + 1 : 0;

    digitalView();

    if ((t.houres == alarm.houres && t.minutes == alarm.minutes && t.seconds == alarm.seconds) || lightsCounter && lightsCounter < 20){
        if (checkLed1Pressed()){
            handleLed1();
            lightsCounter = 20;
            return;
        } else if (checkLed2Pressed()){
            handleLed2();
            lightsCounter = 20;
            oledC_DrawRectangle(0, 0, 2, 2, OLEDC_COLOR_WHITE);
            return;
        }
        oledC_sendCommand(lights ? OLEDC_CMD_SET_DISPLAY_MODE_INVERSE : OLEDC_CMD_SET_DISPLAY_MODE_ON, NULL, 0);
        lights = !lights;
        --lightsCounter;
        if (lightsCounter == 0)
            oledC_DrawRectangle(0, 0, 2, 2, OLEDC_COLOR_WHITE);
    }

    IFS0bits.T1IF = 0;
}

int readPot(){
    AD1CHS = 8 ;
    AD1CON1bits.SAMP = 1;
    DELAY_milliseconds(300);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    return ADC1BUF0;
}

char* getStringTime(time* t){
    char str1[10], str2[10], str3[10];
    sprintf(str1, "%02d:", t->houres);
    sprintf(str2, "%02d:", t->minutes);
    sprintf(str3, "%02d", t->seconds);

    strcat(str1, str2);
    return strcat(str1, str3);
}

char* getStringDate(){
    char str1[10], str2[10];
    sprintf(str1, "%02d/", d.day);
    sprintf(str2, "%02d", d.month);
    return strcat(str1, str2);
}

void setAlarm(){
    int cursor = 5;
    oledC_clearScreen();
    char stringTime[9];
    strcpy(stringTime, getStringTime(&t));
    oledC_DrawString(4, 40, 2, 2, stringTime, OLEDC_COLOR_BLACK);
    oledC_DrawLine(cursor, 60, 20, 60, 1, OLEDC_COLOR_BLACK);

    while(!checkLed2Pressed()){
        if (checkLed1Pressed()){
            handleLed1();
            oledC_DrawLine(cursor, 60, cursor + 15, 60, 1, OLEDC_COLOR_WHITE);
            cursor = cursor == 75 ? 5 : cursor + 35;
            oledC_DrawLine(cursor, 60, cursor + 15, 60, 1, OLEDC_COLOR_BLACK);
            handleLed1();
        }
        switch (cursor) {
            case 5:
                alarm.houres = handler(cursor, 44);
                break;

            case 40:
                alarm.minutes = handler(cursor, 17);
                break;

            case 75:
                alarm.seconds = handler(cursor, 17);
                break;
        }
    }
    handleLed2();
    strcpy(stringTime, getStringTime(&alarm));
    oledC_DrawCharacter(1, 1, 2, 2, 'c', OLEDC_COLOR_BLACK);
}

void setTime(){
    int cursor = 5;
    oledC_clearScreen();
    char stringTime[9];
    strcpy(stringTime, getStringTime(&t));
    oledC_DrawString(4, 40, 2, 2, stringTime, OLEDC_COLOR_BLACK);
    oledC_DrawLine(cursor, 60, 20, 60, 1, OLEDC_COLOR_BLACK);

    while(!checkLed2Pressed()){
        if (checkLed1Pressed()){
            handleLed1();
            oledC_DrawLine(cursor, 60, cursor + 15, 60, 1, OLEDC_COLOR_WHITE);
            cursor = cursor == 75 ? 5 : cursor + 35;
            oledC_DrawLine(cursor, 60, cursor + 15, 60, 1, OLEDC_COLOR_BLACK);
            handleLed1();
        }
        switch (cursor) {
            case 5:
                t.houres = handler(cursor, 44);
                break;

            case 40:
                t.minutes = handler(cursor, 17);
                break;

            case 75:
                t.seconds = handler(cursor, 17);
                break;
        }
    }
    handleLed2();
}

int handler(int cursor, int divider){
    int potval;
    char buffer[4];

    potval = readPot() / divider;
    oledC_DrawRectangle(cursor - 5, 40, cursor + 20, 55, OLEDC_COLOR_WHITE);
    sprintf(buffer, "%02d", potval);
    oledC_DrawString(divider == 44 ? 4 : cursor - 4, 40, 2, 2, buffer, OLEDC_COLOR_BLACK);

    return potval;
}

int setDate(){
    int cursor = 20;
    oledC_clearScreen();
    char stringDate[9];
    strcpy(stringDate, getStringDate());
    oledC_DrawString(cursor, 40, 2, 2, stringDate, OLEDC_COLOR_BLACK);
    oledC_DrawLine(cursor, 60, cursor + 20, 60, 1, OLEDC_COLOR_BLACK);

    while(!checkLed2Pressed()){
        if (checkLed1Pressed()){
            handleLed1();
            oledC_DrawLine(cursor, 60, cursor + 20, 60, 1, OLEDC_COLOR_WHITE);
            cursor = cursor == 55 ? 20 : cursor + 35;
            oledC_DrawLine(cursor, 60, cursor + 15, 60, 1, OLEDC_COLOR_BLACK);
            handleLed1();
        }
        switch (cursor) {
            case 20:
                d.month = handler(cursor, 34);
                break;

            case 55:
                d.day = handler(cursor, 85);
                break;
        }
    }
    handleLed2();
    strcpy(stringDate, getStringDate());
    oledC_DrawString(63, 80, 1, 1, stringDate, OLEDC_COLOR_BLACK);
    handleLed2();
}

void displayMenu(){
    flag = TRUE;
    while(!checkLed2Pressed() && flag){
        int point = 30;
        oledC_DrawRectangle(0, 20, 95, 80, OLEDC_COLOR_WHITE);
        oledC_DrawString(10, 25, 1, 1, "Display Mode", OLEDC_COLOR_BLACK);
        oledC_DrawString(10, 35, 1, 1, "Set Time", OLEDC_COLOR_BLACK);
        oledC_DrawString(10, 45, 1, 1, "Set Date", OLEDC_COLOR_BLACK);
        oledC_DrawString(10, 55, 1, 1, "Set Alarm", OLEDC_COLOR_BLACK);
        oledC_DrawThickPoint(4, point, 4, OLEDC_COLOR_BLACK);

        while(!checkLed2Pressed()){
            if (checkLed1Pressed()){
                handleLed1();
                oledC_DrawThickPoint(4, point, 4, OLEDC_COLOR_WHITE);
                point = (point == 60) ? 30 : point + 10;
                oledC_DrawThickPoint(4, point, 4, OLEDC_COLOR_BLACK);
                handleLed1();
            }
        }
        handleLed2();
        handleLed2();

        switch (point) {
            case 30:
                oledC_DrawRectangle(45, 0, 95, 25, OLEDC_COLOR_WHITE);
                oledC_DrawRectangle(0, 20, 95, 80, OLEDC_COLOR_WHITE);
                flag = FALSE;
              break;

            case 40:
                setTime();
              break;

            case 50:
                setDate();
              break;

            default:
                setAlarm();
              break;
        }
    }
    handleLed2();
}

int checkLed1Pressed(void) {
    return PORTAbits.RA11 == 0;
}

int checkLed2Pressed(void) {
    return PORTAbits.RA12 == 0;
}

void handleLed1(){
    LATAbits.LATA8 = 1;
    DELAY_milliseconds(100);
    LATAbits.LATA8 = 0;
}

void handleLed2(){
    LATAbits.LATA9 = 1;
    DELAY_milliseconds(200);
    LATAbits.LATA9 = 0;
}


int main(void) {
    SYSTEM_Initialize();
    oledC_setBackground(OLEDC_COLOR_WHITE);
    oledC_clearScreen();

    initDate();
    displayDate();
    
    
oledC_DrawLine(45, 45, 55, 55, 2, OLEDC_COLOR_BLACK);

//    while(1) {
//        if (checkLed1Pressed()){
//            handleLed1();
//            if (counter > 1)
//                displayMenu();
//        }
//    }
}

