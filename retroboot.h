/*=============================================================================
 * RetroBoot - Advanced UEFI Legacy BIOS Emulation Layer
 * retroboot.h - Master Header: All Types, Structures, Protocols, Constants
 *
 * Build:
 *   x86_64-w64-mingw32-gcc -Wall -Wextra -std=c11 -O2            \
 *     -ffreestanding -fno-stack-protector -fno-stack-check         \
 *     -fno-strict-aliasing -mno-red-zone -maccumulate-outgoing-args\
 *     -nostdlib -shared -Wl,--subsystem,10 -Wl,-dll               \
 *     -e efi_main retroboot.c -o RETROBOOT.EFI
 *
 *   Then: copy RETROBOOT.EFI to EFI/BOOT/BOOTX64.EFI on a FAT32 ESP
 *
 * Architecture: x86-64 UEFI → 32-bit PM → 16-bit PM → Real Mode → MBR
 *=============================================================================*/

#ifndef RETROBOOT_H
#define RETROBOOT_H

/*-----------------------------------------------------------------------------
 * Compiler Portability
 *---------------------------------------------------------------------------*/
#if defined(__GNUC__) || defined(__clang__)
#  define RB_PACKED          __attribute__((packed))
#  define RB_ALIGNED(n)      __attribute__((aligned(n)))
#  define RB_NORETURN        __attribute__((noreturn))
#  define RB_NAKED           __attribute__((naked))
#  define RB_NOINLINE        __attribute__((noinline))
#  define RB_SECTION(s)      __attribute__((section(s)))
#  define RB_USED            __attribute__((used))
#  define RB_UNUSED          __attribute__((unused))
#  define RB_ALWAYS_INLINE   __attribute__((always_inline)) inline
#  define RB_LIKELY(x)       __builtin_expect(!!(x), 1)
#  define RB_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#  define RB_BARRIER()       __asm__ __volatile__("" ::: "memory")
#else
#  error "Unsupported compiler. Use GCC or Clang."
#endif

/*-----------------------------------------------------------------------------
 * Fundamental Types (EFI-compatible, no stdlib)
 *---------------------------------------------------------------------------*/
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;
typedef signed char         INT8;
typedef signed short        INT16;
typedef signed int          INT32;
typedef signed long long    INT64;
typedef UINT8               BOOLEAN;
typedef UINT16              CHAR16;
typedef void                VOID;
typedef UINT64              UINTN;
typedef INT64               INTN;

#define TRUE   ((BOOLEAN)1)
#define FALSE  ((BOOLEAN)0)
#define NULL   ((VOID*)0)

/* EFI calling convention */
#ifdef _WIN32
#  define EFIAPI __cdecl
#else
#  define EFIAPI
#endif

typedef UINTN               EFI_STATUS;
typedef VOID*               EFI_HANDLE;
typedef VOID*               EFI_EVENT;
typedef UINT64              EFI_LBA;
typedef UINT64              EFI_PHYSICAL_ADDRESS;
typedef UINT64              EFI_VIRTUAL_ADDRESS;
typedef UINTN               EFI_TPL;

/*-----------------------------------------------------------------------------
 * EFI Status Codes
 *---------------------------------------------------------------------------*/
#define EFI_SUCCESS                    0ULL
#define EFI_ERROR_BIT                  (1ULL << 63)
#define EFI_LOAD_ERROR                 (EFI_ERROR_BIT | 1ULL)
#define EFI_INVALID_PARAMETER          (EFI_ERROR_BIT | 2ULL)
#define EFI_UNSUPPORTED                (EFI_ERROR_BIT | 3ULL)
#define EFI_BAD_BUFFER_SIZE            (EFI_ERROR_BIT | 4ULL)
#define EFI_BUFFER_TOO_SMALL           (EFI_ERROR_BIT | 5ULL)
#define EFI_NOT_READY                  (EFI_ERROR_BIT | 6ULL)
#define EFI_DEVICE_ERROR               (EFI_ERROR_BIT | 7ULL)
#define EFI_WRITE_PROTECTED            (EFI_ERROR_BIT | 8ULL)
#define EFI_OUT_OF_RESOURCES           (EFI_ERROR_BIT | 9ULL)
#define EFI_VOLUME_CORRUPTED           (EFI_ERROR_BIT | 10ULL)
#define EFI_VOLUME_FULL                (EFI_ERROR_BIT | 11ULL)
#define EFI_NO_MEDIA                   (EFI_ERROR_BIT | 12ULL)
#define EFI_MEDIA_CHANGED              (EFI_ERROR_BIT | 13ULL)
#define EFI_NOT_FOUND                  (EFI_ERROR_BIT | 14ULL)
#define EFI_ACCESS_DENIED              (EFI_ERROR_BIT | 15ULL)
#define EFI_NO_RESPONSE                (EFI_ERROR_BIT | 16ULL)
#define EFI_NO_MAPPING                 (EFI_ERROR_BIT | 17ULL)
#define EFI_TIMEOUT                    (EFI_ERROR_BIT | 18ULL)
#define EFI_NOT_STARTED                (EFI_ERROR_BIT | 19ULL)
#define EFI_ALREADY_STARTED            (EFI_ERROR_BIT | 20ULL)
#define EFI_ABORTED                    (EFI_ERROR_BIT | 21ULL)
#define EFI_ICMP_ERROR                 (EFI_ERROR_BIT | 22ULL)
#define EFI_TFTP_ERROR                 (EFI_ERROR_BIT | 23ULL)
#define EFI_PROTOCOL_ERROR             (EFI_ERROR_BIT | 24ULL)
#define EFI_INCOMPATIBLE_VERSION       (EFI_ERROR_BIT | 25ULL)
#define EFI_SECURITY_VIOLATION         (EFI_ERROR_BIT | 26ULL)
#define EFI_WARN_UNKNOWN_GLYPH         (1ULL)
#define EFI_WARN_DELETE_FAILURE        (2ULL)
#define EFI_WARN_WRITE_FAILURE         (3ULL)
#define EFI_WARN_BUFFER_TOO_SMALL      (4ULL)

#define EFI_ERROR(x)  (((INTN)(x)) < 0)

/*-----------------------------------------------------------------------------
 * EFI GUID
 *---------------------------------------------------------------------------*/
typedef struct {
    UINT32  Data1;
    UINT16  Data2;
    UINT16  Data3;
    UINT8   Data4[8];
} RB_PACKED EFI_GUID;

#define GUID_INIT(d1,d2,d3,b0,b1,b2,b3,b4,b5,b6,b7) \
    { (d1), (d2), (d3), { (b0),(b1),(b2),(b3),(b4),(b5),(b6),(b7) } }

/* Protocol GUIDs */
#define EFI_BLOCK_IO_PROTOCOL_GUID \
    GUID_INIT(0x964e5b21,0x6459,0x11d2,0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b)

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
    GUID_INIT(0x9042a9de,0x23dc,0x4a38,0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a)

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
    GUID_INIT(0x5b1b31a1,0x9562,0x11d2,0x8e,0x3f,0x00,0xa0,0xc9,0x69,0x72,0x3b)

#define EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID \
    GUID_INIT(0x387477c1,0x69c7,0x11d2,0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b)

#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
    GUID_INIT(0x387477c2,0x69c7,0x11d2,0x8e,0x38,0x00,0xa0,0xc9,0x69,0x72,0x3b)

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
    GUID_INIT(0x09576e91,0x6d3f,0x11d2,0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b)

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
    GUID_INIT(0x0964e5b22,0x6459,0x11d2,0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b)

#define EFI_DISK_IO_PROTOCOL_GUID \
    GUID_INIT(0xce345171,0xba0b,0x11d2,0x8e,0x4f,0x00,0xa0,0xc9,0x69,0x72,0x3b)

#define EFI_GLOBAL_VARIABLE_GUID \
    GUID_INIT(0x8be4df61,0x93ca,0x11d2,0xaa,0x0d,0x00,0xe0,0x98,0x03,0x2b,0x8c)

#define EFI_ACPI_20_TABLE_GUID \
    GUID_INIT(0x8868e871,0xe4f1,0x11d3,0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81)

#define EFI_ACPI_TABLE_GUID \
    GUID_INIT(0xeb9d2d30,0x2d88,0x11d3,0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d)

