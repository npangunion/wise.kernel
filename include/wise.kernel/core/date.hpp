#pragma once 

#include <stdint.h>

namespace wise {
namespace kernel {


//! \brief A type for representing date data.
struct date
{
	int16_t year = 0; //!< Year [0-inf).
	int16_t month = 0; //!< Month of the year [1-12].
	int16_t day = 0; //!< Day of the month [1-31].
};

//! \brief A type for representing timestamp data.
struct timestamp
{
	int16_t year = 0;   //!< Year [0-inf).
	int16_t month = 0;  //!< Month of the year [1-12].
	int16_t day = 0;    //!< Day of the month [1-31].
	int16_t hour = 0;   //!< Hours since midnight [0-23].
	int16_t min = 0;    //!< Minutes after the hour [0-59].
	int16_t sec = 0;    //!< Seconds after the minute.
	int32_t fract = 0;  //!< Fractional seconds.
};

} // kernel
} // wise