#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>

// Vendor GUID UsbConfigDxe uses for its port-role variables.
static EFI_GUID mUsbCfgVendorGuid = {
  0x882F8C2B, 0x9646, 0x435F, { 0x8D, 0xE5, 0xF2, 0x08, 0xFF, 0x80, 0xC1, 0xBD }
};

#define ROLE_HOST  1

static CHAR16 *mPortNames[] = { L"UsbConfigPrimaryPort", L"UsbConfigSecondaryPort" };

static
VOID
ShowRole (
  IN CHAR16  *Name
  )
{
  UINT8       Value;
  UINTN       Size;
  EFI_STATUS  Status;

  Size   = sizeof (Value);
  Value  = 0;
  Status = gST->RuntimeServices->GetVariable (Name, &mUsbCfgVendorGuid, NULL, &Size, &Value);
  if (EFI_ERROR (Status)) {
    Print (L"  %s : NOT SET (%r)\n", Name, Status);
  } else {
    Print (L"  %s : %d\n", Name, Value);
  }
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT8       Value;
  EFI_STATUS  Status;

  Print (L"\n=== UsbRoleCfg: force USB HOST role ===\n\n");
  Print (L"Before:\n");
  ShowRole (mPortNames[0]);
  ShowRole (mPortNames[1]);
  Print (L"\n");

  Value  = ROLE_HOST;
  Status = gST->RuntimeServices->SetVariable (
                                  mPortNames[0],
                                  &mUsbCfgVendorGuid,
                                  EFI_VARIABLE_NON_VOLATILE |
                                  EFI_VARIABLE_BOOTSERVICE_ACCESS |
                                  EFI_VARIABLE_RUNTIME_ACCESS,
                                  sizeof (Value),
                                  &Value
                                  );
  Print (L"SetVariable(%s = %d) : %r\n\n", mPortNames[0], Value, Status);

  Print (L"After:\n");
  ShowRole (mPortNames[0]);
  ShowRole (mPortNames[1]);

  Print (L"\nReboot, then run UsbKbdTest.\n");
  gBS->Stall (8 * 1000 * 1000);

  return EFI_SUCCESS;
}