/*-----------------------------------------------------------------------------
 * EFI Time
 *---------------------------------------------------------------------------*/
typedef struct {
    UINT16  Year;
    UINT8   Month;
    UINT8   Day;
    UINT8   Hour;
    UINT8   Minute;
    UINT8   Second;
    UINT8   Pad1;
    UINT32  Nanosecond;
    INT16   TimeZone;
    UINT8   Daylight;
    UINT8   Pad2;
} EFI_TIME;

typedef struct {
    UINT32  Resolution;
    UINT32  Accuracy;
    BOOLEAN SetsToZero;
} EFI_TIME_CAPABILITIES;

/*-----------------------------------------------------------------------------
 * EFI Memory
 *---------------------------------------------------------------------------*/
typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

#define EFI_MEMORY_UC           0x0000000000000001ULL
#define EFI_MEMORY_WC           0x0000000000000002ULL
#define EFI_MEMORY_WT           0x0000000000000004ULL
#define EFI_MEMORY_WB           0x0000000000000008ULL
#define EFI_MEMORY_UCE          0x0000000000000010ULL
#define EFI_MEMORY_WP           0x0000000000001000ULL
#define EFI_MEMORY_RP           0x0000000000002000ULL
#define EFI_MEMORY_XP           0x0000000000004000ULL
#define EFI_MEMORY_NV           0x0000000000008000ULL
#define EFI_MEMORY_MORE_RELIABLE 0x0000000000010000ULL
#define EFI_MEMORY_RO           0x0000000000020000ULL
#define EFI_MEMORY_SP           0x0000000000040000ULL
#define EFI_MEMORY_CPU_CRYPTO   0x0000000000080000ULL
#define EFI_MEMORY_RUNTIME      0x8000000000000000ULL

typedef struct {
    UINT32              Type;
    EFI_PHYSICAL_ADDRESS PhysicalStart;
    EFI_VIRTUAL_ADDRESS VirtualStart;
    UINT64              NumberOfPages;
    UINT64              Attribute;
} EFI_MEMORY_DESCRIPTOR;

/*-----------------------------------------------------------------------------
 * EFI Allocate Types
 *---------------------------------------------------------------------------*/
typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

/*-----------------------------------------------------------------------------
 * EFI Input Key
 *---------------------------------------------------------------------------*/
typedef struct {
    UINT16  ScanCode;
    CHAR16  UnicodeChar;
} EFI_INPUT_KEY;

/*-----------------------------------------------------------------------------
 * EFI Simple Text Output Protocol
 *---------------------------------------------------------------------------*/
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    BOOLEAN                                  ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_OUTPUT_STRING)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    CHAR16                                  *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_TEST_STRING)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    CHAR16                                  *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_QUERY_MODE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN                                    ModeNumber,
    UINTN                                   *Columns,
    UINTN                                   *Rows);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_MODE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN                                    ModeNumber);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN                                    Attribute);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_CURSOR_POSITION)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN                                    Column,
    UINTN                                    Row);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_ENABLE_CURSOR)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    BOOLEAN                                  Visible);

typedef struct {
    INT32   MaxMode;
    INT32   Mode;
    INT32   Attribute;
    INT32   CursorColumn;
    INT32   CursorRow;
    BOOLEAN CursorVisible;
} SIMPLE_TEXT_OUTPUT_MODE;

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET              Reset;
    EFI_TEXT_OUTPUT_STRING      OutputString;
    EFI_TEXT_TEST_STRING        TestString;
    EFI_TEXT_QUERY_MODE         QueryMode;
    EFI_TEXT_SET_MODE           SetMode;
    EFI_TEXT_SET_ATTRIBUTE      SetAttribute;
    EFI_TEXT_CLEAR_SCREEN       ClearScreen;
    EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
    EFI_TEXT_ENABLE_CURSOR      EnableCursor;
    SIMPLE_TEXT_OUTPUT_MODE    *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

/* Text Attributes */
#define EFI_BLACK         0x00
#define EFI_BLUE          0x01
#define EFI_GREEN         0x02
#define EFI_CYAN          0x03
#define EFI_RED           0x04
#define EFI_MAGENTA       0x05
#define EFI_BROWN         0x06
#define EFI_LIGHTGRAY     0x07
#define EFI_BRIGHT        0x08
#define EFI_DARKGRAY      0x08
#define EFI_LIGHTBLUE     0x09
#define EFI_LIGHTGREEN    0x0A
#define EFI_LIGHTCYAN     0x0B
#define EFI_LIGHTRED      0x0C
#define EFI_LIGHTMAGENTA  0x0D
#define EFI_YELLOW        0x0E
#define EFI_WHITE         0x0F
#define EFI_BACKGROUND_BLACK      0x00
#define EFI_BACKGROUND_BLUE       0x10
#define EFI_BACKGROUND_GREEN      0x20
#define EFI_BACKGROUND_CYAN       0x30
#define EFI_BACKGROUND_RED        0x40
#define EFI_BACKGROUND_MAGENTA    0x50
#define EFI_BACKGROUND_BROWN      0x60
#define EFI_BACKGROUND_LIGHTGRAY  0x70
#define EFI_TEXT_ATTR(fg,bg)  ((fg) | ((bg) << 4))

/*-----------------------------------------------------------------------------
 * EFI Simple Text Input Protocol
 *---------------------------------------------------------------------------*/
struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET)(
    struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    BOOLEAN                                 ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(
    struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    EFI_INPUT_KEY                          *Key);

typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET    Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    EFI_EVENT          WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

/*-----------------------------------------------------------------------------
 * EFI Runtime Services
 *---------------------------------------------------------------------------*/
typedef EFI_STATUS (EFIAPI *EFI_GET_TIME)(EFI_TIME*, EFI_TIME_CAPABILITIES*);
typedef EFI_STATUS (EFIAPI *EFI_SET_TIME)(EFI_TIME*);
typedef EFI_STATUS (EFIAPI *EFI_GET_WAKEUP_TIME)(BOOLEAN*, BOOLEAN*, EFI_TIME*);
typedef EFI_STATUS (EFIAPI *EFI_SET_WAKEUP_TIME)(BOOLEAN, EFI_TIME*);
typedef EFI_STATUS (EFIAPI *EFI_SET_VIRTUAL_ADDRESS_MAP)(
    UINTN, UINTN, UINT32, EFI_MEMORY_DESCRIPTOR*);
typedef EFI_STATUS (EFIAPI *EFI_CONVERT_POINTER)(UINTN, VOID**);
typedef EFI_STATUS (EFIAPI *EFI_GET_VARIABLE)(
    CHAR16*, EFI_GUID*, UINT32*, UINTN*, VOID*);
typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_VARIABLE_NAME)(
    UINTN*, CHAR16*, EFI_GUID*);
typedef EFI_STATUS (EFIAPI *EFI_SET_VARIABLE)(
    CHAR16*, EFI_GUID*, UINT32, UINTN, VOID*);
typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_HIGH_MONO_COUNT)(UINT32*);
typedef VOID       (EFIAPI *EFI_RESET_SYSTEM)(UINT32, EFI_STATUS, UINTN, VOID*);
typedef EFI_STATUS (EFIAPI *EFI_UPDATE_CAPSULE)(VOID**, UINTN, EFI_PHYSICAL_ADDRESS);
typedef EFI_STATUS (EFIAPI *EFI_QUERY_CAPSULE_CAPABILITIES)(
    VOID**, UINTN, UINT64*, UINT32*);
typedef EFI_STATUS (EFIAPI *EFI_QUERY_VARIABLE_INFO)(
    UINT32, UINT64*, UINT64*, UINT64*);

#define EFI_RUNTIME_SERVICES_SIGNATURE 0x56524553544e5552ULL
#define EFI_RUNTIME_SERVICES_REVISION  0x00020000

