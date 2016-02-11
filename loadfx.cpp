#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0A00

#include <windows.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <string.h>

using namespace std;

int wmain(int argc, wchar_t*argv[])
{
  if (argc != 2) {
    wcout << L"Usage: " << argv[0] << " <path_to_firefox_binary>" << endl;
    return 1;
  }
  // wchar_t *exe = L"C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe";
  wchar_t *exe = argv[1];
  wostringstream oss;
  oss << L"\"" << exe << L"\" -no-remote -profile \"C:\\Users\\dblohm7\\dumps\\profile\"";
  const wstring& cls = oss.str();
  auto cl = make_unique<wchar_t[]>(cls.size() + 1);
  wcscpy(cl.get(), cls.c_str());

  STARTUPINFOEXW siex;
  ZeroMemory(&siex, sizeof(siex));
  siex.StartupInfo.cb = sizeof(STARTUPINFOEXW);
  siex.StartupInfo.lpDesktop = L"winsta0\\Default";
  SIZE_T listSize = 0;
  DWORD err;
  if (!InitializeProcThreadAttributeList(nullptr, 1, 0, &listSize) && (err = GetLastError()) != ERROR_INSUFFICIENT_BUFFER) {
    cout << "InitializeProcThreadAttributeList failed with code " << err << endl;
    return 1;
  }
  auto listBytes = make_unique<char[]>(listSize);
  LPPROC_THREAD_ATTRIBUTE_LIST list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(listBytes.get());
  if (!InitializeProcThreadAttributeList(list, 1, 0, &listSize)) {
    cout << "InitializeProcThreadAttributeList (2) failed" << endl;
    return 1;
  }
  // DWORD64 policy = PROCESS_CREATION_MITIGATION_POLICY_EXTENSION_POINT_DISABLE_ALWAYS_ON;
  DWORD64 policy = PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON;
  if (!UpdateProcThreadAttribute(list, 0, PROC_THREAD_ATTRIBUTE_MITIGATION_POLICY, &policy, sizeof(policy), nullptr, nullptr)) {
    cout << "UpdateProcThreadAttribute failed" << endl;
    return 1;
  }
  siex.lpAttributeList = list;

  PROCESS_INFORMATION pi = {0};
  BOOL ok = CreateProcessW(exe, cl.get(), nullptr, nullptr, FALSE, EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, &siex.StartupInfo, &pi);
  err = GetLastError();

  DeleteProcThreadAttributeList(list);

  if (!ok) {
    cout << "CreateProcess failed with error code " << err << endl;
    return 1;
  }
  cout << "Created PID " << pi.dwProcessId << endl;
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return 0;
}

