/*******************************************************************************
*   ______
*  / _____) /|
* | |      / /
* | | /\  / /
* | | \ \/ /  _      Copyright (c) IoTize, 2016
* | |  \__/  | |        http://www.iotize.com
* | |________| |
*  \__________/
*
********************************************************************************
*@file          : Tap.cpp
*@author        : IoTize
*@brief         : Alternative to the hardware SWD for IoTize.
*                 This does NOT belong to the IoTize project but it has to be added
*                 to the end user's application. 
*******************************************************************************/

/* Includes (manufacturer specific) ------------------------------------------*/
#include "arduino.h"            /*!< general declarations     */
#include "tap.h"                /*!< general declarations     */
#include "..\..\..\TapNLink\S3P_conf.h"

class Tap  myTap;

/* Debug and comm registers --------------------------------------------------*/
//CSW is always present, even if SWD emulation is disabled
u32 S3P_CSW_reg = S3P_INVALID_CSW_REG;



//note: size.7=1 means error during first phase of frame, and in this case the error code is in size[6..0]
#define Size_ERROR      (0x80)


/* Macro definitions ---------------------------------------------------------*/
#define GETWRITEBIT(command)     ( ( (command) & 0x80 ) ? 1 : 0 )   //extract the direction from the command
#define GETSIZE(command)    ( 1 << ( ( (command) & ( 0x60 ) ) >> 5 ) )   //extract the data size from the command
#define GETEXT(command)    ( ( (command) & ( 0x10 ) ) ? 1 : 0 )   //extract the EXT bit from the command
#define GETADDRESS(command) ( (command) & 7 )        //extract the address from the command
//note: parity check is performed with a function, not a macro


void S3P_IrqHandler( void );


#define DEBUG_S3P 1
//This signal could be usefull to analyse the duration of the interrupt handling
#if DEBUG_S3P
byte debugPin = 7 ;
#endif

/*******************************************************************************
* Function Name  : Init
* Description    : This routine has to be called as soon as possible by the
*                  main() function. It initialises the IO ports normally used
*                  for SWD as a standard GPIO port. The SWD feature is disabled,
*                  and an interrupt will be setup in order to establish a
*                  communication with the debugger.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Tap::Init( int CLK, int IO)
{
  
    Pin_CK = CLK;   //For Uno, only pin2 or 3 are available as external interrupt (for CK)
    Pin_IO = IO;
  
    Command = 0;
    Size = 0;
    Addr = 0;
    Value = 0;
    ArraySize = S3P_REGN;

    // hw and irq initialisation (HW specific)
#if DEBUG_S3P
    digitalWrite(debugPin, LOW);
#endif
    ConfigureIOs();   
}

/*******************************************************************************
* Function Name  : ConfigureIOs
* Description    : This routine has to be called as soon as possible by the
*                  main() function. It initialises the IO ports normally used
*                  for SWD as a standard GPIO port. The SWD feature is disabled,
*                  and an interrupt will be setup in order to establish a
*                  communication with the debugger.
*                  This function is specific to the target MCU.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Tap::ConfigureIOs( void )
{
#if DEBUG_S3P
   pinMode(debugPin, OUTPUT);
#endif 
   pinMode(Pin_CK, INPUT);
   pinMode(Pin_IO, INPUT_PULLUP);

//Clear interrupt flag status
#if defined(ARDUINO_SAM_DUE)
    volatile u32 isr = *(volatile u32*) &(PIOA->PIO_ISR);
#elif (defined(ARDUINO_AVR_UNO)|| defined(ARDUINO_AVR_MINI)||defined(ARDUINO_AVR_NANO))     //Check {build.board}
    switch ( Pin_CK )
    {
      case 2: EIFR = 1; break; 
      case 3: EIFR = 2; break;
      default: break; //Should not work!!!!
    }
#elif (defined(ARDUINO_AVR_MEGA2560)||defined(ARDUINO_AVR_ADK))    //Check {build.board}
    switch ( Pin_CK )
    {
      case 2: EIFR = 0x10; break;   //Pin2 is PE4==Int4 
      case 3: EIFR = 0x20; break;   //Pin3 is PE5==Int5
      case 18: EIFR = 0x08; break;  //Pin18 is PD3==Int3
      case 19: EIFR = 0x04; break;  //Pin19 is PD2==Int2
      case 20: EIFR = 0x02; break;  //Pin20 is PD1==Int1
      case 21: EIFR = 0x01; break;  //Pin21 is PD0==Int0
      default: break; //Should not work!!!!
    }
#else
#   error "Board not yet support by the TAP library (clearing interrupt flag missing)"
#endif
    attachInterrupt(digitalPinToInterrupt(Pin_CK), S3P_IrqHandler, FALLING );
}


/*******************************************************************************
* Function Name  : PrepareCom
* Description    : prepare for com. first thing done after IT to tie CLK low ASAP
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Tap::PrepareCom( void )
{
    detachInterrupt(digitalPinToInterrupt(Pin_CK));
    digitalWrite(Pin_CK, LOW);   
    pinMode(Pin_CK, OUTPUT); 
}


/*******************************************************************************
* Function Name  : EndOfCom
* Description    : end of com, get ready for next IT. last thing done at end of IT handler
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Tap::EndOfCom( void )
{
    ConfigureIOs();   
#if DEBUG_S3P
    digitalWrite(debugPin, LOW);   
#endif
}


/*******************************************************************************
* Function Name  : BitCount8
* Description    : count the number of bits at 1 in a byte
*                  This is used for checking parity on command bytes
* Input          : input data byte.
* Output         : None.
* Return         : number of bits at 1.
*******************************************************************************/
int Tap::BitCount8( u8 inbyte )
{
    int bitcnt = 0;
    int i = 8;
    while ( i-- )
    {
        if ( inbyte & 1 )
            bitcnt++;
        inbyte >>= 1;
    }
    return bitcnt;
}

