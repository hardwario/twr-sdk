
#include <bc_i2c.h>
#include <bc_spi.h>
#include <bc_log.h>
#include <bc_gpio.h>
#include <bc_system.h>

#include <bc_tca9534a.h>
#include <bc_scheduler.h>

#include <stm32l0xx.h>


#define int8 int8_t
#define int16 int16_t
#define uns8 uint8_t
#define uns16 uint16_t
#define byte uint8_t

#include "DPA.h"
#include "IQRF_HWPID.h"
#include "IQRFstandard.h"

bc_tca9534a_t tca9534a;

static void _bc_module_iqrf_task(void *param);

#define _BC_MODULE_IQRF_EXPANDER_IQRF_POWER (1 << 4)
#define _BC_MODULE_IQRF_EXPANDER_IQRF_EN_CS (1 << 7)

bool bc_module_iqrf_init(void)
{

    if (!bc_tca9534a_init(&tca9534a, BC_I2C_I2C0, 0x20))
    {
        return false;
    }

    // Disconnect power
    if (!bc_tca9534a_write_port(&tca9534a, 0x00))
    {
        return false;
    }

    if (!bc_tca9534a_set_port_direction(&tca9534a, (uint8_t)~(_BC_MODULE_IQRF_EXPANDER_IQRF_POWER | _BC_MODULE_IQRF_EXPANDER_IQRF_EN_CS)))
    {
        return false;
    }

    // Delay
    bc_tick_t timestamp = bc_tick_get();
    while (bc_tick_get() - timestamp < 2000)
    {
        continue;
    }

    // Connect power
    if (!bc_tca9534a_write_port(&tca9534a, _BC_MODULE_IQRF_EXPANDER_IQRF_POWER | _BC_MODULE_IQRF_EXPANDER_IQRF_EN_CS))
    {
        return false;
    }

    bc_gpio_init(BC_GPIO_INT);
    bc_gpio_set_mode(BC_GPIO_INT, BC_GPIO_MODE_INPUT);

    bc_spi_init(BC_SPI_SPEED_250_KHZ, BC_SPI_MODE_1);
    bc_spi_set_timing(15, 700, 15);

    bc_scheduler_register(_bc_module_iqrf_task, NULL, 0);

    bc_scheduler_disable_sleep();
    bc_system_deep_sleep_disable();

    bc_system_pll_enable();

    return true;
}

// HDLC byte stuffing bytes
// Flag Sequence
#define   HDLC_FRM_FLAG_SEQUENCE    0x7e
// Asynchronous Control Escape
#define   HDLC_FRM_CONTROL_ESCAPE   0x7d
// Asynchronous transparency modifier
#define   HDLC_FRM_ESCAPE_BIT       0x20

// Flag to DpaEvent_DpaRequest event value to indicate return TRUE not FALSE
#define EVENT_RETURN_TRUE           0x80
// Flag to DpaEvent_DpaRequest event value to report error, error value is the 1st data byte
#define EVENT_RESPONSE_ERROR        0x40

// Received data from IQRF
uint8_t RxBuffer[2 * sizeof( byte ) + sizeof( TDpaIFaceHeader ) + sizeof( TDpaMessage )];

// Data from IQRF length limits
#define MIN_RX_PACKET_DATA_LENGTH  2
#define MAX_RX_PACKET_DATA_LENGTH  sizeof( RxBuffer )

#define SPI_BYTES_GAP 40


    // HDLC machine states
  typedef enum { RXstateWaitHead, RXstatePacket, RXstateEscape } TState;

  // HDLC state
  uint8_t state = RXstateWaitHead;
  // Length of the already received data
  uint8_t rxLength;
  // Pointer to the received data
  uint8_t *pRxBuffer;




//############################################################################################
// Sends one byte to IQRF
void TxByte( byte data )
//############################################################################################
{
    //SPI.transfer( data );
    //delayMicroseconds( SPI_BYTES_GAP );

    uint8_t spiSource = data;
    uint8_t oneByte;
    bc_spi_transfer(&spiSource, &oneByte, 1);

    //bc_log_debug("TX Byte %02x", data);
}

