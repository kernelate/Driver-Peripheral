#ifndef _DOORTALK_DRIVERS_H_
#define _DOORTALK_DRIVERS_H_

#define SPI_MAX_BIT		16

typedef struct {
	int special_function_register;
	int value;
} DTDRIVERS_DATA;

#define ON					_IOW('p', 1, DTDRIVERS_DATA)
#define ON_DELAY			_IOW('p', 2, DTDRIVERS_DATA)
#define SLOW	 			_IOW('p', 3, DTDRIVERS_DATA)
#define FAST				_IOW('p', 4, DTDRIVERS_DATA)
#define ONCE		    	_IOW('p', 5, DTDRIVERS_DATA)
#define TWICE		    	_IOW('p', 6, DTDRIVERS_DATA)
#define RECOVERY	    	_IOW('p', 7, DTDRIVERS_DATA)
#define OFF					_IOW('p', 8, DTDRIVERS_DATA)
#define IRLED_ON			_IOW('p', 9, DTDRIVERS_DATA)
#define IRLED_OFF			_IOW('p', 10, DTDRIVERS_DATA)
#define SENSOR_ENABLED		_IOW('p', 11, DTDRIVERS_DATA)
#define SENSOR_DISABLED		_IOW('p', 12, DTDRIVERS_DATA)
#define MOTION_SENSITIVITY	_IOW('p', 13, DTDRIVERS_DATA)
#define SENSOR_STATUS		_IOR('p', 14, DTDRIVERS_DATA)
#define ETHERNET_ON			_IOW('p', 15, DTDRIVERS_DATA)
#define ETHERNET_OFF		_IOW('p', 16, DTDRIVERS_DATA)
#define DOORLOCK_OFF		_IOW('p', 17, DTDRIVERS_DATA)
#define DOORLOCK_ON			_IOW('p', 18, DTDRIVERS_DATA)
#define RELAY_STATUS		_IOR('p', 19, DTDRIVERS_DATA)


/*
 * Left intentionally to be used as future reference
 *
 * #define GPIO_TTA20_STAT	    _IOR('p', 5, unsigned long)
*/

/* #define VA_GPIO_BASE	0xf4500000 */
/* is this address revised ??? */
#define VA_GPIO_BASE	0xfd500000

#define GPBCON		(VA_GPIO_BASE + 0x0040)
#define GPBDAT		(VA_GPIO_BASE + 0x0044)

#define GPC1CON		(VA_GPIO_BASE + 0x0080)
#define GPC1DAT		(VA_GPIO_BASE + 0x0084)

#define GPH0CON		(VA_GPIO_BASE + 0x0c00)
#define GPH0DAT		(VA_GPIO_BASE + 0x0c04)

#define GPH1CON		(VA_GPIO_BASE + 0x0c20)

#define GPH1DAT		(VA_GPIO_BASE + 0x0c24)

#define GPH2CON		(VA_GPIO_BASE + 0x0c40)
#define GPH2DAT		(VA_GPIO_BASE + 0x0c44)

#define GPH3CON		(VA_GPIO_BASE + 0x0c60)

#define GPH3DAT		(VA_GPIO_BASE + 0x0c64)
#define GPH3PUD		(VA_GPIO_BASE + 0x0c68)
#define GPH3INTCON	(VA_GPIO_BASE + 0x0e0c)
#define GPH3MASK	(VA_GPIO_BASE + 0x0f0c)
#define GPH3PEND	(VA_GPIO_BASE + 0xef4c)



/* SPI Config here */
#define SPI_DATA		0x1
#define SPI_LATCH		0x2
#define SPI_CLOCK		0x3
/* I FORGOT HOW TO IMPLEMENT THIS :TODO  */

#endif