/*******************************************************************************
* Function Name  : Parity8
* Description    : get the parity of a byte
*                  This is used for checking parity on command bytes
* Input          : input data byte.
* Output         : None.
* Return         : parity. (0=odd, 1=even)
*******************************************************************************/
int Tap::Parity8( u8 inbyte )
{
    return ( ( BitCount8( inbyte ) & 1 ) ? 1 : 0 );
}


void S3P_IrqHandler( void ) 
{
     myTap.IrqHandler();
}

/*******************************************************************************
* Function Name  : S3P_IrqHandler
* Description    : This is the entry point for the communication.
*                  This interrupt handler is called on falling edge detected at SWDCLK.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Tap::IrqHandler( void )
{
#if DEBUG_S3P
    digitalWrite(debugPin, HIGH);   
#endif

    S3P_err ret = 0;

    //Prepare com (Pull the SWDCLK signal low.)
    PrepareCom();

    //Command is 0 when waiting for a command... (and only then)
    if ( ! Command )
    {
        //Get the command
        Command = GetValSPI( SIZE_COMMAND );    // One byte command

        //check the parity
        if ( ! Parity8( Command ) )
        {
            //parity check failed...
            Command = 0; //abort command... (allows to resync)
            Size    = 0;
        }
        //for Indexed mode, check EXT bit and if 1 get CMD2 and ext addr...
        if ( ( Command ) && ( GETEXT( Command ) ) )
        {
            //get CMD2
            Cmd2 = GetValSPI( SIZE_COMMAND );

            //check the parity
            if ( ! Parity8( Cmd2 ) )
            {
                //parity check failed...
                Command = 0; //abort command... (allows to resync)
                Size    = 0;
                Cmd2    = 0;
            }
        }
        else
        {
            Cmd2 = 0;
        }

        if ( Command )
        {
            // Extract address
            Addr = ( ( ( ( u16 )Cmd2 ) & 0x7F ) << 3 ) | GETADDRESS( Command ) ;

            // Extract size
            Size = GETSIZE( Command );

            // Write access if (DIR == 1)
            if ( GETWRITEBIT( Command ) )
            {
                u8 expCS1;
                u8 getCS1;
                int j;

                // Value to write
                Value = GetValSPI( Size );

                //calc expected CS1
                expCS1 = 0;
                expCS1 += Command;
                expCS1 += Cmd2;
                for ( j = 0; j < Size; j++ )
                {
                    expCS1 += ( ( Value >> ( j * 8 ) ) & 0xFF );
                }
                expCS1 = ~expCS1;

                //get CS1 from IoTize
                getCS1 = GetValSPI( 1 );

                //cmp CS1s
                if ( getCS1 != expCS1 )
                {
                    //during the read phase of the frame, there will be no data, but there will be an errcode in the ACK
                    Size = Size_ERROR + S3P_ERR_COM;
                }
            }
            else // Read access if (DIR == 0)
            {
                //read... nothing to do
            }
        }
    }
    else
    {
        //second phase of the frame...

        u8 myACK = S3P_NOERROR;

        //check for com error during first phase of the frame
        if ( Size & Size_ERROR )
        {
            //format error
            if ( Size & Size_ERROR )
            {
                myACK = ( Size & S3P_ERR_CODE_MASK );
                if ( ! myACK )
                    myACK = S3P_ERR_UNKNOWN; //unknown error
            }
        }
        else
        {
            //command OK. on with phase 2 of the frame...

            //Command is not 0 when received command, waiting for second pulse...
            //second step of the frame: send ACK&CS (+data if cmd is read)
            if ( GETWRITEBIT( Command ) )  // Write access if (DIR == 1)
            {
                //perform write access...
                if ( GETEXT( Command ) )
                    ret = WriteIndexedReg( Addr, Size, Value );
                else
                    ret = WriteReg( (S3P_Register_addr) Addr, Size, Value );

                //check for error...
                if ( ret )
                {
                    //try to get errcode from WriteReg
                    myACK = ret & 0x7F;
                    //otherwise return unknown error
                    if ( ! myACK )
                        myACK = S3P_ERR_UNKNOWN;
                }

            }
            else
            {
                //read access...
                if ( GETEXT( Command ) )
                    ret = ReadIndexedReg( Addr, Size, &Value );
                else
                    ret = ReadReg( (S3P_Register_addr) Addr, Size, &Value );
                //check for error...
                if ( ret )
                {
                    //try to get errcode from WriteReg
                    myACK = ret & 0x7F;
                    //otherwise return unknown error
                    if ( ! myACK )
                        myACK = S3P_ERR_UNKNOWN; //unknown error
                }
            }
        }

        //ACK parity in ACK.7
        if ( ! Parity8( myACK ) )
            myACK ^= S3P_ACK_PARITYBIT_MASK;

        //send ACK to IoTize
        SendValSPI( myACK, 1 );

        //send data and CS2 (if reading and if no error)
        if ( ( !GETWRITEBIT( Command ) )
                && ( !( myACK & 0x7F ) ) && ( !( Size & 0xF0 ) ) && ( Size )
           )
        {
            SendValSPI( Value, Size );      // Return value to IoTize
            int j;
            //calc CS2
            u8 CS2 = Command;
            CS2 += Cmd2;
            CS2 += myACK;
            for ( j = 0; j < Size; j++ )
            {
                CS2 += ( ( Value >> ( j * 8 ) ) & 0xFF );
            }
            CS2 = ~CS2;

           SendValSPI( CS2, 1 );      // Return value to IoTize
        }

        //prepare to accept next command
        Command = 0;
        Cmd2 = 0;
        Size = 0;
    }

    // Reconfigure the pins for the next IRQ!
    EndOfCom( );
}


/*******************************************************************************
* Function Name  : ReadReg
* Description    : Read a register value.
* Input          : address:  register address.
*                  Size: number of bytes to be read (either 1 or 4).
* Output         : read value in *pval
* Return         : errcode: 0=OK
*******************************************************************************/
S3P_err Tap::ReadReg( S3P_Register_addr address, u8 size, u32* pval )
{
    u32 value = -1;
    S3P_err ret = S3P_ERR_OK;
    unsigned new_address = address;

    if ( ! ret )
    {
        switch ( address )
        {

        case S3P_CSW_AD :       // Memory status register
        {
            value = S3P_CSW_reg & ( ~( S3P_CSW_READ_FEATURES_MASK ) );
            value |= S3P_CSW_FEATURE_MASK_INDEXED_MODE_ENABLE;
            //send other (future) features in HSB
            break;
        }

        case S3P_ID_AD :
            value = S3P_ID_VAL;
            break;

        default:
            //unknown or unimplemented address
            ret = ( S3P_ERR_CMD );
            break;
        }
    }

    if ( !ret )
    {
        if ( pval )
            *pval = value;
    }
    return ret;
}


