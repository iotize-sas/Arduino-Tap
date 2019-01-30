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
**********************************************************************************
*@file          : S3P_handler.h
*@author        : Francis Lamotte
*@version       : $Id: S3P_handler.h 4264 2016-06-21 08:43:34Z vincent $
*@brief         :
*********************************************************************************/
#ifndef    _TAP_H_
#define    _TAP_H_

/* Private type ----------------------------------------------------------------*/
typedef unsigned char   u8;         /*!< represents an unsigned  8bit-wide type */
typedef unsigned short  u16;        /*!< represents an unsigned 16bit-wide type */
typedef unsigned long   u32;        /*!< represents an unsigned 32bit-wide type */
typedef signed char     s8;         /*!< represents a signed  8bit-wide type    */
typedef signed short    s16;        /*!< represents a signed 16bit-wide type    */
typedef signed long     s32;        /*!< represents a signed 32bit-wide type    */


//S3P error codes

typedef unsigned int S3P_err;

#define _S3P_INDEXEDMODE_C_ 1
#define S3P_NOERROR         (0x00)

//bit 7 of ACK holds the parity of the ACK
#define S3P_ACK_PARITYBIT       (7)
#define S3P_ACK_PARITYBIT_MASK  (0x80)

//in both ACK and Size, error code is in bits 6-0
#define S3P_ERR_CODE_MASK   (0x7F)

#define S3P_ERR_OK  0 //no error
#define S3P_ERR_DEV 1 //device error (internal hardware error...)
#define S3P_ERR_CMD 2 //command error (bad conf...)
#define S3P_ERR_COM 3 //communication error (bad signals...)
#define S3P_ERR_AR  4 //access rights error (=unauthorized request)
#define S3P_ERR_UNKNOWN  (-1) //unknown error (=internal software error)


/* Various constant definitions */
#define BYTESIZE            sizeof(u8)   //1
#define HALFWORDSIZE        sizeof(u16)  //2
#define WORDSIZE            sizeof(u32)  //4

/* Some constant definitions */
#define TCOMMAND            u8
#define SIZE_COMMAND        sizeof(TCOMMAND)
#define READACCESS          1
#define WRITEACCESS         2


/* Specific definitions: need to be specified */
#define     S3P_ID_VAL       0

//S3P CSW register (not identical to SWD's CSW)
extern u32 S3P_CSW_reg;

//masks for S3P_CSW (not identical to SWD's CSW)

#define S3P_CSW_READ_FEATURES_MASK      0xE0000000

#define S3P_CSW_FEATURE_MASK_MEMORY_ACCESS_ENABLE   0x80000000
#define S3P_CSW_FEATURE_MASK_COMBUF_ENABLE          0x40000000
#define S3P_CSW_FEATURE_MASK_INDEXED_MODE_ENABLE    0x20000000

#define S3P_CSW_MULTIPLE_MASK           0x0000000C //defines auto-inc modes: 0=no auto-inc, 1=auto-inc, 2&3 reserved 
#define S3P_CSW_SIZE_MASK               0x00000003 //defines the size of the accesses: 0=byte, 1=halfword, 2=word, 3=as requested in frame (no pack)
#define S3P_CSW_SIZE_SHIFT              0 //shift for extracting size from CSW



#define S3P_INVALID_TAR_REG (0xFFFFFFFF)
#define S3P_INVALID_CSW_REG (0xFFFFFFFF)


//The following flags are semaphore to perform IoTize<=>target communication.
#define S3P_CSW_COMBUF_MASK             0x00000030
#define S3P_CSW_COMBUF_TR               0x00000010 //Transfer buffer Ready to send some data
#define S3P_CSW_COMBUF_RF               0x00000020 //Reception Full, unable to receive data for now

#define S3P_CSW_READ_ONLY_MASK          ( S3P_CSW_COMBUF_MASK )


//The following definitions specify the address of the S3P Control/Status registers
typedef enum _S3P_Register_addr
{
    S3P_CSW_AD      = 0x00, //Control and Status (for SWD and also for the Indexed mode)
    S3P_TAR_AD      = 0x01, //Address (TAR for SWD, Offset for the Indexed mode)
    S3P_DRW_AD      = 0x02, //Data (for SWD)
    S3P_COMBUF_AD   = 0x03, //Data (for Combuf)
    //S3P_..._AD = 0x04, //unused...
    //S3P_..._AD = 0x05, //unused...
    //S3P_..._AD = 0x06, //unused...
    S3P_ID_AD       = 0x07,
    S3P_MAXREG      = 0x08
} S3P_Register_addr;


/* Hardware specific exported functions */
extern void S3P_ConfigureIOs( void );
extern void S3P_PrepareCom( void );
extern void S3P_EndOfCom( void );
extern void S3P_SendValSPI( u32 val, u8 size );
extern u32  S3P_GetValSPI ( u8 size );

typedef u16 S3P_Indreg_addr;


/* Macro definitions ---------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

class Tap {
  public:
    Tap( ){
        Command = Cmd2 = Size = Addr = Value = INDEX_reg = 0;
        Last_WrittenReg = Last_ReadReg = Last_AccessReg = 0xff;
        }
    void Init ( int CLK, int IO);       /* This function initialises the hw and irq            */
    void IrqHandler( void );   /* Interrupt handler (to be called from the IRQ vector */  

  private:
 
    typedef u16 S3P_Indreg_addr;
    int Parity8( u8 inbyte );
    int BitCount8( u8 inbyte );
    
    S3P_err WriteIndexedReg( S3P_Indreg_addr address, u8 size, u32 val );
    S3P_err ReadIndexedReg( S3P_Indreg_addr address, u8 size, u32* pval );
    S3P_err WriteReg( S3P_Register_addr address, u8 size, u32 val );
    S3P_err ReadReg( S3P_Register_addr address, u8 size, u32* pval );

    void SendValSPI( u32 val, u8 size );
    u32  GetValSPI( u8 size );
    void EndOfCom( void );
    void PrepareCom( void );
    void ConfigureIOs( void );


/* User check functions to be customized */
    virtual S3P_err CheckIndexedReg( S3P_Indreg_addr addr, u8 size, int accessmode ) { return 0; }
    

  private:
    byte Pin_CK ;
    byte Pin_IO ;

    u8 Command ; //note: 0 is not a valid command thanks to parity bit. use value 0 to indicate "waiting for command" status
    u8 Cmd2 ;
    u8 Size ; //note: size.7=1 means error, and in this case the error code is in size[6..0]
    u16 Addr ;
    u32 Value ;
    u8 INDEX_reg;

    u8 Last_WrittenReg;
    u8 Last_ReadReg;
    u8 Last_AccessReg;
    int   ArraySize;
} ;
extern Tap myTap;               /* The unique class instance */
extern void S3P_IrqHandler( void );   /* Interrupt handler (to be called from the IRQ vector */


/*============================================================================*\
*                           (C) COPYRIGHT IoTize
*                               END OF FILE
*
\*============================================================================*/
#endif            // _TAP_H_
