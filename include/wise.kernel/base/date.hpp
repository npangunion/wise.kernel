#pragma once 

#include <stdint.h>

namespace wise
{


//! \brief A type for representing date data.
struct date
{
	std::int16_t year = 0; //!< Year [0-inf).
	std::int16_t month = 0; //!< Month of the year [1-12].
	std::int16_t day = 0; //!< Day of the month [1-31].
};

//! \brief A type for representing timestamp data.
struct timestamp
{
	std::int16_t year = 0;   //!< Year [0-inf).
	std::int16_t month = 0;  //!< Month of the year [1-12].
	std::int16_t day = 0;    //!< Day of the month [1-31].
	std::int16_t hour = 0;   //!< Hours since midnight [0-23].
	std::int16_t min = 0;    //!< Minutes after the hour [0-59].
	std::int16_t sec = 0;    //!< Seconds after the minute.
	std::int32_t fract = 0;  //!< Fractional seconds.
};

} // wise