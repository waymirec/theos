#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include "types.h"

typedef struct {
    void *BaseAddress;
    size_t BufferSize;
    unsigned int HorizontalResolution;
    unsigned int VerticalResolution;
    unsigned int PixelsPerScanLine;
} Framebuffer;
Framebuffer g_framebuffer;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
    unsigned char magic[2];
    unsigned char mode;
    unsigned char charSize;
} PSF1_HEADER;

typedef struct {
    PSF1_HEADER *header;
    void *glyphBuffer;
} PSF1_FONT;

typedef struct {
    EFI_MEMORY_DESCRIPTOR *memoryMap;
    UINTN memoryMapSize;
    UINTN memoryMapKey;
    UINTN memoryMapDescriptorSize;
    UINT32 memoryMapDescriptorVersion;
} MemoryInfo;
MemoryInfo g_memoryInfo;

typedef struct {
    Framebuffer *framebuffer;
    PSF1_FONT *font;
    MemoryInfo *memoryInfo;
    void *rootSystemDescriptionPointer;
} BootInfo;

EFI_FILE* LoadFile(EFI_FILE *, CHAR16 *, EFI_HANDLE, EFI_SYSTEM_TABLE *);
PSF1_FONT* LoadPSF1Font(EFI_FILE *, CHAR16 *, EFI_HANDLE, EFI_SYSTEM_TABLE *);
int VerifyKernelFormat(Elf64_Ehdr *);
int memcmp(const void *, const void *, size_t);
Framebuffer* InitializeGop();
MemoryInfo* GetMemoryInfo(EFI_SYSTEM_TABLE *);
void* GetRootSystemDescriptor(EFI_SYSTEM_TABLE *);
UINTN strcmp(CHAR8 *, CHAR8 *, UINTN);

EFI_STATUS efi_main (EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
    InitializeLib(imageHandle, systemTable);

    EFI_FILE *kernel = LoadFile(NULL, L"kernel.elf", imageHandle, systemTable);
    if (kernel == NULL) {
        Print(L"ERROR: Unable to load kernel.\n\r");
        return EFI_LOAD_ERROR;
    }
    Print(L"Kernel loaded successfully.\n\r");

    Elf64_Ehdr header;
    {
        UINTN FileInfoSize;
        EFI_FILE_INFO *FileInfo;
        kernel->GetInfo(kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
        systemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void **) &FileInfo);
        kernel->GetInfo(kernel, &gEfiFileInfoGuid, &FileInfoSize, (void **) &FileInfo);
        UINTN size = sizeof(header);
        kernel->Read(kernel, &size, &header);
    }

    if (!VerifyKernelFormat(&header))
    {
        Print(L"ERROR: Kernel header is malformed.\n\r");
        return EFI_LOAD_ERROR;
    }
    Print(L"Kernel header verified.\n\r");

    UINTN headerTableSize = header.e_phnum * header.e_phentsize;
    Elf64_Phdr *pHeaders;
    {
        kernel->SetPosition(kernel, header.e_phoff);
        systemTable->BootServices->AllocatePool(EfiLoaderData, headerTableSize, (void**)&pHeaders);
        kernel->Read(kernel, &headerTableSize, pHeaders);
    }

    for (Elf64_Phdr *pHeader = pHeaders; (char *)pHeader < (char *)pHeaders + headerTableSize; pHeader = (Elf64_Phdr *)((char *)pHeader + header.e_phentsize))
    {
        switch (pHeader->p_type)
        {
            case PT_LOAD:
            {
                uint64_t pages = (pHeader->p_memsz + 0x1000 - 1) / 0x1000;
                Elf64_Addr segment = pHeader->p_paddr;
                systemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);
                kernel->SetPosition(kernel, pHeader->p_offset);
                UINTN size = pHeader->p_filesz;
                kernel->Read(kernel, &size, (void *)segment);
                break;
            }
        }
    }

    Print(L"Kernel Loaded.\n\r");

    PSF1_FONT *font = LoadPSF1Font(NULL, L"zap-light16.psf", imageHandle, systemTable);
    if (font == NULL)
    {
        Print(L"ERROR: font is invalid or missing.\n\r");
        return EFI_LOAD_ERROR;
    }

    Print(L"Loaded Font (%d).\n\r", font->header->charSize);

    Framebuffer *framebuffer = InitializeGop();

    MemoryInfo *memoryInfo = GetMemoryInfo(systemTable);
    void *rsdp = GetRootSystemDescriptor(systemTable);

    BootInfo bootInfo;
    bootInfo.framebuffer = framebuffer;
    bootInfo.font = font;
    bootInfo.memoryInfo = memoryInfo;
    bootInfo.rootSystemDescriptionPointer = rsdp;

    systemTable->BootServices->ExitBootServices(imageHandle, memoryInfo->memoryMapKey);

    systemTable->RuntimeServices->SetVirtualAddressMap(memoryInfo->memoryMapSize,
                                                       memoryInfo->memoryMapDescriptorSize,
                                                       memoryInfo->memoryMapDescriptorVersion,
                                                       memoryInfo->memoryMap);

    void (*KernelStart)(BootInfo*) = (__attribute__((sysv_abi)) void (*)(BootInfo*) ) header.e_entry;
    KernelStart(&bootInfo);

	return EFI_SUCCESS; // Exit the UEFI application
}