//############################################################################################
// Sends one HDLC byte to IQRF
void TxHdlcByte( byte data )
//############################################################################################
{
  switch ( data )
  {
    default:
      TxByte( data );
      return;

    case HDLC_FRM_FLAG_SEQUENCE:
    case HDLC_FRM_CONTROL_ESCAPE:
    {
      TxByte( HDLC_FRM_CONTROL_ESCAPE );
      TxByte( data ^ HDLC_FRM_ESCAPE_BIT );
      return;
    }
  }
}

//############################################################################################
// Returns FRC value back to IQRF
void ResponseFRCvalue( unsigned long frcValue )
//############################################################################################
{
  // Start packet
  TxByte( HDLC_FRM_FLAG_SEQUENCE );
  // Send event value
  TxHdlcByte( DpaEvent_FrcValue );
  // Send FRC value up to 4 bytes
  TxHdlcByte( frcValue & 0xFF );
  TxHdlcByte( ( frcValue >> 8 ) & 0xFF );
  TxHdlcByte( ( frcValue >> 16 ) & 0xFF );
  TxHdlcByte( ( frcValue >> 24 ) & 0xFF );
  // Stop packet
  TxByte( HDLC_FRM_FLAG_SEQUENCE );
}

//############################################################################################
// Return DPA response back to IQRF
void ResponseCommand( byte returnFlags, byte _DpaDataLength, byte dataLength, byte *pData )
//############################################################################################
{
  // Start packet
  TxByte( HDLC_FRM_FLAG_SEQUENCE );
  // Send event value
  TxHdlcByte( DpaEvent_DpaRequest | returnFlags );
  // Send DPA variable data length (must not equal to the actual data length sent)
  TxHdlcByte( _DpaDataLength );
  // Send DPA response data
  for ( ; dataLength != 0; dataLength-- )
    TxHdlcByte( *pData++ );
  // Stop packet
  TxByte( HDLC_FRM_FLAG_SEQUENCE );
}

//############################################################################################
// Returns sensor value
byte GetSensor0Value()
//############################################################################################
{
  // Remap values from Alcohol Gas Sensor MQ-3 connected to the analog input 0
  return 0xEA;
}