typedef struct {
    UINT64                       Signature;
    UINT32                       Revision;
    UINT32                       HeaderSize;
    UINT32                       CRC32;
    UINT32                       Reserved;
    EFI_GET_TIME                 GetTime;
    EFI_SET_TIME                 SetTime;
    EFI_GET_WAKEUP_TIME          GetWakeupTime;
    EFI_SET_WAKEUP_TIME          SetWakeupTime;
    EFI_SET_VIRTUAL_ADDRESS_MAP  SetVirtualAddressMap;
    EFI_CONVERT_POINTER          ConvertPointer;
    EFI_GET_VARIABLE             GetVariable;
    EFI_GET_NEXT_VARIABLE_NAME   GetNextVariableName;
    EFI_SET_VARIABLE             SetVariable;
    EFI_GET_NEXT_HIGH_MONO_COUNT GetNextHighMonotonicCount;
    EFI_RESET_SYSTEM             ResetSystem;
    EFI_UPDATE_CAPSULE           UpdateCapsule;
    EFI_QUERY_CAPSULE_CAPABILITIES QueryCapsuleCapabilities;
    EFI_QUERY_VARIABLE_INFO      QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

/*-----------------------------------------------------------------------------
 * EFI Boot Services
 *---------------------------------------------------------------------------*/
typedef EFI_STATUS (EFIAPI *EFI_RAISE_TPL)(EFI_TPL);
typedef VOID       (EFIAPI *EFI_RESTORE_TPL)(EFI_TPL);
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_PAGES)(
    EFI_ALLOCATE_TYPE, EFI_MEMORY_TYPE, UINTN, EFI_PHYSICAL_ADDRESS*);
typedef EFI_STATUS (EFIAPI *EFI_FREE_PAGES)(EFI_PHYSICAL_ADDRESS, UINTN);
typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP)(
    UINTN*, EFI_MEMORY_DESCRIPTOR*, UINTN*, UINTN*, UINT32*);
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)(EFI_MEMORY_TYPE, UINTN, VOID**);
typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL)(VOID*);
typedef EFI_STATUS (EFIAPI *EFI_CREATE_EVENT)(
    UINT32, EFI_TPL, VOID*, VOID*, EFI_EVENT*);
typedef EFI_STATUS (EFIAPI *EFI_SET_TIMER)(EFI_EVENT, UINT32, UINT64);
typedef EFI_STATUS (EFIAPI *EFI_WAIT_FOR_EVENT)(UINTN, EFI_EVENT*, UINTN*);
typedef EFI_STATUS (EFIAPI *EFI_SIGNAL_EVENT)(EFI_EVENT);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_EVENT)(EFI_EVENT);
typedef EFI_STATUS (EFIAPI *EFI_CHECK_EVENT)(EFI_EVENT);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE*, EFI_GUID*, UINT32, VOID*);
typedef EFI_STATUS (EFIAPI *EFI_REINSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE, EFI_GUID*, VOID*, VOID*);
typedef EFI_STATUS (EFIAPI *EFI_UNINSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE, EFI_GUID*, VOID*);
typedef EFI_STATUS (EFIAPI *EFI_HANDLE_PROTOCOL)(
    EFI_HANDLE, EFI_GUID*, VOID**);
typedef EFI_STATUS (EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY)(
    EFI_GUID*, EFI_EVENT, VOID**);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE)(
    UINT32, EFI_GUID*, VOID*, UINTN*, EFI_HANDLE*);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_DEVICE_PATH)(
    EFI_GUID*, VOID**, EFI_HANDLE*);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_CONFIGURATION_TABLE)(EFI_GUID*, VOID*);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_LOAD)(
    BOOLEAN, EFI_HANDLE, VOID*, VOID*, UINTN, EFI_HANDLE*);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_START)(EFI_HANDLE, UINTN*, CHAR16**);
typedef EFI_STATUS (EFIAPI *EFI_EXIT)(EFI_HANDLE, EFI_STATUS, UINTN, CHAR16*);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_UNLOAD)(EFI_HANDLE);
typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE, UINTN);
typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_MONOTONIC_COUNT)(UINT64*);
typedef EFI_STATUS (EFIAPI *EFI_STALL)(UINTN);
typedef EFI_STATUS (EFIAPI *EFI_SET_WATCHDOG_TIMER)(UINTN, UINT64, UINTN, CHAR16*);
typedef EFI_STATUS (EFIAPI *EFI_CONNECT_CONTROLLER)(
    EFI_HANDLE, EFI_HANDLE*, VOID*, BOOLEAN);
typedef EFI_STATUS (EFIAPI *EFI_DISCONNECT_CONTROLLER)(
    EFI_HANDLE, EFI_HANDLE, EFI_HANDLE);
typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL)(
    EFI_HANDLE, EFI_GUID*, VOID**, EFI_HANDLE, EFI_HANDLE, UINT32);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_PROTOCOL)(
    EFI_HANDLE, EFI_GUID*, EFI_HANDLE, EFI_HANDLE);
typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL_INFORMATION)(
    EFI_HANDLE, EFI_GUID*, VOID**, UINTN*);
typedef EFI_STATUS (EFIAPI *EFI_PROTOCOLS_PER_HANDLE)(
    EFI_HANDLE, EFI_GUID***, UINTN*);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE_BUFFER)(
    UINT32, EFI_GUID*, VOID*, UINTN*, EFI_HANDLE**);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_PROTOCOL)(EFI_GUID*, VOID*, VOID**);
typedef EFI_STATUS (EFIAPI *EFI_COPY_MEM_FN)(VOID*, VOID*, UINTN);
typedef EFI_STATUS (EFIAPI *EFI_SET_MEM_FN)(VOID*, UINTN, UINT8);
typedef EFI_STATUS (EFIAPI *EFI_CREATE_EVENT_EX)(
    UINT32, EFI_TPL, VOID*, VOID*, EFI_GUID*, EFI_EVENT*);

/* Open Protocol Attributes */
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL         0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL        0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER  0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER            0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE            0x00000020

/* Locate Handle Search Types */
#define AllHandles       0
#define ByRegisterNotify 1
#define ByProtocol       2

#define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42ULL
#define EFI_BOOT_SERVICES_REVISION  0x00020000

typedef struct {
    UINT64                          Signature;
    UINT32                          Revision;
    UINT32                          HeaderSize;
    UINT32                          CRC32;
    UINT32                          Reserved;
    /* Task Priority */
    EFI_RAISE_TPL                   RaiseTPL;
    EFI_RESTORE_TPL                 RestoreTPL;
    /* Memory */
    EFI_ALLOCATE_PAGES              AllocatePages;
    EFI_FREE_PAGES                  FreePages;
    EFI_GET_MEMORY_MAP              GetMemoryMap;
    EFI_ALLOCATE_POOL               AllocatePool;
    EFI_FREE_POOL                   FreePool;
    /* Events / Timers */
    EFI_CREATE_EVENT                CreateEvent;
    EFI_SET_TIMER                   SetTimer;
    EFI_WAIT_FOR_EVENT              WaitForEvent;
    EFI_SIGNAL_EVENT                SignalEvent;
    EFI_CLOSE_EVENT                 CloseEvent;
    EFI_CHECK_EVENT                 CheckEvent;
    /* Protocol Interfaces */
    EFI_INSTALL_PROTOCOL_INTERFACE  InstallProtocolInterface;
    EFI_REINSTALL_PROTOCOL_INTERFACE ReinstallProtocolInterface;
    EFI_UNINSTALL_PROTOCOL_INTERFACE UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL             HandleProtocol;
    VOID                           *Reserved2;
    EFI_REGISTER_PROTOCOL_NOTIFY    RegisterProtocolNotify;
    EFI_LOCATE_HANDLE               LocateHandle;
    EFI_LOCATE_DEVICE_PATH          LocateDevicePath;
    EFI_INSTALL_CONFIGURATION_TABLE InstallConfigurationTable;
    /* Image Services */
    EFI_IMAGE_LOAD                  LoadImage;
    EFI_IMAGE_START                 StartImage;
    EFI_EXIT                        Exit;
    EFI_IMAGE_UNLOAD                UnloadImage;
    EFI_EXIT_BOOT_SERVICES          ExitBootServices;
    /* Miscellaneous */
    EFI_GET_NEXT_MONOTONIC_COUNT    GetNextMonotonicCount;
    EFI_STALL                       Stall;
    EFI_SET_WATCHDOG_TIMER          SetWatchdogTimer;
    /* Driver Support */
    EFI_CONNECT_CONTROLLER          ConnectController;
    EFI_DISCONNECT_CONTROLLER       DisconnectController;
    /* Open/Close Protocol */
    EFI_OPEN_PROTOCOL               OpenProtocol;
    EFI_CLOSE_PROTOCOL              CloseProtocol;
    EFI_OPEN_PROTOCOL_INFORMATION   OpenProtocolInformation;
    /* Library Services */
    EFI_PROTOCOLS_PER_HANDLE        ProtocolsPerHandle;
    EFI_LOCATE_HANDLE_BUFFER        LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL             LocateProtocol;
    VOID                           *InstallMultipleProtocolInterfaces;
    VOID                           *UninstallMultipleProtocolInterfaces;
    /* 32-bit CRC */
    VOID                           *CalculateCrc32;
    /* Memory Utilities */
    VOID                           *CopyMem;
    VOID                           *SetMem;
    EFI_CREATE_EVENT_EX             CreateEventEx;
} EFI_BOOT_SERVICES;

