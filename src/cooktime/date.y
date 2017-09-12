/*
 * cook - file construction tool
 * Copyright (C) 1993-1995, 1997-2001, 2006, 2007 Peter Miller;
 * All rights reserved.
 *      Peter Miller <millerp@canb.auug.org.au>
 *
 * This code is derived from code which is
 * Copyright (C) 1986 Steven M. Bellovin
 *      Steven Bellovin <smb@cs.unc.edu>
 */

%token  AGO
%token  COLON
%token  COMMA
%token  DAY
%token  DAYZONE
%token  ID
%token  JUNK
%token  MERIDIAN
%token  MONTH
%token  MUNIT
%token  NUMBER
%token  SLASH
%token  SUNIT
%token  UNIT
%token  ZONE

%{

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/time.h>
#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <cooktime/date.h>
#include <common/str.h>
#include <common/trace.h>

#define YYSTYPE int

#define daysec (24L * 60L * 60L)

#define AM 1
#define PM 2

#define DAYLIGHT 1
#define STANDARD 2
#define MAYBE    3

#define MAX_ID_LENGTH 20

static  int     timeflag;
static  int     zoneflag;
static  int     dateflag;
static  int     dayflag;
static  int     relflag;
static  time_t  relsec;
static  time_t  relmonth;
static  int     hh;
static  int     mm;
static  int     ss;
static  int     merid;
static  int     day_light_flag;
static  int     dayord;
static  int     dayreq;
static  int     month;
static  int     day;
static  int     year;
static  int     ourzone;
static  char    *lptr;
extern  YYSTYPE yylval;
extern  int     yydebug;


static int mdays[12] =
{
    31, 0, 31,  30, 31, 30,  31, 31, 30,  31, 30, 31
};

#define epoch 1970

int yyparse(void); /* forward */
static int yylex(void); /* forward */


/*
 * NAME
 *      timeconv - convert a time
 *
 * SYNOPSIS
 *      time_t timeconv(int hh, int mm, int ss, int mer);
 *
 * DESCRIPTION
 *      The timeconv function is used to convert a time
 *      specified in hours minutes and seconds, into seconds past midnight.
 *
 * ARGUMENTS
 *      hh      hours, range depends on the meridian
 *      mm      minutes, 0..59
 *      ss      seconds, 0..59
 *      mer     meridian to use: AM, PM or 24
 *
 * RETURNS
 *      time_t; seconds past midnight; -1 on any error.
 */

static time_t
timeconv(int ahh, int amm, int ass, int mer)
{
    time_t          result;

    /*
     * perform sanity checks on input
     */
    trace(("timeconv(ahh = %d, amm = %d, ass = %d, mer = %d)\n{\n",
        ahh, amm, ass, mer));
    result = -1;
    if (amm < 0 || amm > 59 || ass < 0 || ass > 59)
        goto done;

    /*
     * perform range checks depending on the meridian
     */
    switch (mer)
    {
    case AM:
        if (ahh < 1 || ahh > 12)
            goto done;
        if (ahh == 12)
            ahh = 0;
        break;

    case PM:
        if (ahh < 1 || ahh > 12)
            goto done;
        if (ahh == 12)
            ahh = 0;
        ahh += 12;
        break;

    case 24:
        if (ahh < 0 || ahh > 23)
            goto done;
        break;

    default:
        goto done;
    }
    result = ((ahh * 60L + amm) * 60L + ass);
    done:
    trace(("return %ld;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      dateconv - convert a date
 *
 * SYNOPSIS
 *      time_t dateconv(int mm, int dd, int year, int h, int m, int s,
 *              int mer, int zone, int dayflag);
 *
 * DESCRIPTION
 *      The dateconv function may be used to convert a date after the
 *      date string has been taken apart by yyparse.
 *
 * ARGUMENTS
 *      mm      month number, in the range 1..12
 *      year    year number,  in several ranges:
 *              0..37 means 2000..2037
 *              70..99 means 1970..1999
 *              1970..2037 mean themselves.
 *      dd      day of month, in the range 1..max, where max varies for
 *              each month, as per the catchy jingle (except February,
 *              which is a monster).
 *      h       hours since midnight or meridian
 *      m       minutes past hour
 *      s       seconds past minute
 *      mer     meridian, AM or PM.
 *      zone    minutes correction for the time zone.
 *      dayflag whether to use daylight savings: STANDARD, DAYLIGHT or MAYBE.
 *
 * RETURNS
 *      time_t; the time in seconds past Jan 1 0:00:00 1970 GMT, this will
 *      always be positive or zero; -1 is returned for any error.
 *
 * CAVEAT
 *      The date functions only work between 1970 and 2037,
 *      because 0 is Jan 1 00:00:00 1970 GMT
 *      and (2^31-1) is Jan 19 03:14:07 2038 GMT
 *      hence some if the weir magic number below.
 *
 *      Because -1 is used to represent errors, times before noon Jan 1 1970
 *      in places east of GMT can't always be represented.
 */

static time_t
dateconv(int amm, int dd, int ayear, int h, int m, int s, int mer, int zone,
    int adayflag)
{
    time_t          result;
    time_t          tod;
    time_t          jdate;
    int             i;

    /*
     * make corrections for the year
     *
     * If it is 0..99, RFC822 says pick closest century.
     */
    trace(("dateconv(amm = %d, dd = %d, ayear = %d, h = %d, m = %d, s = %d, "
        "mer = %d, zone = %d, adayflag = %d)\n{\n", amm, dd, ayear, h, m, s,
        mer, zone, adayflag));
    result = -1;
    if (ayear < 0)
        ayear = -ayear;
    if (ayear < 38)
        ayear += 2000;
    else if (ayear < 100)
        ayear += 1900;

    /*
     * correct February length once we know the year
     */
    mdays[1] = 28 + (ayear % 4 == 0 && (ayear % 100 != 0 || ayear % 400 == 0));

    /*
     * perform some sanity checks on the input
     */
    if
    (
        ayear < epoch
    ||
        ayear >= 2038
    ||
        amm < 1
    ||
        amm > 12
    ||
        dd < 1
    ||
        dd > mdays[--amm]
    )
        goto done;

    /*
     * Determine the julian day number of the dd-mm-yy given.
     * Turn it into seconds, and add in the time zone correction.
     */
    jdate = dd - 1;
    for (i = 0; i < amm; i++)
        jdate += mdays[i];
    for (i = epoch; i < ayear; i++)
        jdate += 365 + (i % 4 == 0);
    jdate *= daysec;
    jdate += zone * 60L;

    /*
     * Determine the time of day.
     * that is, seconds from midnight.
     * Add it into the julian date.
     */
    tod = timeconv(h, m, s, mer);
    if (tod < 0)
        goto done;
    jdate += tod;

    /*
     * Perform daylight savings correction if necessary.
     * (This assumes 1 hour daylite savings, which is probably wrong.)
     */
    if
    (
        adayflag == DAYLIGHT
    ||
        (adayflag == MAYBE && localtime(&jdate)->tm_isdst)
    )
        jdate += -1 * 60 * 60;

    /*
     * there you have it.
     */
    result = jdate;
    done:
    trace(("return %ld;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      daylcorr
 *
 * SYNOPSIS
 *      time_t daylcorr(time_t future, time_t now);
 *
 * DESCRIPTION
 *      The daylcorr function is used to determine the difference in seconds
 *      between two times, taking daylight savings into account.
 *
 * ARGUMENTS
 *      future  - a later time
 *      now     - an earlier time
 *
 * RETURNS
 *      time_t; the difference in seconds
 *
 * CAVEAT
 *      Assumes daylight savings is alays an integral number of hours.
 *      This is wrong is Saudi Arabia (time zone changes during the day),
 *      and South Australia (half hour DLS).
 */

static time_t
daylcorr(time_t future, time_t now)
{
    int             fdayl;
    int             nowdayl;
    time_t          result;

    trace(("daylcorr(future = %ld, now = %ld)\n{\n", (long)future, (long)now));
    nowdayl = (localtime(&now)->tm_hour + 1) % 24;
    fdayl = (localtime(&future)->tm_hour + 1) % 24;
    result = ((future - now) + 60L * 60L * (nowdayl - fdayl));
    trace(("return %ld;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      dayconv
 *
 * SYNOPSIS
 *      time_t dayconv(int ord, int day, time_t now);
 *
 * DESCRIPTION
 *      The dayconv function is used to convert a day-of-the-week into
 *      a meaningful time.
 *
 * ARGUMENTS
 *      ord     - the ord'th day from now
 *      day     - which day of the week
 *      now     - relative to this
 *
 * RETURNS
 *      time_t; time in seconds from epoch
 */

static time_t
dayconv(int ord, int aday, time_t now)
{
    time_t          tod;
    time_t          result;

    trace(("dayconv(ord = %d, aday = %d, now = %ld)\n{\n", ord, aday,
        (long)now));
    tod = now;
    tod += daysec * ((aday - localtime(&tod)->tm_wday + 7) % 7);
    tod += 7 * daysec * (ord <= 0 ? ord : ord - 1);
    result = daylcorr(tod, now);
    trace(("return %ld;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      monthadd
 *
 * SYNOPSIS
 *      time_t monthadd(time_t sdate, time_t relmonth);
 *
 * DESCRIPTION
 *      The monthadd function is used to add a given number of
 *      months to a specified time.
 *
 * ARGUMENTS
 *      sdate   - add the months to this
 *      relmonth - add this many months
 *
 * RETURNS
 *      time_t; seconds since the epoch
 */

static time_t
monthadd(time_t sdate, time_t arelmonth)
{
    struct tm       *ltime;
    int             amm;
    int             ayear;
    time_t          result;

    trace(("monthadd(sdate = %ld, arelmonth = %ld)\n{\n", (long)sdate,
        (long)arelmonth));
    if (arelmonth == 0)
        result = 0;
    else
    {
        ltime = localtime(&sdate);
        amm = 12 * (ltime->tm_year + 1900) + ltime->tm_mon + arelmonth;
        ayear = amm / 12;
        amm = amm % 12 + 1;
        result =
            dateconv
            (
                amm,
                ltime->tm_mday,
                ayear,
                ltime->tm_hour,
                ltime->tm_min,
                ltime->tm_sec,
                24,
                ourzone,
                MAYBE
            );
        if (result >= 0)
            result = daylcorr(result, sdate);
    }
    trace(("return %ld;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      date_scan
 *
 * SYNOPSIS
 *      time_t date_scan(char *s);
 *
 * DESCRIPTION
 *      The date_scan function is used to scan a string and
 *      return a number of seconds since epoch.
 *
 * ARGUMENTS
 *      s       - string to scan
 *
 * RETURNS
 *      time_t; seconds to epoch, -1 on error.
 *
 * CAVEAT
 *      it isn't psychic
 */

time_t
date_scan(char *p)
{
    time_t          now;
    struct tm       *lt;
    time_t          result;
    time_t          tod;

    /*
     * find time zone info, if not given
     */
    trace(("date_scan(p = \"%s\")\n{\n", p));
    lptr = p;

    /*
     * initialize things
     */
    time(&now);
    lt = localtime(&now);
    year = lt->tm_year + 1900;
    month = lt->tm_mon + 1;
    day = lt->tm_mday;
    relsec = 0;
    relmonth = 0;
    timeflag = 0;
    zoneflag = 0;
    dateflag = 0;
    dayflag = 0;
    relflag = 0;
    ourzone = 0;
    day_light_flag = MAYBE;
    hh = 0;
    mm = 0;
    ss = 0;
    merid = 24;

    /*
     * parse the string
     */
#ifdef DEBUG
    yydebug = trace_pretest_;
#endif
    trace(("yyparse()\n{\n"));
    result = yyparse();
    trace(("}\n"));
    if (result)
    {
        result = -1;
        goto done;
    }

    /*
     * sanity checks
     */
    result = -1;
    if (timeflag > 1 || zoneflag > 1 || dateflag > 1 || dayflag > 1)
        goto done;

    if (dateflag || timeflag || dayflag)
    {
        result =
            dateconv
            (
                month,
                day,
                year,
                hh,
                mm,
                ss,
                merid,
                ourzone,
                day_light_flag
            );
        if (result < 0)
            goto done;
    }
    else
    {
        result = now;
        if (!relflag)
        {
            result -= ((lt->tm_hour * 60L + lt->tm_min * 60) + lt->tm_sec);
        }
    }

    result += relsec;
    relsec = monthadd(result, relmonth);
    if (relsec < 0)
    {
        result = -1;
        goto done;
    }
    result += relsec;

    if (dayflag && !dateflag)
    {
        tod = dayconv(dayord, dayreq, result);
        result += tod;
    }

    /*
     * here for all exits
     */
    done:
    trace(("return %ld;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      date_string - build one
 *
 * SYNOPSIS
 *      char *date_string(time_t when);
 *
 * DESCRIPTION
 *      The date_string function may be used to construct a
 *      string from a given time in seconds.
 *
 *      The string will conform to the RFC822 standard,
 *      which states a definite preference for GMT dates.
 *
 * ARGUMENTS
 *      when    the time to be rendered.
 *
 * RETURNS
 *      Pointer to string containing rendered time.
 *      The contents of this string will remain undisturbed
 *      only until the next call to date_string.
 */

char *
date_string(time_t when)
{
    struct tm       *tm;
    static char     buffer[32];

    static char    *weekday_name[] =
    {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
    };

    static char    *month_name[] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    /*
     * break the given time down into components
     *      (RFC1036 likes GMT, remember)
     */
    trace(("date_string(when = %ld)\n{\n", (long)when));
    tm = gmtime(&when);

    /*
     * build the date string
     */
    snprintf
    (
        buffer,
        sizeof(buffer),
        "%s,%3d %s %4.4d %2.2d:%2.2d:%2.2d GMT",
        weekday_name[tm->tm_wday],
        tm->tm_mday,
        month_name[tm->tm_mon],
        tm->tm_year + 1900,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec
    );
    trace(("return \"%s\";\n", buffer));
    trace(("}\n"));
    return buffer;
}


/*
 * NAME
 *      yyerror
 *
 * SYNOPSIS
 *      void yyerror(char *);
 *
 * DESCRIPTION
 *      The yyerror function is invoked by yacc to report
 *      errors, but we just throw it away.
 *
 * ARGUMENTS
 *      s       - error to report
 */

static void
yyerror(char *s)
{
    trace(("yyerror(s = \"%s\")\n{\n", s));
    (void)s;
    trace(("}\n"));
}


/*
 * NAME
 *      yytrace - follow parser actions
 *
 * SYNOPSIS
 *      void yytrace(char *, ...);
 *
 * DESCRIPTION
 *      The yytrace function is used to print the various shifts
 *      and reductions, etc, done by the yacc-generated parser.
 *      lines are accumulated and printed whole,
 *      so as to avoid confusing the trace output.
 *
 * ARGUMENTS
 *      as for printf
 *
 * CAVEAT
 *      only available when DEBUG is defined
 */

#ifdef DEBUG
#define YYDEBUG 1

#define printf yytrace

static void
yytrace(char *s, ...)
{
    va_list         ap;
    static char     line[1024];
    string_ty       *buffer;
    char            *cp;

    sva_init(ap, s);
    buffer = str_vformat(s, ap);
    va_end(ap);
    cp = line + strlen(line);
    cp = strendcpy(cp, buffer->str_text, line + sizeof(line));
    str_free(buffer);
    if (cp > line && cp[-1] == '\n')
    {
        --cp;
        *cp = 0;
        trace_printf("%s\n", line);
        line[0] = 0;
    }
}

#endif /* DEBUG */

%}

%%

timedate
    : /* empty */
    | timedate item
    | error
        {
            /*
             * Mostly, this production is unnecessary,
             * however it silences warnings about unused
             * labels, etc.
             */
            return -1;
        }
    ;

item
    : TimeSpecification
        { timeflag++; }
    | TimeZone
        { zoneflag++; }
    | DateSpecification
        { dateflag++; }
    | DayOfWeekSpecification
        { dayflag++; }
    | RelativeSpecification
        { relflag++; }
    | NumberSpecification
    ;

NumberSpecification
    : NUMBER
        {
            if (timeflag && dateflag && !relflag)
                year = $1;
            else
            {
                timeflag++;
                hh = $1 / 100;
                mm = $1 % 100;
                ss = 0;
                merid = 24;
            }
        }
    ;

TimeSpecification
    : NUMBER MERIDIAN
        {
            hh = $1;
            mm = 0;
            ss = 0;
            merid = $2;
        }
    | NUMBER COLON NUMBER
        {
            hh = $1;
            mm = $3;
            merid = 24;
        }
    | NUMBER COLON NUMBER MERIDIAN
        {
            hh = $1;
            mm = $3;
            merid = $4;
        }
    | NUMBER COLON NUMBER NUMBER
        {
            hh = $1;
            mm = $3;
            merid = 24;
            day_light_flag = STANDARD;
            $4 = -$4;
            ourzone = $4 % 100 + 60 * $4 / 100;
        }
    | NUMBER COLON NUMBER COLON NUMBER
        {
            hh = $1;
            mm = $3;
            ss = $5;
            merid = 24;
        }
    | NUMBER COLON NUMBER COLON NUMBER MERIDIAN
        {
            hh = $1;
            mm = $3;
            ss = $5;
            merid = $6;
        }
    | NUMBER COLON NUMBER COLON NUMBER NUMBER
        {
            hh = $1;
            mm = $3;
            ss = $5;
            merid = 24;
            day_light_flag = STANDARD;
            $6 = -$6;
            ourzone = $6 % 100 + 60 * $6 / 100;
        }
    ;

TimeZone
    : ZONE
        {
            ourzone = $1;
            day_light_flag = STANDARD;
        }
    | DAYZONE
        {
            ourzone = $1;
            day_light_flag = DAYLIGHT;
        }
    ;

DayOfWeekSpecification
    : DAY
        {
            dayord = 1;
            dayreq = $1;
        }
    | DAY COMMA
        {
            dayord = 1;
            dayreq = $1;
        }
    | NUMBER DAY
        {
            dayord = $1;
            dayreq = $2;
        }
    ;

DateSpecification
    : NUMBER SLASH NUMBER
        {
            month = $1;
            day = $3;
        }
    | NUMBER SLASH NUMBER SLASH NUMBER
        {
            month = $1;
            day = $3;
            year = $5;
        }
    | MONTH NUMBER
        {
            month = $1;
            day = $2;
        }
    | MONTH NUMBER COMMA NUMBER
        {
            month = $1;
            day = $2;
            year = $4;
        }
    | NUMBER MONTH
        {
            month = $2;
            day = $1;
        }
    | NUMBER MONTH NUMBER
        {
            month = $2;
            day = $1;
            year = $3;
        }
    ;

RelativeSpecification
    : NUMBER UNIT
        { relsec +=  60L * $1 * $2; }
    | NUMBER MUNIT
        { relmonth += $1 * $2; }
    | NUMBER SUNIT
        { relsec += $1; }
    | UNIT
        { relsec +=  60L * $1; }
    | MUNIT
        { relmonth += $1; }
    | SUNIT
        { relsec++; }
    | RelativeSpecification AGO
        {
            relsec = -relsec;
            relmonth = -relmonth;
        }
    ;

%%

/* The following need to be here to make sure Berkeley Yacc doesn't puke. */


/*
 * NAME
 *      table - list of known names
 *
 * SYNOPSIS
 *      table_t table[];
 *
 * DESCRIPTION
 *      The table is used to hold the list of known names.
 *      This includes time zone names and days of the week, etc.
 *
 * CAVEAT
 *      It is in English.
 *      It is impossible to have a full list of time zones.
 */

typedef struct table_t table_t;
struct table_t
{
    char        *name;
    int         type;
    int         value;
};

#define HRMIN(a, b) ((a) * 60 + (b))

static table_t table[] =
{
    { "a",      ZONE,       HRMIN(1, 0),    },
    { "a.c.s.s.t.", DAYZONE,    -HRMIN(9, 30),  },
    { "a.c.s.t.",   ZONE,       -HRMIN(9, 30),  },
    { "a.d.t.",     DAYZONE,    HRMIN(4, 0),    },
    { "a.e.s.s.t.", DAYZONE,    -HRMIN(10, 0),  },
    { "a.e.s.t.",   ZONE,       -HRMIN(10, 0),  },
    { "a.m.",       MERIDIAN,       AM,         },
    { "a.s.t.",     ZONE,       HRMIN(4, 0),    },
    { "a.w.s.t.", ZONE, -HRMIN(8, 0), }, /* (no daylight time there, I'm told */
    { "acsst",    DAYZONE, -HRMIN(9, 30), }, /* Australian Central Summer */
    { "acst",       ZONE,    -HRMIN(9, 30), }, /* Australian Central Time */
    { "adt",    DAYZONE,    HRMIN(4, 0),    },
    { "aesst", DAYZONE, -HRMIN(10, 0), }, /* Australian Eastern Summer Time */
    { "aest",       ZONE,    -HRMIN(10, 0), }, /* Australian Eastern Time */
    { "ago",    AGO,        1,          },
    { "am",     MERIDIAN,       AM,         },
    { "apr",    MONTH,      4,          },
    { "apr.",       MONTH,      4,          },
    { "april",      MONTH,      4,          },
    { "ast",    ZONE,       HRMIN(4, 0),    },  /* Atlantic */
    { "aug",    MONTH,      8,          },
    { "aug.",       MONTH,      8,          },
    { "august",     MONTH,      8,          },
    { "awst",       ZONE,     -HRMIN(8, 0), }, /* Australian Western Time */
    { "b",      ZONE,       HRMIN(2, 0),    },
    { "b.s.t.",     DAYZONE,    HRMIN(0, 0),    },
    { "bst",    DAYZONE,       HRMIN(0, 0), }, /* British Summer Time */
    { "c",      ZONE,       HRMIN(3, 0),    },
    { "c.d.t.",     DAYZONE,    HRMIN(6, 0),    },
    { "c.s.t.",     ZONE,       HRMIN(6, 0),    },
    { "cdt",    DAYZONE,    HRMIN(6, 0),    },
    { "cst",    ZONE,       HRMIN(6, 0),    }, /* Central */
    { "d",      ZONE,       HRMIN(4, 0),    },
    { "day",    UNIT,       1 * 24 * 60,    },
    { "days",       UNIT,       1 * 24 * 60,    },
    { "dec",    MONTH,      12,         },
    { "dec.",       MONTH,      12,         },
    { "december",   MONTH,      12,         },
    { "e",      ZONE,       HRMIN(5, 0),    },
    { "e.d.t.",     DAYZONE,    HRMIN(5, 0),    },
    { "e.e.s.t.",   DAYZONE,    HRMIN(0, 0),    },
    { "e.e.t.",     ZONE,       HRMIN(0, 0),    },
    { "e.s.t.",     ZONE,       HRMIN(5, 0),    },
    { "edt",    DAYZONE,    HRMIN(5, 0),    },
    { "eest",    DAYZONE, HRMIN(0, 0), }, /* European Eastern Summer Time */
    { "eet",    ZONE,    HRMIN(0, 0), }, /* European Eastern Time */
    { "eigth",      NUMBER,     8,          },
    { "eleventh",   NUMBER,     11,         },
    { "est",    ZONE,       HRMIN(5, 0),    }, /* Eastern */
    { "f",      ZONE,       HRMIN(6, 0),    },
    { "feb",    MONTH,      2,          },
    { "feb.",       MONTH,      2,          },
    { "february",   MONTH,      2,          },
    { "fifth",      NUMBER,     5,          },
    { "first",      NUMBER,     1,          },
    { "fortnight",  UNIT,       14 * 24 * 60,   },
    { "fortnights", UNIT,       14 * 24 * 60,   },
    { "fourth",     NUMBER,     4,          },
    { "fri",    DAY,        5,          },
    { "fri.",       DAY,        5,          },
    { "friday",     DAY,        5,          },
    { "g",      ZONE,       HRMIN(7, 0),    },
    { "g.m.t.",     ZONE,       HRMIN(0, 0),    },
    { "gmt",    ZONE,       HRMIN(0, 0),    },
    { "h",      ZONE,       HRMIN(8, 0),    },
    { "h.d.t.",     DAYZONE,    HRMIN(10, 0),   },
    { "h.s.t.",     ZONE,       HRMIN(10, 0),   },
    { "hdt",    DAYZONE,    HRMIN(10, 0),   },
    { "hour",       UNIT,       60,         },
    { "hours",      UNIT,       60,         },
    { "hr",     UNIT,       60,         },
    { "hrs",    UNIT,       60,         },
    { "hst",    ZONE,       HRMIN(10, 0),   }, /* Hawaii */
    { "i",      ZONE,       HRMIN(9, 0),    },
    { "j.s.t.",     ZONE,     -HRMIN(9, 0), }, /* Japan Standard Time */
    { "jan",    MONTH,      1,          },
    { "jan.",       MONTH,      1,          },
    { "january",    MONTH,      1,          },
    { "jst",    ZONE,     -HRMIN(9, 0), }, /* Japan Standard Time */
    { "jul",    MONTH,      7,          },
    { "jul.",       MONTH,      7,          },
    { "july",       MONTH,      7,          },
    { "jun",    MONTH,      6,          },
    { "jun.",       MONTH,      6,          },
    { "june",       MONTH,      6,          },
    { "k",      ZONE,       HRMIN(10, 0),   },
    { "l",      ZONE,       HRMIN(11, 0),   },
    { "last",       NUMBER,     -1,         },
    { "m",      ZONE,       HRMIN(12, 0),   },
    { "m.d.t.",     DAYZONE,    HRMIN(7, 0),    },
    { "m.e.s.t.",   DAYZONE,    -HRMIN(1, 0),   },
    { "m.e.t.",     ZONE,       -HRMIN(1, 0),   },
    { "m.s.t.",     ZONE,       HRMIN(7, 0),    },
    { "mar",    MONTH,      3,          },
    { "mar.",       MONTH,      3,          },
    { "march",      MONTH,      3,          },
    { "may",    MONTH,      5,          },
    { "mdt",    DAYZONE,    HRMIN(7, 0),    },
    { "mest",    DAYZONE, -HRMIN(1, 0), }, /* Middle European Summer Time */
    { "met",    ZONE,    -HRMIN(1, 0), }, /* Middle European Time */
    { "min",    UNIT,       1,          },
    { "mins",       UNIT,       1,          },
    { "minute",     UNIT,       1,          },
    { "minutes",    UNIT,       1,          },
    { "mon",    DAY,        1,          },
    { "mon.",       DAY,        1,          },
    { "monday",     DAY,        1,          },
    { "month",      MUNIT,      1,          },
    { "months",     MUNIT,      1,          },
    { "mst",    ZONE,       HRMIN(7, 0),    }, /* Mountain */
    { "n",      ZONE,       -HRMIN(1, 0),   },
    { "n.s.t.",     ZONE,       HRMIN(3, 30),   },
    { "next",       NUMBER,     2,          },
    { "ninth",      NUMBER,     9,          },
    { "nov",    MONTH,      11,         },
    { "nov.",       MONTH,      11,         },
    { "november",   MONTH,      11,         },
    { "now",    UNIT,       0,          },
    { "nst",    ZONE,       HRMIN(3, 30),   }, /* Newfoundland */
    { "o",      ZONE,       -HRMIN(2, 0),   },
    { "oct",    MONTH,      10,         },
    { "oct.",       MONTH,      10,         },
    { "october",    MONTH,      10,         },
    { "p",      ZONE,       -HRMIN(3, 0),   },
    { "p.d.t.",     DAYZONE,    HRMIN(8, 0),    },
    { "p.m.",       MERIDIAN,       PM,         },
    { "p.s.t.",     ZONE,       HRMIN(8, 0),    },
    { "pdt",    DAYZONE,    HRMIN(8, 0),    },
    { "pm",     MERIDIAN,       PM,         },
    { "pst",    ZONE,       HRMIN(8, 0),    }, /* Pacific */
    { "q",      ZONE,       -HRMIN(4, 0),   },
    { "r",      ZONE,       -HRMIN(5, 0),   },
    { "s",      ZONE,       -HRMIN(6, 0),   },
    { "sat",    DAY,        6,          },
    { "sat.",       DAY,        6,          },
    { "saturday",   DAY,        6,          },
    { "sec",    SUNIT,      1,          },
    { "second",     SUNIT,      1,          },
    { "seconds",    SUNIT,      1,          },
    { "secs",       SUNIT,      1,          },
    { "sep",    MONTH,      9,          },
    { "sep.",       MONTH,      9,          },
    { "sept",       MONTH,      9,          },
    { "sept.",      MONTH,      9,          },
    { "september",  MONTH,      9,          },
    { "seventh",    NUMBER,     7,          },
    { "sixth",      NUMBER,     6,          },
    { "sun",    DAY,        0,          },
    { "sun.",       DAY,        0,          },
    { "sunday",     DAY,        0,          },
    { "t",      ZONE,       -HRMIN(7, 0),   },
    { "tenth",      NUMBER,     10,         },
    { "third",      NUMBER,     3,          },
    { "this",       UNIT,       0,          },
    { "thu",    DAY,        4,          },
    { "thu.",       DAY,        4,          },
    { "thur",       DAY,        4,          },
    { "thur.",      DAY,        4,          },
    { "thurs",      DAY,        4,          },
    { "thurs.",     DAY,        4,          },
    { "thursday",   DAY,        4,          },
    { "today",      UNIT,       0,          },
    { "tomorrow",   UNIT,       1 * 24 * 60,    },
    { "tue",    DAY,        2,          },
    { "tue.",       DAY,        2,          },
    { "tues",       DAY,        2,          },
    { "tues.",      DAY,        2,          },
    { "tuesday",    DAY,        2,          },
    { "twelfth",    NUMBER,     12,         },
    { "u",      ZONE,       -HRMIN(8, 0),   },
    { "u.t.",       ZONE,       HRMIN(0, 0),    },
    { "ut",     ZONE,       HRMIN(0, 0),    },
    { "v",      ZONE,       -HRMIN(9, 0),   },
    { "w",      ZONE,       -HRMIN(10, 0),  },
    { "w.e.s.t.",   DAYZONE,    -HRMIN(2, 0),   },
    { "w.e.t.",     ZONE,       -HRMIN(2, 0),   },
    { "wed",    DAY,        3,          },
    { "wed.",       DAY,        3,          },
    { "wednes",     DAY,        3,          },
    { "wednes.",    DAY,        3,          },
    { "wednesday",  DAY,        3,          },
    { "week",       UNIT,       7 * 24 * 60,    },
    { "weeks",      UNIT,       7 * 24 * 60,    },
    { "west",   DAYZONE, -HRMIN(2, 0), }, /* Western European Summer Time */
    { "wet",    ZONE,       -HRMIN(2, 0), }, /* Western European Time */
    { "x",      ZONE,       -HRMIN(11, 0),  },
    { "y",      ZONE,       -HRMIN(12, 0),  },
    { "y.d.t.",     DAYZONE,    HRMIN(9, 0),    },
    { "y.s.t.",     ZONE,       HRMIN(9, 0),    },
    { "ydt",    DAYZONE,    HRMIN(9, 0),    },
    { "year",       MUNIT,      12,         },
    { "years",      MUNIT,      12,         },
    { "yesterday",  UNIT,       -1*24*60,       },
    { "yst",    ZONE,       HRMIN(9, 0),    }, /* Yukon */
    { "z",      ZONE,       HRMIN(0, 0),    },
};


/*
 * NAME
 *      lookup - find name
 *
 * SYNOPSIS
 *      int lookup(char *id);
 *
 * DESCRIPTION
 *      The lookup function is used to find a token corresponding to
 *      a given name.
 *
 * ARGUMENTS
 *      id      - name to search for.  Assumes already downcased.
 *
 * RETURNS
 *      int; yacc token, ID if not found.
 */

static int
lookup(char *id)
{
    table_t     *tp;
    int         min;
    int         max;
    int         mid;
    int         cmp;
    int         result;

    /*
     * binary chop the table
     */
    trace(("lookup(id = \"%s\")\n{\n", id));
    result = ID;
    min = 0;
    max = SIZEOF(table) - 1;
    while (min <= max)
    {
        mid = (min + max) / 2;
        tp = table + mid;
        cmp = strcmp(id, tp->name);
        if (!cmp)
        {
            yylval = tp->value;
            result = tp->type;
            break;
        }
        if (cmp < 0)
            max = mid - 1;
        else
            min = mid + 1;
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      yylex - lexical analyser
 *
 * SYNOPSIS
 *      int yylex(void);
 *
 * DESCRIPTION
 *      The yylex function is used to scan the input string
 *      and break it into discrete tokens.
 *
 * RETURNS
 *      int; the yacc token, 0 means the-end.
 */

static int
yylex(void)
{
    int         sign;
    int         c;
    char        *p;
    char        idbuf[MAX_ID_LENGTH];
    int         pcnt;
    int         token;

    trace(("yylex()\n{\n"));
    yylval = 0;
    for (;;)
    {
        /*
         * get the next input character
         */
        c = *lptr++;

        /*
         * action depends on the character
         */
        switch (c)
        {
        case 0:
            token = 0;
            lptr--;
            break;

        case ' ':
        case '\t':
            /*
             * ignore white space
             */
            continue;

        case ':':
            token = COLON;
            break;

        case ',':
            token = COMMA;
            break;

        case '/':
            token = SLASH;
            break;

        case '-':
            if (!isdigit(*lptr))
            {
                /*
                 * ignore lonely '-'s
                 */
                continue;
            }
            sign = -1;
            c = *lptr++;
            goto number;

        case '+':
            if (!isdigit(*lptr))
            {
                token = c;
                break;
            }
            sign = 1;
            c = *lptr++;
            goto number;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            /*
             * numbers
             */
            sign = 1;
          number:
            for (;;)
            {
                yylval = yylval * 10 + c - '0';
                c = *lptr++;
                switch (c)
                {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    continue;
                }
                break;
            }
            yylval *= sign;
            lptr--;
            token = NUMBER;
            break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
            /*
             * name
             */
            p = idbuf;
            for (;;)
            {
                if (isupper(c))
                    c = tolower(c);
                if (p < idbuf + sizeof(idbuf) - 1)
                    *p++ = c;
                c = *lptr++;
                switch (c)
                {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                case 'h':
                case 'i':
                case 'j':
                case 'k':
                case 'l':
                case 'm':
                case 'n':
                case 'o':
                case 'p':
                case 'q':
                case 'r':
                case 's':
                case 't':
                case 'u':
                case 'v':
                case 'w':
                case 'x':
                case 'y':
                case 'z':
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                case 'G':
                case 'H':
                case 'I':
                case 'J':
                case 'K':
                case 'L':
                case 'M':
                case 'N':
                case 'O':
                case 'P':
                case 'Q':
                case 'R':
                case 'S':
                case 'T':
                case 'U':
                case 'V':
                case 'W':
                case 'X':
                case 'Y':
                case 'Z':
                case '.':
                    continue;
                }
                break;
            }
            *p = 0;
            lptr--;
            token = lookup(idbuf);
            break;

        case '(':
            /*
             * comment
             */
            for (pcnt = 1; pcnt > 0;)
            {
                c = *lptr++;
                switch (c)
                {
                case 0:
                    --lptr;
                    pcnt = 0;
                    break;

                case '(':
                    pcnt++;
                    break;

                case ')':
                    pcnt--;
                    break;
                }
            }
            continue;

        default:
            /*
             * unrecognosed
             */
            token = JUNK;
            break;
        }
        break;
    }
    trace(("yylval = %d;\n", yylval));
    trace(("return %d;\n", token));
    trace(("}\n"));
    return token;
}
