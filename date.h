#ifndef XV6_DATE_H
#define XV6_DATE_H

struct rtcdate {
  unsigned int second;
  unsigned int minute;
  unsigned int hour;
  unsigned int day;
  unsigned int month;
  unsigned int year;
};

#endif // XV6_DATE_H