/*-----------------------------------------------------------------------------
 * EFI Configuration Table
 *---------------------------------------------------------------------------*/
typedef struct {
    EFI_GUID  VendorGuid;
    VOID     *VendorTable;
} EFI_CONFIGURATION_TABLE;

/*-----------------------------------------------------------------------------
 * EFI System Table
 *---------------------------------------------------------------------------*/
#define EFI_SYSTEM_TABLE_SIGNATURE  0x5453595320494249ULL
#define EFI_SYSTEM_TABLE_REVISION   0x00020000

typedef struct {
    UINT64                          Signature;
    UINT32                          Revision;
    UINT32                          HeaderSize;
    UINT32                          CRC32;
    UINT32                          Reserved;
    CHAR16                         *FirmwareVendor;
    UINT32                          FirmwareRevision;
    EFI_HANDLE                      ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    EFI_HANDLE                      ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                      StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES           *RuntimeServices;
    EFI_BOOT_SERVICES              *BootServices;
    UINTN                           NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE        *ConfigurationTable;
} EFI_SYSTEM_TABLE;

/*-----------------------------------------------------------------------------
 * EFI Loaded Image Protocol
 *---------------------------------------------------------------------------*/
#define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000

typedef struct {
    UINT32       Revision;
    EFI_HANDLE   ParentHandle;
    EFI_SYSTEM_TABLE *SystemTable;
    EFI_HANDLE   DeviceHandle;
    VOID        *FilePath;
    VOID        *Reserved;
    UINT32       LoadOptionsSize;
    VOID        *LoadOptions;
    VOID        *ImageBase;
    UINT64       ImageSize;
    UINT32       ImageCodeType;
    UINT32       ImageDataType;
    EFI_IMAGE_UNLOAD Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

/*-----------------------------------------------------------------------------
 * EFI Block IO Protocol
 *---------------------------------------------------------------------------*/
typedef struct {
    UINT32  MediaId;
    BOOLEAN RemovableMedia;
    BOOLEAN MediaPresent;
    BOOLEAN LogicalPartition;
    BOOLEAN ReadOnly;
    BOOLEAN WriteCaching;
    UINT32  BlockSize;
    UINT32  IoAlign;
    EFI_LBA LastBlock;
    EFI_LBA LowestAlignedLba;
    UINT32  LogicalBlocksPerPhysicalBlock;
    UINT32  OptimalTransferLengthGranularity;
} EFI_BLOCK_IO_MEDIA;

struct _EFI_BLOCK_IO_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_BLOCK_RESET)(
    struct _EFI_BLOCK_IO_PROTOCOL *This, BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_BLOCK_READ)(
    struct _EFI_BLOCK_IO_PROTOCOL *This, UINT32 MediaId,
    EFI_LBA Lba, UINTN BufferSize, VOID *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_BLOCK_WRITE)(
    struct _EFI_BLOCK_IO_PROTOCOL *This, UINT32 MediaId,
    EFI_LBA Lba, UINTN BufferSize, VOID *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_BLOCK_FLUSH)(
    struct _EFI_BLOCK_IO_PROTOCOL *This);

typedef struct _EFI_BLOCK_IO_PROTOCOL {
    UINT64               Revision;
    EFI_BLOCK_IO_MEDIA  *Media;
    EFI_BLOCK_RESET      Reset;
    EFI_BLOCK_READ       ReadBlocks;
    EFI_BLOCK_WRITE      WriteBlocks;
    EFI_BLOCK_FLUSH      FlushBlocks;
} EFI_BLOCK_IO_PROTOCOL;

#define EFI_BLOCK_IO_PROTOCOL_REVISION  0x00010000
#define EFI_BLOCK_IO_PROTOCOL_REVISION2 0x00020001
#define EFI_BLOCK_IO_PROTOCOL_REVISION3 0x00020031

/*-----------------------------------------------------------------------------
 * EFI Graphics Output Protocol
 *---------------------------------------------------------------------------*/
typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    UINT32  RedMask;
    UINT32  GreenMask;
    UINT32  BlueMask;
    UINT32  ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct {
    UINT32                     Version;
    UINT32                     HorizontalResolution;
    UINT32                     VerticalResolution;
    EFI_GRAPHICS_PIXEL_FORMAT  PixelFormat;
    EFI_PIXEL_BITMASK          PixelInformation;
    UINT32                     PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32                             MaxMode;
    UINT32                             Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN                              SizeOfInfo;
    EFI_PHYSICAL_ADDRESS               FrameBufferBase;
    UINTN                              FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

struct _EFI_GRAPHICS_OUTPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL *This, UINT32 ModeNumber,
    UINTN *SizeOfInfo, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info);
typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL *This, UINT32 ModeNumber);
typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL *This, VOID *BltBuffer,
    UINT32 BltOperation, UINTN SrcX, UINTN SrcY,
    UINTN DestX, UINTN DestY, UINTN Width, UINTN Height, UINTN Delta);

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE QueryMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE   SetMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT        Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE      *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

/*-----------------------------------------------------------------------------
 * EFI Device Path Protocol
 *---------------------------------------------------------------------------*/
typedef struct {
    UINT8   Type;
    UINT8   SubType;
    UINT16  Length;
} EFI_DEVICE_PATH_PROTOCOL;

#define END_DEVICE_PATH_TYPE            0x7F
#define END_ENTIRE_DEVICE_PATH_SUBTYPE  0xFF

/*=============================================================================
 * BIOS / Legacy Structures
 *=============================================================================*/

/*-----------------------------------------------------------------------------
 * BIOS Data Area (BDA) - at physical 0x0400
 *---------------------------------------------------------------------------*/