/*******************************************************************************
* Function Name  : WriteReg
* Description    : Write a S3P register
* Input          : val:     value (u32) to be written
*                  address: index of the data to be written
*                  size:    1 or 4 (number of bytes to be written)
* Output         : None.
* Return         : errcode: 0=OK
*******************************************************************************/
S3P_err Tap::WriteReg( S3P_Register_addr address, u8 size, u32 val )
{
    unsigned new_address = address;
    S3P_err ret = S3P_ERR_OK;

    if ( ! ret )
    {
        switch ( address )
        {
        case S3P_CSW_AD:
        {
            if ( !ret )
            {
                if ( size == 1 )
                {
                    ( *( ( u8* )( &S3P_CSW_reg ) ) ) = val & 0x000000FF;
                }
                else if ( size == 2 )
                {
                    ( *( ( u16* )( &S3P_CSW_reg ) ) ) = val & 0x0000FFFF;
                }
                else if ( size == 4 )
                    S3P_CSW_reg = ( val & ( ~S3P_CSW_READ_ONLY_MASK ) ) | ( S3P_CSW_reg & S3P_CSW_READ_ONLY_MASK )  ;
                else
                    ret = S3P_ERR_CMD;

                //any CSW write clears the S3P indirect mode index
                INDEX_reg = 0;
            }
            break;
        }
        default :
            //unknown or unimplemented address
            ret = S3P_ERR_CMD;
            break;
        }
    }

    return ret;
}