//############################################################################################
// Packet from Custom DPA Handler was received
void CustomDpaHandler( byte dataLength )
//############################################################################################
{

    bc_log_debug("DPA Handler len:%d, evt: %d", dataLength, RxBuffer[0]);

  // Which Custom DPA Handler event to handle?
  switch ( RxBuffer[0] )
  {
    // Prepare DPA response to DPA request
    case DpaEvent_DpaRequest:
      if ( dataLength >= ( 2 * sizeof( byte ) + sizeof( TDpaIFaceHeader ) ) )
      {
        // Fake important DPA variables for the DPA Request/Response so the Custom DPA handler code will can be written almost same way on both platforms
        #define _DpaDataLength  (RxBuffer[1])
        #define _NADR           (RxBuffer[2])
        #define _NADRhigh       (RxBuffer[3])
        #define _PNUM           (RxBuffer[4])
        #define _PCMD           (RxBuffer[5])
        #define _HWPIDlow       (RxBuffer[6])
        #define _HWPIDhigh      (RxBuffer[7])
        #define _DpaMessage     (*((TDpaMessage*)(RxBuffer+8)))

            // Fake Custom DPA Handler macro to return DPA error (this macro does not do return the same way the DPA original macro)
        #define DpaApiReturnPeripheralError(error) do { \
        _DpaMessage.ErrorAnswer.ErrN = error; \
        returnDataLength = _DpaDataLength = sizeof( _DpaMessage.ErrorAnswer.ErrN ); \
        returnFlags = EVENT_RESPONSE_ERROR | EVENT_RETURN_TRUE; \
        } while( 0 )

        // Value or error flag to return from Custom DPA handler
        byte returnFlags = 0;
        // Length data to return (may not equal to _DpaDataLength)
        byte returnDataLength = 0;

        // Device enumeration?
        if ( IsDpaEnumPeripheralsRequest() )
        {
          // We implement 1 user peripheral
          _DpaMessage.EnumPeripheralsAnswer.UserPerNr = 1;
          FlagUserPer( _DpaMessage.EnumPeripheralsAnswer.UserPer, PNUM_STD_SENSORS );
          _DpaMessage.EnumPeripheralsAnswer.HWPID = 0x123F;
          _DpaMessage.EnumPeripheralsAnswer.HWPIDver = 0xABCD;

          // Return the enumeration structure but do not modify _DpaDataLength
          returnDataLength = sizeof( _DpaMessage.EnumPeripheralsAnswer );
          // Return TRUE
          returnFlags = EVENT_RETURN_TRUE;
        }

        // Get information about peripherals?
        else if ( IsDpaPeripheralInfoRequest() )
        {
          if ( _PNUM == PNUM_STD_SENSORS )
          {
            _DpaMessage.PeripheralInfoAnswer.PerT = PERIPHERAL_TYPE_STD_SENSORS;
            _DpaMessage.PeripheralInfoAnswer.PerTE = PERIPHERAL_TYPE_EXTENDED_READ_WRITE;
            // Set standard version
            _DpaMessage.PeripheralInfoAnswer.Par1 = 13;

            // Return the information structure but do not modify _DpaDataLength
            returnDataLength = sizeof( _DpaMessage.PeripheralInfoAnswer );
            // Return TRUE
            returnFlags = EVENT_RETURN_TRUE;
          }
        }
        else
        {
          // Handle peripheral command

          // Supported peripheral number?
          if ( _PNUM == PNUM_STD_SENSORS )
          {
            // Supported commands?
            switch ( _PCMD )
            {
              // Invalid command
              default:
                // Return error
                DpaApiReturnPeripheralError( ERROR_PCMD );
                break;

                // Sensor enumeration
              case PCMD_STD_ENUMERATE:
                // Check data request length
                if ( _DpaDataLength != 0 )
                {
                  DpaApiReturnPeripheralError( ERROR_DATA_LEN );
                  break;
                }

                // 1st byte is sensor type
                _DpaMessage.Response.PData[0] = STD_SENSOR_TYPE_BINARYDATA7;

                // Return just one sensor type
                returnDataLength = _DpaDataLength = sizeof( _DpaMessage.Response.PData[0] );
                // Return TRUE
                returnFlags = EVENT_RETURN_TRUE;
                break;

                // Supported commands. They are handled almost the same way
              case PCMD_STD_SENSORS_READ_VALUES:
              case PCMD_STD_SENSORS_READ_TYPES_AND_VALUES:
              {
                // No sensor bitmap specified?
                if ( _DpaDataLength == 0 )
                {
                  // Bitmap is 32 bits long = 4
                  _DpaDataLength = 4;
                  // Simulate 1st sensor in the bitmap (states of the other unimplemented sensors do not care)
                  _DpaMessage.Request.PData[0] |= 0x01; // Note: must not modify W
                }
                // Valid bitmap length?
                else if ( _DpaDataLength != 4 )
                {
                  // Return error
                  DpaApiReturnPeripheralError( ERROR_DATA_LEN );
                  break;
                }

                // Pointer to the response data
                byte *pResponseData = _DpaMessage.Response.PData;
                // Is my only sensor selected?
                if ( ( _DpaMessage.Request.PData[0] & 0x01 ) != 0 )
                {
                  // Return also sensor type?
                  if ( _PCMD == PCMD_STD_SENSORS_READ_TYPES_AND_VALUES )
                    *pResponseData++ = STD_SENSOR_TYPE_BINARYDATA7;

                  // Return sensor data
                  *pResponseData++ = GetSensor0Value();
                }

                // Returned data length
                returnDataLength = _DpaDataLength = ( pResponseData - _DpaMessage.Response.PData );
                // Return TRUE
                returnFlags = EVENT_RETURN_TRUE;
                break;
              }
            }
          }
        }

        // Return DPA response
        ResponseCommand( returnFlags, _DpaDataLength, returnDataLength, (byte*)&_DpaMessage );
      }
      break;

      // Return FRC Value
    case DpaEvent_FrcValue:
      // Check for the minimum length (FRC command and at least 2 bytes of data)
      if ( dataLength >= ( 2 + 1 + 2 ) )
      {
        // Fake important DPA variables for the DPA FRC handling
#define FrcCommand               (RxBuffer[1])
#define DataOutBeforeResponseFRC ((byte*)( &RxBuffer[2] ))

      // Check the correct FRC request
        if ( DataOutBeforeResponseFRC[0] == PNUM_STD_SENSORS &&
          ( DataOutBeforeResponseFRC[1] == 0x00 || DataOutBeforeResponseFRC[1] == STD_SENSOR_TYPE_BINARYDATA7 ) &&
             ( DataOutBeforeResponseFRC[2] & 0x1f ) == 0 )
        {
          // Which FRC command to handle?
          switch ( FrcCommand )
          {
            case FRC_STD_SENSORS_1B:
              ResponseFRCvalue( GetSensor0Value() + 4 );
              break;

            case FRC_STD_SENSORS_BIT:
              ResponseFRCvalue( ( GetSensor0Value() & ( 0x01 << ( DataOutBeforeResponseFRC[2] >> 5 ) ) ) != 0 ? 0x03 : 0x01 );
              break;

            default:
                break;
          }
        }
      }
      break;

      default:
        break;
  }
}