typedef struct RB_PACKED {
    UINT16  com_port[4];           /* 0x400: COM port base addresses */
    UINT16  lpt_port[3];           /* 0x408: LPT port base addresses */
    UINT16  ebda_segment;          /* 0x40E: EBDA segment (>> 4) */
    UINT16  equipment_flags;       /* 0x410: Equipment flags */
    UINT8   mfr_test;              /* 0x412: POST result code */
    UINT16  conv_mem_kb;           /* 0x413: Conventional memory (KB) */
    UINT16  reserved_0415;         /* 0x415: Reserved */
    UINT8   keyboard_flags1;       /* 0x417: Keyboard status flags 1 */
    UINT8   keyboard_flags2;       /* 0x418: Keyboard status flags 2 */
    UINT8   alt_keypad;            /* 0x419: Alt-Numpad work area */
    UINT16  kbd_buffer_head;       /* 0x41A: Keyboard buffer head ptr */
    UINT16  kbd_buffer_tail;       /* 0x41C: Keyboard buffer tail ptr */
    UINT16  kbd_buffer[16];        /* 0x41E: Keyboard buffer (32 bytes) */
    UINT8   display_mode;          /* 0x449: Display mode */
    UINT16  display_cols;          /* 0x44A: Display columns */
    UINT16  video_page_size;       /* 0x44C: Video page size in bytes */
    UINT16  video_page_offset;     /* 0x44E: Video page offset */
    UINT16  cursor_pos[8];         /* 0x450: Cursor position (8 pages) */
    UINT16  cursor_shape;          /* 0x460: Cursor shape */
    UINT8   active_page;           /* 0x462: Active display page */
    UINT16  crt_base_port;         /* 0x463: CRT controller base address */
    UINT8   crt_mode_ctrl;         /* 0x465: CRT mode control register */
    UINT8   color_palette;         /* 0x466: Color palette mask register */
    UINT32  rom_init_ptr;          /* 0x467: Pointer to ROM BASIC init */
    UINT8   last_intr;             /* 0x46B: Last interrupt number */
    UINT32  timer_ticks;           /* 0x46C: Timer tick count */
    UINT8   timer_overflow;        /* 0x470: Timer midnight flag */
    UINT8   break_flag;            /* 0x471: Ctrl-Break flag */
    UINT16  soft_reset_flag;       /* 0x472: Soft reset flag (0x1234=warm) */
    UINT8   last_hd_status;        /* 0x474: Last HD operation status */
    UINT8   hd_count;              /* 0x475: Number of hard disks */
    UINT8   hd_ctrl_byte;          /* 0x476: HD control byte */
    UINT8   hd_io_offset;          /* 0x477: HD I/O port offset */
    UINT8   lpt_timeout[3];        /* 0x478: LPT timeouts */
    UINT8   com_timeout[4];        /* 0x47B: COM timeouts */
    UINT16  kbd_buffer_start;      /* 0x480: Kbd buffer start offset */
    UINT16  kbd_buffer_end;        /* 0x482: Kbd buffer end offset */
    UINT8   video_rows;            /* 0x484: Video rows minus 1 */
    UINT16  char_height;           /* 0x485: Character height (scan lines) */
    UINT8   video_ctrl_state;      /* 0x487: Video control state */
    UINT8   video_switches;        /* 0x488: Video mode switches */
    UINT8   shadow_ram_flags;      /* 0x489: Shadow RAM flags */
    UINT8   hard_disk_xmit;        /* 0x48A: Hard disk transmit rate */
    UINT8   display_combo;         /* 0x48B: Display combination */
    UINT8   last_diskette_status;  /* 0x48C: Last diskette operation status */
    UINT8   diskette_count;        /* 0x48D: Number of diskette drives */
    UINT8   diskette_ctrl_state;   /* 0x48E: Diskette control state */
    UINT8   diskette_current_track[2]; /* 0x48F: Current track per drive */
    UINT8   keyboard_flags3;       /* 0x496: Keyboard flags 3 */
    UINT8   keyboard_flags4;       /* 0x497: Keyboard LED flags */
    UINT32  usr_wait_ptr;          /* 0x498: User wait complete flag ptr */
    UINT32  usr_wait_count;        /* 0x49C: User wait count in microsecs */
    UINT8   rtc_wait_flag;         /* 0x4A0: RTC wait flag */
    UINT8   lana_dma_chan;         /* 0x4A1: LAN A DMA channel flags */
    UINT8   hd_interrupt_flag;     /* 0x4A2: Hard disk interrupt flag */
    UINT8   network_rom_data;      /* 0x4A3: Network ROM data area */
    UINT8   reserved_04A4[2];      /* 0x4A4: Reserved */
    UINT32  video_save_ptr;        /* 0x4A8: Video Save Pointer Table */
    UINT8   reserved_04AC[68];    /* 0x4AC: Reserved to 0x4F0 */
    UINT8   print_screen_status;   /* 0x500: Print screen status byte */
} BIOS_DATA_AREA;

/* Equipment flags bits */
#define BDA_EQUIP_FLOPPY_BOOT    (1 << 0)
#define BDA_EQUIP_MATH_COPROC    (1 << 1)
#define BDA_EQUIP_PS2_MOUSE      (1 << 2)
#define BDA_EQUIP_VIDEO_MASK     (0x03 << 4)
#define BDA_EQUIP_VIDEO_80X25C   (0x02 << 4)
#define BDA_EQUIP_VIDEO_80X25M   (0x03 << 4)
#define BDA_EQUIP_DRIVES_MASK    (0x03 << 6)
#define BDA_EQUIP_DRIVE_0        (0x00 << 6)
#define BDA_EQUIP_DRIVE_1        (0x01 << 6)
#define BDA_EQUIP_DRIVE_2        (0x02 << 6)
#define BDA_EQUIP_DRIVE_3        (0x03 << 6)
#define BDA_EQUIP_COM1           (1 << 9)
#define BDA_EQUIP_COM2           (1 << 10)
#define BDA_EQUIP_LPT1           (1 << 14)
#define BDA_EQUIP_LPT2           (1 << 15)

/*-----------------------------------------------------------------------------
 * Extended BIOS Data Area (EBDA) - at 0x9000:0x0000 by default
 *---------------------------------------------------------------------------*/
#define EBDA_SEGMENT     0x9000
#define EBDA_SIZE_KB     4

typedef struct RB_PACKED {
    UINT8   size_kb;               /* 0x00: EBDA size in KB */
    UINT8   reserved_01[0x0F];    /* 0x01: Reserved */
    UINT8   hd0_info[0x10];       /* 0x10: Hard disk 0 info */
    UINT8   hd1_info[0x10];       /* 0x20: Hard disk 1 info */
    UINT8   reserved_30[0x40];   /* 0x30: Reserved */
    /* Boot drive geometry info (RetroBoot extension) */
    UINT32  rb_magic;              /* 0x70: RetroBoot magic = 0x52423030 */
    UINT8   rb_boot_drive;         /* 0x74: Boot drive number */
    UINT8   rb_drive_count;        /* 0x75: Total drive count */
    UINT16  rb_version;            /* 0x76: RetroBoot version */
    UINT32  rb_e820_table_addr;    /* 0x78: Physical address of E820 table */
    UINT16  rb_e820_count;         /* 0x7C: Number of E820 entries */
    UINT32  rb_disk_cache_addr;    /* 0x7E: Physical addr of disk cache */
    UINT32  rb_disk_cache_sectors; /* 0x82: Number of cached sectors */
    UINT8   rb_flags;              /* 0x86: Flags */
    UINT8   rb_reserved[0x79];    /* 0x87: Reserved to end of EBDA */
} EBDA_STRUCTURE;

#define EBDA_MAGIC  0x52423030  /* "RB00" */
#define EBDA_FLAG_A20_ENABLED    (1 << 0)
#define EBDA_FLAG_PIC_REMAPPED   (1 << 1)
#define EBDA_FLAG_PIT_INIT       (1 << 2)
#define EBDA_FLAG_HAS_DISK_CACHE (1 << 3)

/*-----------------------------------------------------------------------------
 * INT 15h E820 Memory Map
 *---------------------------------------------------------------------------*/
#define E820_MAX_ENTRIES  128

typedef struct RB_PACKED {
    UINT64  base;          /* Base address */
    UINT64  length;        /* Length in bytes */
    UINT32  type;          /* Memory type */
    UINT32  extended;      /* Extended attributes (ACPI 3.0+) */
} E820_ENTRY;

/* E820 memory types */
#define E820_TYPE_RAM           1   /* Usable conventional memory */
#define E820_TYPE_RESERVED      2   /* Reserved / not available */
#define E820_TYPE_ACPI_DATA     3   /* ACPI reclaimable memory */
#define E820_TYPE_ACPI_NVS      4   /* ACPI non-volatile storage */
#define E820_TYPE_BAD_MEMORY    5   /* Bad memory */
#define E820_TYPE_PMEM          7   /* Persistent memory (NVDIMM) */

/* Extended attribute bits (ACPI 3.0+) */
#define E820_ATTR_ENABLED    (1 << 0)
#define E820_ATTR_NVS        (1 << 1)

/*-----------------------------------------------------------------------------
 * Interrupt Vector Table (IVT) - at 0x0000:0x0000 (4 bytes * 256 = 1KB)
 *---------------------------------------------------------------------------*/
typedef struct RB_PACKED {
    UINT16  offset;        /* IP */
    UINT16  segment;       /* CS */
} IVT_ENTRY;

/* BIOS interrupt numbers */
#define INT_VIDEO            0x10
#define INT_EQUIPMENT        0x11
#define INT_MEMORY_SIZE      0x12
#define INT_DISK             0x13
#define INT_SERIAL           0x14
#define INT_SYSTEM           0x15
#define INT_KEYBOARD         0x16
#define INT_PARALLEL         0x17
#define INT_BASIC            0x18
#define INT_BOOT             0x19
#define INT_TIME_OF_DAY      0x1A
#define INT_KEYBOARD_BREAK   0x1B
#define INT_TIMER_TICK       0x1C
#define INT_VIDEO_PARAMS     0x1D
#define INT_DISK_PARAMS      0x1E
#define INT_DISK_PARAMS2     0x1F

