#include <efi.h>
#include <efilib.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;

    InitializeLib(ImageHandle, SystemTable);
    ST = SystemTable;

    EFI_GUID GopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop = NULL;

    Status = uefi_call_wrapper(BS->LocateProtocol, 3, &GopGUID, NULL, (void**)&Gop);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    UINT32 ScreenWidth = Gop->Mode->Info->HorizontalResolution;
    UINT32 ScreenHeight = Gop->Mode->Info->VerticalResolution;
    UINT32 PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;

    UINT32 *Framebuffer = (UINT32 *)Gop->Mode->FrameBufferBase;

    for (UINT32 y = 0; y < ScreenHeight; ++y) {
        for (UINT32 x = 0; x < ScreenWidth; ++x) {
            UINT32 Color = 0xFFFF0000;
            Framebuffer[y * PixelsPerScanLine + x] = Color;
        }
    }

    return Status;
}
