#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/UsbIo.h>
#include <Protocol/SimpleTextInEx.h>

#define POLL_INTERVAL_MS  50
#define TEST_DURATION_MS  60000

STATIC
VOID
PrintScanCode (
  IN UINT16  ScanCode
  )
{
  switch (ScanCode) {
    case SCAN_UP:     Print (L"UP");     break;
    case SCAN_DOWN:   Print (L"DOWN");   break;
    case SCAN_LEFT:   Print (L"LEFT");   break;
    case SCAN_RIGHT:  Print (L"RIGHT");  break;
    case SCAN_HOME:   Print (L"HOME");   break;
    case SCAN_END:    Print (L"END");    break;
    case SCAN_INSERT: Print (L"INSERT"); break;
    case SCAN_DELETE: Print (L"DELETE"); break;
    case SCAN_PAGE_UP:   Print (L"PAGE_UP");   break;
    case SCAN_PAGE_DOWN: Print (L"PAGE_DOWN"); break;
    case SCAN_F1:     Print (L"F1");      break;
    case SCAN_F2:     Print (L"F2");      break;
    case SCAN_F3:     Print (L"F3");      break;
    case SCAN_F4:     Print (L"F4");      break;
    case SCAN_F5:     Print (L"F5");      break;
    case SCAN_F6:     Print (L"F6");      break;
    case SCAN_F7:     Print (L"F7");      break;
    case SCAN_F8:     Print (L"F8");      break;
    case SCAN_F9:     Print (L"F9");      break;
    case SCAN_F10:    Print (L"F10");     break;
    case SCAN_F11:    Print (L"F11");     break;
    case SCAN_F12:    Print (L"F12");     break;
    case SCAN_ESC:    Print (L"ESC");     break;
    default:          Print (L"SCAN_0x%04x", ScanCode); break;
  }
}

STATIC
VOID
EnumerateUsbDevices (
  VOID
  )
{
  EFI_STATUS    Status;
  EFI_HANDLE    *Handles;
  UINTN         HandleCount;
  UINTN         Index;

  Handles = NULL;
  Status  = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiUsbIoProtocolGuid,
                   NULL,
                   &HandleCount,
                   &Handles
                   );
  if (EFI_ERROR (Status) || HandleCount == 0) {
    Print (L"USB DEVICES: 0 (LocateHandleBuffer: %r)\n", Status);
    return;
  }

  Print (L"USB DEVICES: %d\n", HandleCount);
  for (Index = 0; Index < HandleCount; Index++) {
    EFI_USB_IO_PROTOCOL     *UsbIo;
    USB_DEVICE_DESCRIPTOR   DevDesc;

    Status = gBS->HandleProtocol (Handles[Index], &gEfiUsbIoProtocolGuid, (VOID **)&UsbIo);
    if (EFI_ERROR (Status)) {
      Print (L"  [%d] HandleProtocol failed: %r\n", Index, Status);
      continue;
    }

    Status = UsbIo->UsbGetDeviceDescriptor (UsbIo, &DevDesc);
    if (EFI_ERROR (Status)) {
      Print (L"  [%d] GetDeviceDescriptor failed: %r\n", Index, Status);
      continue;
    }

    Print (
      L"  [%d] VID=%04x PID=%04x Class=%02x SubClass=%02x Proto=%02x\n",
      Index,
      DevDesc.IdVendor,
      DevDesc.IdProduct,
      DevDesc.DeviceClass,
      DevDesc.DeviceSubClass,
      DevDesc.DeviceProtocol
      );
  }

  FreePool (Handles);

  Handles = NULL;
  Status  = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiSimpleTextInputExProtocolGuid,
                   NULL,
                   &HandleCount,
                   &Handles
                   );
  if (!EFI_ERROR (Status)) {
    // ConSplitter + console-in also expose this protocol; count as a hint only.
    Print (L"SimpleTextInputEx handles: %d\n", HandleCount);
    FreePool (Handles);
  }
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          ElapsedMs = 0;
  UINTN          KeyCount  = 0;

  Print (L"=== UsbKbdTest ===\n\n");

  EnumerateUsbDevices ();

  Print (L"\nKey test for %d s. Press keys; ESC = exit.\n", TEST_DURATION_MS / 1000);

  while (ElapsedMs < TEST_DURATION_MS) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (!EFI_ERROR (Status)) {
      KeyCount++;
      if (Key.ScanCode == SCAN_ESC) {
        Print (L"\nESC pressed - exit.\n");
        break;
      }

      if (Key.ScanCode != 0) {
        Print (L"KEY #%d: ", KeyCount);
        PrintScanCode (Key.ScanCode);
        Print (L"\n");
      } else {
        Print (L"KEY #%d: CHAR '%c' (0x%04x)\n", KeyCount, Key.UnicodeChar, Key.UnicodeChar);
      }

      continue;
    }

    gBS->Stall (POLL_INTERVAL_MS * 1000);
    ElapsedMs += POLL_INTERVAL_MS;
  }

  Print (L"\n=== RESULT: %d key(s) received ===\n", KeyCount);
  if (KeyCount == 0) {
    Print (L"NO KEYBOARD INPUT DETECTED\n");
  } else {
    Print (L"KEYBOARD WORKS\n");
  }

  gBS->Stall (5 * 1000 * 1000);
  return EFI_SUCCESS;
}