/*******************************************************************************
* Function Name  : ReadIndexedReg
* Description    : Read from an indexed variable
* Input          : address is the index of the variable, size is 1/2/3, pval the location
* Return         : S3P_ERR_OK (0) if ok..
*******************************************************************************/
S3P_err Tap::ReadIndexedReg( S3P_Indreg_addr address, u8 size, u32* pval )
{
    u32 value = -1;
    S3P_err ret = S3P_ERR_OK;
    u8 fDone = 0;

    if (!ArraySize)
    {
      return S3P_ERR_CMD;
    }
    
    //We reset the index everytime we access a different address
    if  (   ( Last_AccessReg != Last_ReadReg )  ||
            ( Last_AccessReg != address )       
        )
    {
        INDEX_reg = 0;
    }

    if ( ( address >= ArraySize )          || 
        CheckIndexedReg( address, size, READACCESS )  ||
        ( INDEX_reg >= S3PVarArray[address].qty )  //check for overrun
      )
        ret = S3P_ERR_AR;

    if ( ! ret && ( size == S3PVarArray[address].unitsize ) ) 
    {
        switch ( S3PVarArray[address].unitsize ) 
        {
            case BYTESIZE:      //byte access
                value = ((u8 *) ( S3PVarArray[address].pAddr )) [ INDEX_reg ];
                fDone = 1;
                break;
            case HALFWORDSIZE:  //half word (16-bit) access
                value = ((u16 *) ( S3PVarArray[address].pAddr)) [ INDEX_reg ];
                fDone = 1;
                break;
            case WORDSIZE:      //word (32-bit) access
                value = ((u32 *) ( S3PVarArray[address].pAddr ))[ INDEX_reg ];
                fDone = 1;
                break;
        }
    }

    if ( fDone )
    {
        //Post increment when mode is set
        if ( S3P_CSW_reg & S3P_CSW_MULTIPLE_MASK )
        {
            INDEX_reg++;
        }
    }
    else
    {
        //report param error if params error (!)
        if ( ! ret )
            ret = S3P_ERR_CMD;
    }

    Last_ReadReg = address;
    Last_AccessReg = Last_ReadReg;

    if ( ( !ret ) && ( pval ) )
    {
        *pval = value;
    }

    return ret;
}