static void _bc_module_iqrf_task(void *param)
{
    (void) param;

    state = RXstateWaitHead;

    // Active low
    while (!bc_gpio_get_input(BC_GPIO_INT))
    {


        GPIOB->BSRR = GPIO_BSRR_BR_12;



        volatile int a = 0; a++;

        // Read the byte from IQRF
        //uint8_t oneByte = SPI.transfer( HDLC_FRM_CONTROL_ESCAPE );

        uint8_t spiSource = HDLC_FRM_CONTROL_ESCAPE;
        uint8_t oneByte;
        bc_spi_transfer(&spiSource, &oneByte, 1);

        //bc_log_debug("RX raw byte: 0x%02x, state: %d", oneByte, state);

         switch ( state )
    {
      // Waiting for the DLHC header
      case RXstateWaitHead:
      {
        if ( oneByte == HDLC_FRM_FLAG_SEQUENCE )
        {
          rxLength = 0;
          pRxBuffer = RxBuffer;
          // RXstatePacket
_NextState:
          state++;
        }
        break;
      }

      // Handling packet data byte
      case RXstatePacket:
      {
        switch ( oneByte )
        {
          case HDLC_FRM_CONTROL_ESCAPE:
            goto _NextState;

          case HDLC_FRM_FLAG_SEQUENCE:
          {
            if ( rxLength >= MIN_RX_PACKET_DATA_LENGTH )
            {
                // Delay
                /*
        bc_tick_t timestamp = bc_tick_get();
        while (bc_tick_get() - timestamp < 50)
        {
            continue;
        }*/

              // Packet received, handle it
              CustomDpaHandler( rxLength );
              // Exit loop
              bc_scheduler_plan_current_relative(10);
              return;
            }

            goto _SetRXstateWaitHead;
          }

          default:
            break;
        }

_StoreByte:
        if ( rxLength == ( MAX_RX_PACKET_DATA_LENGTH + 2 ) )
          goto _SetRXstateWaitHead;

        *pRxBuffer++ = oneByte;
        rxLength++;
        break;
      }

      // Handle escaped byte
      case RXstateEscape:
      {
        switch ( oneByte )
        {
          case HDLC_FRM_FLAG_SEQUENCE:
          case HDLC_FRM_CONTROL_ESCAPE:
_SetRXstateWaitHead:
            state = RXstateWaitHead;
            break;

          default:
            state--; // RXstatePacket
            oneByte ^= HDLC_FRM_ESCAPE_BIT;
            goto _StoreByte;
        }
        break;
      }
      default:
        break;
    }


    }

    GPIOB->BSRR = GPIO_BSRR_BS_12;
    bc_scheduler_plan_current_now();
}