/* IRQ → INT mapping (after PIC remap) */
#define IRQ0_INT  0x08   /* Timer */
#define IRQ1_INT  0x09   /* Keyboard */
#define IRQ2_INT  0x0A   /* Cascade */
#define IRQ3_INT  0x0B   /* COM2 */
#define IRQ4_INT  0x0C   /* COM1 */
#define IRQ5_INT  0x0D   /* LPT2 */
#define IRQ6_INT  0x0E   /* Floppy */
#define IRQ7_INT  0x0F   /* LPT1 */
#define IRQ8_INT  0x70   /* RTC */
#define IRQ9_INT  0x71   /* IRQ2 redirect */
#define IRQ10_INT 0x72
#define IRQ11_INT 0x73
#define IRQ12_INT 0x74   /* PS/2 Mouse */
#define IRQ13_INT 0x75   /* FPU */
#define IRQ14_INT 0x76   /* IDE primary */
#define IRQ15_INT 0x77   /* IDE secondary */

/*-----------------------------------------------------------------------------
 * x86 Descriptor Structures
 *---------------------------------------------------------------------------*/

/* GDT Entry (8 bytes) */
typedef struct RB_PACKED {
    UINT16  limit_low;     /* Limit bits 0-15 */
    UINT16  base_low;      /* Base bits 0-15 */
    UINT8   base_mid;      /* Base bits 16-23 */
    UINT8   access;        /* Access flags */
    UINT8   granularity;   /* Granularity + limit bits 16-19 */
    UINT8   base_high;     /* Base bits 24-31 */
} GDT_ENTRY;

/* Access byte bits */
#define GDT_PRESENT    (1 << 7)
#define GDT_DPL0       (0 << 5)
#define GDT_DPL3       (3 << 5)
#define GDT_SYSTEM     (1 << 4)
#define GDT_EXEC       (1 << 3)
#define GDT_DC         (1 << 2)
#define GDT_RW         (1 << 1)
#define GDT_ACCESSED   (1 << 0)

/* Granularity byte bits */
#define GDT_GRAN_4K    (1 << 7)
#define GDT_GRAN_32BIT (1 << 6)
#define GDT_GRAN_64BIT (1 << 5)

/* GDT Pointer */
typedef struct RB_PACKED {
    UINT16  limit;
    UINT32  base;
} GDT_POINTER32;

typedef struct RB_PACKED {
    UINT16  limit;
    UINT64  base;
} GDT_POINTER64;

/* IDT Entry (8 bytes, real mode compatible) */
typedef struct RB_PACKED {
    UINT16  offset_low;
    UINT16  selector;
    UINT8   zero;
    UINT8   type_attr;
    UINT16  offset_high;
} IDT_ENTRY32;

/* IDT Pointer */
typedef struct RB_PACKED {
    UINT16  limit;
    UINT32  base;
} IDT_POINTER;

/*-----------------------------------------------------------------------------
 * ACPI Structures
 *---------------------------------------------------------------------------*/

/* RSDP (Root System Description Pointer) */
typedef struct RB_PACKED {
    UINT8   signature[8];    /* "RSD PTR " */
    UINT8   checksum;
    UINT8   oem_id[6];
    UINT8   revision;
    UINT32  rsdt_address;
    /* ACPI 2.0+ extension: */
    UINT32  length;
    UINT64  xsdt_address;
    UINT8   extended_checksum;
    UINT8   reserved[3];
} ACPI_RSDP;

/* Generic ACPI Table Header */
typedef struct RB_PACKED {
    UINT8   signature[4];
    UINT32  length;
    UINT8   revision;
    UINT8   checksum;
    UINT8   oem_id[6];
    UINT8   oem_table_id[8];
    UINT32  oem_revision;
    UINT8   creator_id[4];
    UINT32  creator_revision;
} ACPI_TABLE_HEADER;

/* MADT (Multiple APIC Description Table) */
typedef struct RB_PACKED {
    ACPI_TABLE_HEADER header;
    UINT32  local_apic_addr;
    UINT32  flags;
} ACPI_MADT;

/*-----------------------------------------------------------------------------
 * PIC (Programmable Interrupt Controller) Ports
 *---------------------------------------------------------------------------*/
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1
#define PIC_EOI     0x20
#define PIC_ICW1    0x11  /* Init, cascade, ICW4 needed */
#define PIC_ICW4    0x01  /* 8086 mode */
#define PIC1_IRQ_BASE 0x08
#define PIC2_IRQ_BASE 0x70

/*-----------------------------------------------------------------------------
 * PIT (Programmable Interval Timer) Ports
 *---------------------------------------------------------------------------*/
#define PIT_CH0_DATA   0x40
#define PIT_CH1_DATA   0x41
#define PIT_CH2_DATA   0x42
#define PIT_CMD        0x43
#define PIT_FREQ       1193182  /* Hz */
#define PIT_HZ_18_2    (PIT_FREQ / 18)
#define PIT_CMD_CHAN0  (0x00)
#define PIT_CMD_LOBYTE (0x30)
#define PIT_CMD_MODE3  (0x06)

/*-----------------------------------------------------------------------------
 * A20 Gate Ports
 *---------------------------------------------------------------------------*/
#define A20_KBC_CMD_PORT  0x64
#define A20_KBC_DATA_PORT 0x60
#define A20_FAST_PORT     0x92

/*-----------------------------------------------------------------------------
 * Legacy CMOS / RTC
 *---------------------------------------------------------------------------*/
#define CMOS_CMD    0x70
#define CMOS_DATA   0x71
#define CMOS_SEC    0x00
#define CMOS_MIN    0x02
#define CMOS_HOUR   0x04
#define CMOS_DAY    0x07
#define CMOS_MONTH  0x08
#define CMOS_YEAR   0x09
#define CMOS_STATB  0x0B

/*-----------------------------------------------------------------------------
 * Physical Memory Layout Constants
 *---------------------------------------------------------------------------*/
#define PHYS_IVT_BASE       0x00000000  /* Interrupt Vector Table (1KB) */
#define PHYS_BDA_BASE       0x00000400  /* BIOS Data Area (256B) */
#define PHYS_CONV_START     0x00000500  /* Start of conv. memory */
#define PHYS_EBDA_BASE      0x00090000  /* Extended BIOS Data Area (4KB) */
#define PHYS_VIDEO_ROM      0x000A0000  /* Video RAM / adapter ROM */
#define PHYS_VIDEO_TEXT     0x000B8000  /* VGA text buffer */
#define PHYS_BIOS_ROM_EXT   0x000C0000  /* Adapter ROM area */
#define PHYS_BIOS_ROM       0x000F0000  /* System BIOS ROM */
#define PHYS_HIGH_MEMORY    0x00100000  /* 1MB+: Extended memory */
#define PHYS_MBR_LOAD       0x00007C00  /* MBR load address */
#define PHYS_HANDLERS_BASE  0x00007E00  /* BIOS handlers 16-bit code */
#define PHYS_TRAMPOLINE     0x00008000  /* Mode-switch trampoline */
#define PHYS_DISK_CACHE     0x00010000  /* 16-bit disk cache area */
#define PHYS_GDT32          0x00008200  /* 32-bit GDT */
#define PHYS_E820_TABLE     0x00008300  /* E820 table */

/* Memory sizes */
#define CONV_MEM_KB         639         /* 639KB conventional memory */
#define MBR_SIZE            512
#define HANDLER_CODE_MAX    (PHYS_TRAMPOLINE - PHYS_HANDLERS_BASE)

/* Segment:Offset macros */
#define PHYS_TO_SEG(addr)   ((UINT16)(((UINT32)(addr)) >> 4))
#define PHYS_TO_OFF(addr)   ((UINT16)(((UINT32)(addr)) & 0x000F))
#define SEG_OFF_TO_PHYS(s,o) (((UINT32)(s) << 4) + (UINT32)(o))

/*-----------------------------------------------------------------------------
 * Boot Drive Numbers
 *---------------------------------------------------------------------------*/