EFI_FILE* LoadFile(EFI_FILE* directory, CHAR16* path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    EFI_FILE* loadedFile;

    EFI_LOADED_IMAGE_PROTOCOL* loadedImage;
    systemTable->BootServices->HandleProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, (void **)&loadedImage);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem;
    systemTable->BootServices->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&fileSystem);

    if (directory == NULL)
    {
        fileSystem->OpenVolume(fileSystem, &directory);
    }

    EFI_STATUS fileLoadStatus = directory->Open(directory, &loadedFile, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (fileLoadStatus != EFI_SUCCESS)
    {
        return NULL;
    }

    return loadedFile;
}

PSF1_FONT* LoadPSF1Font(EFI_FILE* directory, CHAR16* path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    EFI_FILE *font = LoadFile(directory, path, imageHandle, systemTable);
    if (font == NULL) return NULL;

    PSF1_HEADER *fontHeader;
    UINTN fontHeaderSize = sizeof(PSF1_HEADER);
    systemTable->BootServices->AllocatePool(EfiLoaderData, fontHeaderSize, (void **)&fontHeader);
    font->Read(font, &fontHeaderSize, fontHeader);

    if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1)
    {
        return NULL;
    }

    UINTN glyphCount = fontHeader->mode == 1 ? 512 : 256;
    UINTN glyphBufferSize = fontHeader->charSize * glyphCount;
    void *glyphBuffer;
    {
        font->SetPosition(font, fontHeaderSize);
        systemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void **)&glyphBuffer);
        font->Read(font, &glyphBufferSize, glyphBuffer);
    }

    PSF1_FONT *finishedFont;
    systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void **)&finishedFont);
    finishedFont->header = fontHeader;
    finishedFont->glyphBuffer = glyphBuffer;
    return finishedFont;
}

int memcmp(const void *string1, const void *string2, size_t length)
{
    const unsigned char *s1 = string1, *s2 = string2;
    for(size_t i = 0; i < length; i++)
    {
        if (s1[i] < s2[i]) return -1;
        if (s1[i] > s2[i]) return 1;
    }
    return 0;
}

int VerifyKernelFormat(Elf64_Ehdr* header)
{
    int result = (memcmp(&header->e_ident[EI_MAG0], ELFMAG, SELFMAG) == 0);
    result = result && header->e_ident[EI_CLASS] == ELFCLASS64;
    result = result && header->e_ident[EI_DATA] == ELFDATA2LSB;
    result = result && header->e_type == ET_EXEC;
    result = result && header->e_machine == EM_X86_64;
    result = result && header->e_version == EV_CURRENT;
    return result;
}

Framebuffer* InitializeGop()
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status;

    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    if (EFI_ERROR(status))
    {
        Print(L"ERROR: Unable to locate GOP.\n\r");
        return NULL;
    }

    Print(L"GOP successfully located.\n\r");

    Print(L"Framebuffer:\n\r");
    Print(L"-- Base: 0x%x\n\r", g_framebuffer.BaseAddress);
    Print(L"-- Size: %d\n\r", g_framebuffer.BufferSize);
    Print(L"-- Width: %d\n\r", g_framebuffer.HorizontalResolution);
    Print(L"-- Height: %d\n\r", g_framebuffer.VerticalResolution);
    Print(L"-- PPSL: %d\n\r", g_framebuffer.PixelsPerScanLine);

    g_framebuffer.BaseAddress = (void*)gop->Mode->FrameBufferBase;
    g_framebuffer.BufferSize = gop->Mode->FrameBufferSize;
    g_framebuffer.HorizontalResolution = gop->Mode->Info->HorizontalResolution;
    g_framebuffer.VerticalResolution = gop->Mode->Info->VerticalResolution;
    g_framebuffer.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

    return &g_framebuffer;

}

MemoryInfo* GetMemoryInfo(EFI_SYSTEM_TABLE *systemTable)
{
    EFI_MEMORY_DESCRIPTOR  *memoryMap = NULL;
    UINTN memoryMapSize, memoryMapKey, memoryMapDescriptorSize;
    UINT32 memoryMapDescriptorVersion;
    {
        systemTable->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
        systemTable->BootServices->AllocatePool(EfiLoaderData, memoryMapSize, (void **)&memoryMap);
        systemTable->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
    }

    g_memoryInfo.memoryMap = memoryMap;
    g_memoryInfo.memoryMapSize = memoryMapSize;
    g_memoryInfo.memoryMapKey = memoryMapKey;
    g_memoryInfo.memoryMapDescriptorSize = memoryMapDescriptorSize;
    g_memoryInfo.memoryMapDescriptorVersion = memoryMapDescriptorVersion;

    return &g_memoryInfo;
}

void* GetRootSystemDescriptor(EFI_SYSTEM_TABLE *systemTable)
{
    EFI_CONFIGURATION_TABLE *configTable = systemTable->ConfigurationTable;
    EFI_GUID acpi20TableGuid = ACPI_20_TABLE_GUID;

    void *rsdp = NULL;
    for (UINTN i = 0; i < systemTable->NumberOfTableEntries; i++,configTable++) {
        if (CompareGuid(&configTable[i].VendorGuid, &acpi20TableGuid)) {
            if (strcmp((CHAR8 *)"RSD PTR ", (CHAR8 *)configTable->VendorTable, 8)) {
                rsdp = (void *)configTable->VendorTable;
            }
        }
    }
    return rsdp;
}

UINTN strcmp(CHAR8 *a, CHAR8 *b, UINTN length)
{
    for (UINTN i = 0; i < length; i++) {
        if (*a != *b) return 0;
    }
    return 1;
}