// *********************************************************************
//   IQRF HWPIDs                                                       *
// *********************************************************************
// Copyright (c) IQRF Tech s.r.o.
//
// File:    $RCSfile: IQRF_HWPID.h,v $
// Version: $Revision: 1.21 $
// Date:    $Date: 2018/06/14 15:54:02 $
//
// Revision history:
//   20??/??/??  Release for DPA 3.03
//   2017/11/16  Release for DPA 3.02
//   2017/08/14  Release for DPA 3.01
//
// *********************************************************************

// Online DPA documentation http://www.iqrf.org/DpaTechGuide/

#ifndef _IQRFSTD_HWPID_
#define _IQRFSTD_HWPID_

//# Class #1 #################################################################################
// Manufacturer: bits:0-9, bit.0=0 (even numbers, but not 0x0000)
// Product: bits:10-15 (6 bits)
//############################################################################################
#define HWPID_CLS1(prod,man)  ( (uns16)(man) | ( (uns16)(prod) << 10 ) )

// -------------------------------------------------------------------------------------------
// IQRF Tech s.r.o.
#define	HWPID_IQRF_TECH	  0x002

// DDC-SE-01 sensor example 
// (0002_DDC-SE01.c)
#define	HWPID_IQRF_TECH__DEMO_DDC_SE01				  HWPID_CLS1( 0x00, HWPID_IQRF_TECH ) // 0x0002

// DDC-SE-01 + DDC-RE-01 sensor example 
// (0402_DDC-SE+RE.c)
#define	HWPID_IQRF_TECH__DEMO_DDC_SE01_RE01			  HWPID_CLS1( 0x01, HWPID_IQRF_TECH ) // 0x0402

// TR temperature sensor example 
// (0802_TrThermometer.c)
#define	HWPID_IQRF_TECH__DEMO_TR_THERMOMETER		  HWPID_CLS1( 0x02, HWPID_IQRF_TECH ) // 0x0802

// Binary output example using LEDs and DDC-RE-01 
// (0C02_BinaryOutput-Template.c)
#define	HWPID_IQRF_TECH__DEMO_BINARY_OUTPUT			  HWPID_CLS1( 0x03, HWPID_IQRF_TECH ) // 0x0C02

// Light example 
// (1002_Light-Template.c)
#define	HWPID_IQRF_TECH__DEMO_LIGHT					  HWPID_CLS1( 0x04, HWPID_IQRF_TECH ) // 0x1002

// Sensor template 
// (1402_Sensor-Template.c)
#define	HWPID_IQRF_TECH__DEMO_SENSOR_TEMPLATE		  HWPID_CLS1( 0x05, HWPID_IQRF_TECH ) // 0x1402

// ToDo Description DK-SW2-01
// ToDo (?.c)
#define	HWPID_IQRF_TECH__DK_SW2_01                    HWPID_CLS1( 0x06, HWPID_IQRF_TECH ) // 0x1802

// ToDo Description IQD-SW1-01
// ToDo (?.c)
#define	HWPID_IQRF_TECH__IQD_SW1_01                   HWPID_CLS1( 0x07, HWPID_IQRF_TECH ) // 0x1C02

// ToDo Description IQD-SW2-01
// ToDo (?.c)
#define	HWPID_IQRF_TECH__IQD_SW2_01                   HWPID_CLS1( 0x08, HWPID_IQRF_TECH ) // 0x2002

// ToDo Description IQD-RC3-01
// ToDo (?.c)
#define	HWPID_IQRF_TECH__IQD_RC3_01                   HWPID_CLS1( 0x09, HWPID_IQRF_TECH ) // 0x2402

// ToDo Description IQD-SM1
// ToDo (?.c)
#define	HWPID_IQRF_TECH__IQD_SM1                      HWPID_CLS1( 0x0A, HWPID_IQRF_TECH ) // 0x2802

// ToDo Description IQD-RC4-01
// ToDo (?.c)
#define	HWPID_IQRF_TECH__RC4_01                       HWPID_CLS1( 0x0B, HWPID_IQRF_TECH ) // 0x2C02

// ToDo Description IQD-RC4-02
// ToDo (?.c)
#define	HWPID_IQRF_TECH__RC4_02                       HWPID_CLS1( 0x0C, HWPID_IQRF_TECH ) // 0x3002

// ToDo Description IQD-REP-01
// ToDo (?.c)
#define	HWPID_IQRF_TECH__REP_01                       HWPID_CLS1( 0x0D, HWPID_IQRF_TECH ) // 0x3402

// ToDo Description IQD-REP-02 Template Master
// ToDo (?.c)
#define	HWPID_IQRF_TECH__REP_02_TEMPLATE_MASTER       HWPID_CLS1( 0x0E, HWPID_IQRF_TECH ) // 0x3802