#define DRIVE_FLOPPY_A  0x00
#define DRIVE_FLOPPY_B  0x01
#define DRIVE_HDD_0     0x80
#define DRIVE_HDD_1     0x81
#define DRIVE_HDD_2     0x82
#define DRIVE_HDD_3     0x83

/*-----------------------------------------------------------------------------
 * INT 13h Function Codes
 *---------------------------------------------------------------------------*/
#define INT13_RESET              0x00
#define INT13_GET_STATUS         0x01
#define INT13_READ_SECTORS       0x02
#define INT13_WRITE_SECTORS      0x03
#define INT13_VERIFY_SECTORS     0x04
#define INT13_FORMAT_TRACK       0x05
#define INT13_GET_DISK_TYPE      0x15
#define INT13_DETECT_CHANGE      0x16
#define INT13_EXT_CHECK          0x41
#define INT13_EXT_READ           0x42
#define INT13_EXT_WRITE          0x43
#define INT13_EXT_VERIFY         0x44
#define INT13_EXT_GET_PARAMS     0x48
#define INT13_GET_GEOMETRY       0x08
#define INT13_INIT_CTRL          0x09

/* INT 13h status codes */
#define INT13_OK                 0x00
#define INT13_ERR_INVALID_CMD    0x01
#define INT13_ERR_NOT_READY      0x03
#define INT13_ERR_SECTOR_NF      0x04
#define INT13_ERR_RESET_FAIL     0x05
#define INT13_ERR_MEDIA_CHANGED  0x06
#define INT13_ERR_DMA_OVER       0x09
#define INT13_ERR_BAD_SECTOR     0x0A
#define INT13_ERR_UNCORRECTABLE  0x11
#define INT13_ERR_NO_MEDIA       0x31
#define INT13_ERR_INVALID_CHANGE 0x32
#define INT13_ERR_NO_MEDIA2      0x80

/* INT 13h EDD (Extended Disk Drive) packet */
typedef struct RB_PACKED {
    UINT8   size;           /* Size of packet (0x10 or 0x18) */
    UINT8   reserved;       /* Must be 0 */
    UINT16  blocks;         /* Number of blocks to transfer */
    UINT16  buffer_offset;  /* Transfer buffer offset */
    UINT16  buffer_segment; /* Transfer buffer segment */
    UINT64  lba;            /* Starting LBA */
    /* EDD 3.0 extension: */
    UINT64  flat_buffer;    /* 64-bit flat buffer address */
} EDD_DISK_PACKET;

/* INT 13h AH=48h Get Drive Parameters result */
typedef struct RB_PACKED {
    UINT16  size;           /* Size of buffer */
    UINT16  flags;          /* Information flags */
    UINT32  cylinders;      /* Physical cylinders */
    UINT32  heads;          /* Physical heads */
    UINT32  sectors;        /* Physical sectors per track */
    UINT64  total_sectors;  /* Total sectors */
    UINT16  bytes_per_sector;
    UINT32  edd_ptr;        /* Pointer to EDD config params */
} EDD_DRIVE_PARAMS;

/*-----------------------------------------------------------------------------
 * INT 10h Video Services Function Codes
 *---------------------------------------------------------------------------*/
#define INT10_SET_MODE           0x00
#define INT10_SET_CURSOR_SHAPE   0x01
#define INT10_SET_CURSOR_POS     0x02
#define INT10_GET_CURSOR_POS     0x03
#define INT10_SCROLL_UP          0x06
#define INT10_SCROLL_DOWN        0x07
#define INT10_READ_CHAR_ATTR     0x08
#define INT10_WRITE_CHAR_ATTR    0x09
#define INT10_WRITE_CHAR         0x0A
#define INT10_SET_PALETTE        0x0B
#define INT10_TELETYPE_OUTPUT    0x0E
#define INT10_GET_VIDEO_MODE     0x0F
#define INT10_SET_DAC_REGS       0x10
#define INT10_CHAR_GENERATOR     0x11
#define INT10_ALT_SELECT         0x12
#define INT10_WRITE_STRING       0x13
#define INT10_VESA_CONTROL       0x4F

/* VGA color text attributes */
#define ATTR_FG_BLACK   0x00
#define ATTR_FG_BLUE    0x01
#define ATTR_FG_GREEN   0x02
#define ATTR_FG_CYAN    0x03
#define ATTR_FG_RED     0x04
#define ATTR_FG_MAGENTA 0x05
#define ATTR_FG_BROWN   0x06
#define ATTR_FG_LGRAY   0x07
#define ATTR_FG_DGRAY   0x08
#define ATTR_FG_LBLUE   0x09
#define ATTR_FG_LGREEN  0x0A
#define ATTR_FG_LCYAN   0x0B
#define ATTR_FG_LRED    0x0C
#define ATTR_FG_LMAG    0x0D
#define ATTR_FG_YELLOW  0x0E
#define ATTR_FG_WHITE   0x0F
#define ATTR_BG_BLACK   0x00
#define ATTR_BG_BLUE    0x10
#define ATTR_NORMAL     (ATTR_FG_LGRAY | ATTR_BG_BLACK)
#define ATTR_BRIGHT     (ATTR_FG_WHITE | ATTR_BG_BLACK)
#define ATTR_HIGHLIGHT  (ATTR_FG_YELLOW | ATTR_BG_BLACK)
#define ATTR_ERROR      (ATTR_FG_WHITE | (0x04 << 4))

/* VGA text mode dimensions */
#define VGA_COLS  80
#define VGA_ROWS  25

/*-----------------------------------------------------------------------------
 * RetroBoot Internal State
 *---------------------------------------------------------------------------*/
#define RB_MAX_DRIVES         8
#define RB_DISK_CACHE_SECTORS 256  /* 128KB cache */
#define RB_VERSION_MAJOR      1
#define RB_VERSION_MINOR      0
#define RB_VERSION_PATCH      0
#define RB_MAGIC_SIGNATURE    0x52455442  /* "RETB" */

/* Disk info (one per physical disk) */
typedef struct {
    EFI_BLOCK_IO_PROTOCOL *BlockIo;
    EFI_HANDLE             Handle;
    UINT8                  DriveNumber;    /* BIOS drive number */
    UINT64                 TotalSectors;
    UINT32                 SectorSize;
    UINT32                 Cylinders;
    UINT8                  Heads;
    UINT8                  SectorsPerTrack;
    BOOLEAN                HasEBR;         /* Has EFI Block IO v2 */
    BOOLEAN                IsBootable;
    UINT8                  CachedSectorData[RB_DISK_CACHE_SECTORS * 512];
    UINT64                 CacheStartLBA;
    UINT32                 CachedSectorCount;
} RB_DISK_INFO;

/* RetroBoot global context */
typedef struct {
    UINT32                         Magic;
    EFI_SYSTEM_TABLE              *ST;
    EFI_BOOT_SERVICES             *BS;
    EFI_RUNTIME_SERVICES          *RS;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                     ImageHandle;
    EFI_GRAPHICS_OUTPUT_PROTOCOL  *GOP;
    UINT64                         FrameBufferBase;
    UINT32                         FrameBufferSize;
    UINT32                         ScreenWidth;
    UINT32                         ScreenHeight;
    UINT32                         PixelsPerScanLine;
    EFI_GRAPHICS_PIXEL_FORMAT      PixelFormat;
    RB_DISK_INFO                   Disks[RB_MAX_DRIVES];
    UINT32                         DiskCount;
    UINT32                         BootDriveIndex;
    UINT8                          BootDriveNumber;  /* 0x80 for first HDD */
    E820_ENTRY                     E820Table[E820_MAX_ENTRIES];
    UINT32                         E820Count;
    UINT64                         TotalMemoryBytes;
    UINT64                         ConventionalMemoryBytes;
    EFI_MEMORY_DESCRIPTOR         *MemoryMap;
    UINTN                          MemoryMapSize;
    UINTN                          MemoryMapDescriptorSize;
    UINT32                         MemoryMapDescriptorVersion;
    UINTN                          MemoryMapKey;
    UINT8                          MBRBuffer[512];
    BOOLEAN                        MBRLoaded;
    BOOLEAN                        A20Enabled;
    BOOLEAN                        PICRemapped;
    BOOLEAN                        PITInitialized;
    BOOLEAN                        VerboseMode;
    BOOLEAN                        GraphicsMode;
    UINT32                         TimerTicks;
    ACPI_RSDP                     *RSDP;
    UINT32                         Version;
} RB_CONTEXT;

