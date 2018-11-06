// *********************************************************************
//   IQRF standards header file                                        *
// *********************************************************************
// Copyright (c) IQRF Tech s.r.o.
//
// File:    $RCSfile: IQRFstandard.h,v $
// Version: $Revision: 1.11 $
// Date:    $Date: 2018/09/20 12:15:05 $
//
// Revision history:
//   20??/??/??  Release for DPA 3.03
//   2017/11/16  Release for DPA 3.02
//   2017/08/14  Release for DPA 3.01
//
// *********************************************************************

// Online DPA documentation http://www.iqrf.org/DpaTechGuide/

#ifndef _IQRFSTD_HEADER_
#define _IQRFSTD_HEADER_

//############################################################################################

// Enumerate command valid for all standards
#define	PCMD_STD_ENUMERATE						0x3E

// -------------------------------------------------------------------------------------------
// IQRF Sensors standard
// -------------------------------------------------------------------------------------------
// IQRF Sensors standard peripheral
#define	PNUM_STD_SENSORS						0x5E
// IQRF Sensors standard peripheral type
#define	PERIPHERAL_TYPE_STD_SENSORS				0x5E

// IQRF Sensors standard peripheral - read sensor values command
#define	PCMD_STD_SENSORS_READ_VALUES			0x00
// IQRF Sensors standard peripheral - read sensor types and values command
#define	PCMD_STD_SENSORS_READ_TYPES_AND_VALUES	0x01

// IQRF Sensors standard peripheral - sensor types
//  2 bytes
#define	STD_SENSOR_TYPE_TEMPERATURE				0x01
#define	STD_SENSOR_TYPE_CO2						0x02
#define	STD_SENSOR_TYPE_VOC						0x03
#define	STD_SENSOR_TYPE_EXTRA_LOW_VOLTAGE		0x04
#define	STD_SENSOR_TYPE_EARTHS_MAGNETIC_FIELD	0x05
#define	STD_SENSOR_TYPE_LOW_VOLTAGE				0x06
#define	STD_SENSOR_TYPE_CURRENT					0x07
#define	STD_SENSOR_TYPE_POWER					0x08
#define	STD_SENSOR_TYPE_MAINS_FREQUENCY			0x09
#define	STD_SENSOR_TYPE_TIMESPAN    			0x0A
#define	STD_SENSOR_TYPE_ILLUMINANCE	    		0x0B
#define	STD_SENSOR_TYPE_NO2         			0x0C
#define	STD_SENSOR_TYPE_SO2         			0x0D
#define	STD_SENSOR_TYPE_CO          			0x0E
#define	STD_SENSOR_TYPE_O3          			0x0F
#define	STD_SENSOR_TYPE_ATMOSPHERIC_PRESSURE	0x10
#define	STD_SENSOR_TYPE_COLOR_TEMPERATURE   	0x11

//  1 byte
#define	STD_SENSOR_TYPE_HUMIDITY				0x80
#define	STD_SENSOR_TYPE_BINARYDATA7				0x81
#define	STD_SENSOR_TYPE_POWER_FACTOR			0x82
#define	STD_SENSOR_TYPE_UV_INDEX                0x83  // ToDo

//  4 bytes
#define	STD_SENSOR_TYPE_BINARYDATA30			0xA0
#define	STD_SENSOR_TYPE_CONSUMPTION				0xA1
#define	STD_SENSOR_TYPE_DATETIME				0xA2
#define	STD_SENSOR_TYPE_TIMESPAN_LONG  			0xA3

//  Multiple bytes
#define	STD_SENSOR_TYPE_DATA_BLOCK			    0xC0

// IQRF Sensors standard peripheral - FRC commands
#define	FRC_STD_SENSORS_BIT						0x10
#define	FRC_STD_SENSORS_1B						0x90
#define	FRC_STD_SENSORS_2B						0xE0
#define	FRC_STD_SENSORS_4B						0xF8

// -------------------------------------------------------------------------------------------
// IQRF Binary Outputs standard
// -------------------------------------------------------------------------------------------
// IQRF Binary Outputs standard peripheral
#define	PNUM_STD_BINARY_OUTPUTS					0x4B
// IQRF Binary Outputs peripheral type
#define	PERIPHERAL_TYPE_STD_BINARY_OUTPUTS		0x4B

// IQRF Sensors standard peripheral - read sensor values command
#define	PCMD_STD_BINARY_OUTPUTS_SET				0x00

// -------------------------------------------------------------------------------------------
// IQRF Light standard
// -------------------------------------------------------------------------------------------
// IQRF Light standard peripheral
#define	PNUM_STD_LIGHT							0x71
// IQRF Light standard peripheral type
#define	PERIPHERAL_TYPE_STD_LIGHT				0x71

// IQRF Light standard peripheral - Set Power
#define	PCMD_STD_LIGHT_SET						0x00
// IQRF Light standard peripheral - Increment Power
#define	PCMD_STD_LIGHT_INC						0x01
// IQRF Light standard peripheral - Decrement Power
#define	PCMD_STD_LIGHT_DEC						0x02

// IQRF Sensors standard peripheral - FRC commands
#define	FRC_STD_LIGHT_ONOFF						0x10
#define	FRC_STD_LIGHT_ALARM						0x11

// -------------------------------------------------------------------------------------------

#endif	// _IQRFSTD_HEADER_

//############################################################################################
