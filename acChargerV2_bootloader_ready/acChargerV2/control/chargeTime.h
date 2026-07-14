#ifndef CHARGETIME_H_
#define CHARGETIME_H_


#define TIME_SECOND_MAX 60
#define TIME_MINUTE_MAX 60


typedef struct {
    unsigned int second;
    unsigned int minute;
    unsigned int hours;
} t_time;

t_time time;



//-- bu fonksiyon her bir çalýþtýrýldýðýnda bir saniye saymaktadýr
void updateTime(t_time *time) {
    ++time->second;

    if (time->second == TIME_SECOND_MAX) {
        ++time->minute;
        time->second = 0;
    }

    if (time->minute == TIME_MINUTE_MAX) {
        ++time->hours;
        time->minute = 0;
    }
}



#endif /* CHARGETIME_H_ */