/*=============================================================================
 * 16-bit BIOS Handler Code Stubs
 * Pre-assembled x86 real-mode machine code embedded as byte arrays.
 * These are installed at PHYS_HANDLERS_BASE and pointed to by the IVT.
 *=============================================================================*/

/*
 * Each stub does the minimal BIOS service and returns via IRET.
 * For the full emulation, disk I/O is handled by a pre-cached memory
 * buffer filled before ExitBootServices().
 * For keyboard and time, we rely on the BIOS data area updated by our
 * timer IRQ handler.
 */

/* INT 08h - IRQ0 Timer Handler (18.2 Hz tick) */
/* Increments BDA timer ticks, checks midnight, signals PIC EOI */
extern const UINT8 g_isr_timer[]; 
extern const UINT32 g_isr_timer_size;

/* INT 09h - IRQ1 Keyboard Handler */
extern const UINT8 g_isr_keyboard[];
extern const UINT32 g_isr_keyboard_size;

/* INT 10h - Video Services */
extern const UINT8 g_isr_video[];
extern const UINT32 g_isr_video_size;

/* INT 11h - Equipment Check */
extern const UINT8 g_isr_equipment[];
extern const UINT32 g_isr_equipment_size;

/* INT 12h - Memory Size */
extern const UINT8 g_isr_memsize[];
extern const UINT32 g_isr_memsize_size;

/* INT 13h - Disk Services */
extern const UINT8 g_isr_disk[];
extern const UINT32 g_isr_disk_size;

/* INT 15h - System Services (E820 etc.) */
extern const UINT8 g_isr_system[];
extern const UINT32 g_isr_system_size;

/* INT 16h - Keyboard Services */
extern const UINT8 g_isr_kbd[];
extern const UINT32 g_isr_kbd_size;

/* INT 1Ah - Time of Day */
extern const UINT8 g_isr_time[];
extern const UINT32 g_isr_time_size;

/* INT 1Ch - Timer Tick (user hook, initially IRET) */
extern const UINT8 g_isr_tick_user[];
extern const UINT32 g_isr_tick_user_size;

/* Generic IRET stub (for unhandled interrupts) */
extern const UINT8 g_isr_dummy[];
extern const UINT32 g_isr_dummy_size;

/* Mode-switch trampoline: Long Mode → Protected Mode → Real Mode → MBR */
extern const UINT8 g_trampoline[];
extern const UINT32 g_trampoline_size;

/*=============================================================================
 * Function Prototypes
 *=============================================================================*/

/* Entry point */
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

/* Initialization */
EFI_STATUS RB_Init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST);
EFI_STATUS RB_InitGraphics(VOID);
EFI_STATUS RB_InitDisks(VOID);
EFI_STATUS RB_BuildMemoryMap(VOID);
EFI_STATUS RB_LoadMBR(VOID);

/* BIOS environment setup */
VOID RB_SetupIVT(VOID);
VOID RB_SetupBDA(VOID);
VOID RB_SetupEBDA(VOID);
VOID RB_InstallHandlers(VOID);
VOID RB_SetupGDT32(VOID);

/* Hardware setup */
VOID RB_RemapPIC(VOID);
VOID RB_InitPIT(VOID);
VOID RB_EnableA20(VOID);
BOOLEAN RB_TestA20(VOID);

/* Boot sequence */
EFI_STATUS RB_Boot(VOID);
VOID RB_EnterRealMode(VOID) RB_NORETURN;

/* Display / UI */
VOID RB_ClearScreen(VOID);
VOID RB_PrintBanner(VOID);
VOID RB_PrintStatus(const CHAR16 *msg, BOOLEAN ok);
VOID RB_Print(const CHAR16 *str);
VOID RB_PrintHex64(UINT64 val);
VOID RB_PrintHex32(UINT32 val);
VOID RB_PrintDec(UINT64 val);
VOID RB_PrintNewline(VOID);
VOID RB_DrawProgressBar(UINT32 percent);

/* Utility */
VOID     *RB_Memset(VOID *dst, UINT8 val, UINTN n);
VOID     *RB_Memcpy(VOID *dst, const VOID *src, UINTN n);
INT32     RB_Memcmp(const VOID *a, const VOID *b, UINTN n);
UINTN     RB_Strlen16(const CHAR16 *s);
VOID      RB_Strlcpy16(CHAR16 *dst, const CHAR16 *src, UINTN max);
UINT32    RB_CRC32(const UINT8 *data, UINTN len);
VOID      RB_IoDelay(VOID);
VOID      RB_Stall(UINTN microseconds);
UINT8     RB_InB(UINT16 port);
VOID      RB_OutB(UINT16 port, UINT8 val);
UINT16    RB_InW(UINT16 port);
VOID      RB_OutW(UINT16 port, UINT16 val);

/* Disk helpers */
EFI_STATUS RB_ReadSectors(UINT32 driveIdx, UINT64 lba,
                           UINT32 count, VOID *buffer);
EFI_STATUS RB_DiskCacheLoad(UINT32 driveIdx, UINT64 startLba,
                             UINT32 sectorCount);
VOID       RB_LBAToCHS(UINT64 lba, UINT32 heads, UINT32 spt,
                        UINT32 *c, UINT8 *h, UINT8 *s);

/* Memory map helpers */
VOID RB_AddE820Entry(UINT64 base, UINT64 len, UINT32 type);
VOID RB_SortE820Table(VOID);
VOID RB_MergeE820Table(VOID);
VOID RB_DumpE820Table(VOID);

/* Global context */
extern RB_CONTEXT g_rb;

/*-----------------------------------------------------------------------------
 * Convenience Macros
 *---------------------------------------------------------------------------*/
#define RB_ASSERT(cond) do { if (RB_UNLIKELY(!(cond))) { \
    RB_Print(L"ASSERT FAILED: " L#cond L"\r\n"); \
    for(;;) __asm__ __volatile__("hlt"); } } while(0)

#define RB_CHECK_STATUS(s) do { if (EFI_ERROR(s)) { \
    RB_Print(L"EFI Error: "); RB_PrintHex64(s); RB_PrintNewline(); \
    return (s); } } while(0)

#define RB_PAGE_SIZE  4096ULL
#define RB_PAGE_ROUND_UP(x)   (((x) + RB_PAGE_SIZE - 1) & ~(RB_PAGE_SIZE - 1))
#define RB_PAGE_ROUND_DOWN(x) ((x) & ~(RB_PAGE_SIZE - 1))
#define RB_PAGES(bytes) (RB_PAGE_ROUND_UP(bytes) / RB_PAGE_SIZE)

#define RB_BCD_TO_BIN(v) (((v) >> 4) * 10 + ((v) & 0x0F))
#define RB_BIN_TO_BCD(v) ((((v) / 10) << 4) | ((v) % 10))

#define RB_MIN(a,b) ((a) < (b) ? (a) : (b))
#define RB_MAX(a,b) ((a) > (b) ? (a) : (b))
#define RB_ARRAY_LEN(a) (sizeof(a)/sizeof((a)[0]))

/* Write a byte to physical memory (for low-memory setup) */
#define PHYS_WRITE8(addr, val) \
    (*((volatile UINT8*)(UINTN)(addr)) = (val))
#define PHYS_WRITE16(addr, val) \
    (*((volatile UINT16*)(UINTN)(addr)) = (val))
#define PHYS_WRITE32(addr, val) \
    (*((volatile UINT32*)(UINTN)(addr)) = (val))
#define PHYS_READ8(addr) \
    (*((volatile UINT8*)(UINTN)(addr)))
#define PHYS_READ16(addr) \
    (*((volatile UINT16*)(UINTN)(addr)))
#define PHYS_READ32(addr) \
    (*((volatile UINT32*)(UINTN)(addr)))

/* Pointer to BDA */
#define BDA ((volatile BIOS_DATA_AREA*)(UINTN)PHYS_BDA_BASE)

/* Pointer to IVT */
#define IVT ((volatile IVT_ENTRY*)(UINTN)PHYS_IVT_BASE)

#endif /* RETROBOOT_H */