/*******************************************************************************
* Function Name  : WriteIndexedReg
* Description    : Write to an indexed variable
* Input          : address is the index of the variable, size is 1/2/3, val the value
* Return         : S3P_ERR_OK (0) if ok..
*******************************************************************************/
S3P_err Tap::WriteIndexedReg( S3P_Indreg_addr address, u8 size, u32 val )
{
    S3P_err ret = S3P_ERR_OK;
    u8 fDone = 0;

    if (!ArraySize)
    {
      return S3P_ERR_CMD;
    }

    //We reset the index everytime we access a different address
    if  (   ( Last_AccessReg != Last_WrittenReg )  ||
            ( Last_AccessReg != address )         
        )
    {
        INDEX_reg = 0;
    }

    if ( ( address >= ArraySize ) || 
         CheckIndexedReg( address, size, WRITEACCESS ) || 
        ( INDEX_reg >= S3PVarArray[address].qty )  //check for overrun
      )
        ret = S3P_ERR_AR;

    if ( !ret  && ( size == S3PVarArray[address].unitsize ) ) 
    {
        switch ( S3PVarArray[address].unitsize ) 
        {
            case BYTESIZE:      //byte access
                ((u8*) ( S3PVarArray[address].pAddr )) [ INDEX_reg ] = val;
                fDone = 1;
                break;
            case HALFWORDSIZE:  //half word (16-bit) access
                ((u16*) ( S3PVarArray[address].pAddr )) [ INDEX_reg ] = val;
                fDone = 1;
                break;
            case WORDSIZE:      //word (32-bit) access
                ((u32*) ( S3PVarArray[address].pAddr )) [ INDEX_reg ] = val;
                fDone = 1;
                break;
        }
    }

    if ( fDone )
    {
        //Post increment when mode is set
        if ( S3P_CSW_reg & S3P_CSW_MULTIPLE_MASK )
        {
            INDEX_reg++;
        }
    }
    else
    {
        //report param error if params error (!)
        if ( !ret )
            ret = S3P_ERR_CMD;
    }

    Last_WrittenReg = address;
    Last_AccessReg = Last_WrittenReg;

    return ret;
}





/*******************************************************************************
* Function Name  : GetValSPI
* Description    : Read an object on the SPI (Master mode)
* Input          : size:        number of bytes to be read (either 1 or 4).
* Output         : None.
* Return         : value (either u8 or u32 but coded into u32).
*******************************************************************************/
u32  Tap::GetValSPI( u8 size )
{
    u32 val, i;
    u32 last = size * 8 ;
    u32 mask = 1;

    val = 0;

    /* Configure IOs for reception */
    pinMode(Pin_IO, INPUT); 
    pinMode(Pin_CK, OUTPUT); 
    
    /* Get the (N = 8*size) bits from IoTize */
    for ( i = 0 ; i < last ; i++ )  // loop for each bit
    {
        // Generate the rising edge of the clock
        digitalWrite(Pin_CK, HIGH);   

        // Generate the falling edge of the clock
        digitalWrite(Pin_CK, LOW);   
        
        if ( digitalRead(Pin_IO))
        {
            val |= mask;
        }
        // Wait for synchro if target faster than IoTize.
        mask <<= 1;
    }
    return val;
}

/*******************************************************************************
* Function Name  : S3P_SendValSPI
* Description    : Send an object onto the SPI (Master mode)
* Input          : Val:  value (u32) to be sent (either a byte or a word depending on size)
*                  Size: number of bytes to be read (either 1, 2 or 4). Only 1 and 4 are supposed
*                  to be used at the moment.
* Output         : None.
* Return         : None
*******************************************************************************/
void Tap::SendValSPI( u32 val, u8 size )
{
    u32 mask, i;

    /* Configure IOs for transmission */
    pinMode(Pin_IO, OUTPUT);
    pinMode(Pin_CK, OUTPUT);

    mask = 1;
    
    // Wait for synchro if target faster than IoTize.
    //DELAYSOFT( TYPICAL_DELAYSOFT );

    /* Send the (N = 8*size) bits from IoTize */
    for ( i = 0 ; i < ( size * 8 ) ; i++ )
    {
        // Write the bit value
        digitalWrite(Pin_IO, ( val & mask ) ? HIGH : LOW);  
        
        // Generate the clock pulse
        digitalWrite(Pin_CK, HIGH );  
        digitalWrite(Pin_CK, LOW);  
        
        // Next bit for LSB
        mask <<= 1;
    }   
}
/*============================================================================*\
*                           (C) COPYRIGHT IoTize
*                               END OF FILE
*
\*============================================================================*/