// ToDo Description IQD-SW1-02
// ToDo (?.c)
#define	HWPID_IQRF_TECH__IQD_SW1_02                   HWPID_CLS1( 0x0F, HWPID_IQRF_TECH ) // 0x3C02

// ToDo Description IQD-REP-02 Template Slave
// ToDo (?.c)
#define	HWPID_IQRF_TECH__REP_02_TEMPLATE_SLAVE        HWPID_CLS1( 0x10, HWPID_IQRF_TECH ) // 0x4002

//# Class #2 #################################################################################
// Manufacturer: bits:0-11, bit.0=1 (odd numbers, never value 0x???F)
// Product: bits:12-15 (4 bits)
//############################################################################################
#define HWPID_CLS2(prod,man)  ( (uns16)(man) | ( (uns16)(prod) << 12 ) )

// -------------------------------------------------------------------------------------------
// PROTRONIX s.r.o.
#define	HWPID_PROTRONIX	  0x001

// Temperature+Humidity+CO2 sensor 
// (0001_Protronix-T+RH+CO2.c)
#define	HWPID_PROTRONIX__TEMP_HUM_CO2				  HWPID_CLS2( 0x0, HWPID_PROTRONIX ) // 0x0001

// Temperature+Humidity+VOC sensor 
// (1001_Protronix-T+RH+VOC.c)
#define	HWPID_PROTRONIX__TEMP_HUM_VOC				  HWPID_CLS2( 0x1, HWPID_PROTRONIX ) // 0x1001

// Temperature+Humidity+CO2 sensor + Relay
// (2001_Protronix-T+RH+CO2+Relay.c)
#define	HWPID_PROTRONIX__TEMP_HUM_CO2_RELAY			  HWPID_CLS2( 0x2, HWPID_PROTRONIX ) // 0x2001

// Temperature+Humidity sensor 
// (3001_Protronix-T+RH.c)
#define	HWPID_PROTRONIX__TEMP_HUM					  HWPID_CLS2( 0x3, HWPID_PROTRONIX ) // 0x3001

// -------------------------------------------------------------------------------------------
// NETIO products a.s.
#define	HWPID_NETIO		  0x003

// Cobra 1 - 1x power plug 
// (0003_Netio-Cobra1.c)
#define	HWPID_NETIO__COBRA1							  HWPID_CLS2( 0x0, HWPID_NETIO )	 // 0x0003

// -------------------------------------------------------------------------------------------
// DATmoLUX a.s.
#define	HWPID_DATMOLUX	  0x005

// DATmoLUX Light
// 0005_DATmoLUX-Light.*
#define	HWPID_DATMOLUX__LIGHT						  HWPID_CLS2( 0x0, HWPID_DATMOLUX )	 // 0x0005

// -------------------------------------------------------------------------------------------
// CITIQ s.r.o.
#define	HWPID_CITIQ		  0x007

// -------------------------------------------------------------------------------------------
// Austyn International s.r.o.
#define	HWPID_AUSTYN	  0x009

// Room temperature controller 
// (0009_RoomTemperatureController.c)
#define	HWPID_AUSTYN__ROOM_CONTROLLER				  HWPID_CLS2( 0x0, HWPID_AUSTYN )	 // 0x0009

// -------------------------------------------------------------------------------------------
// Aledo s.r.o.
#define	HWPID_ALEDO		  0x00B

// Room temperature controller 
// (000B_Aledo-Reader_R02A230.c)
#define	HWPID_ALEDO__READER_R02A230					  HWPID_CLS2( 0x0, HWPID_ALEDO )	 // 0x000B

// -------------------------------------------------------------------------------------------
// SANELA spol. s r. o.
#define	HWPID_SANELA	  0x00D

// Sanela SL626 Person presence sensor 
// (000D_Sanela-SL626.c)
#define	HWPID_SANELA__SL626							  HWPID_CLS2( 0x0, HWPID_SANELA )	 // 0x000D

// Sanela SL626A sink sensor 
// (100D_Sanela-SL626A.c)
#define	HWPID_SANELA__SL626A					      HWPID_CLS2( 0x1, HWPID_SANELA )	 // 0x100D

// -------------------------------------------------------------------------------------------
// TESLA Blatná, a.s.
#define	HWPID_TESLA_BLATNA  0x011

// TESLA Blatná Smart City environmental module
// (0011_TESLA_Blatna-EnvironmentalModule.c)
#define	HWPID_TESLA_BLATNA__EnvironmentalModule		  HWPID_CLS2( 0x0, HWPID_TESLA_BLATNA )	 // 0x0011

// -------------------------------------------------------------------------------------------

#endif	// _IQRFSTD_HWPID_

//############################################################################################
