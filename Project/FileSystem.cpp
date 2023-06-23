#include "FileSystem.h"

#include <Shlobj.h>
#include <Windows.h>
#include <strsafe.h>

#include <filesystem>
#include <tchar.h>
#include <thread>

#include "Assert.h"

namespace base_engine {
std::vector<FileSystem::FileSystemChangedCallbackFunc> FileSystem::Callbacks;

static bool s_Watching = false;
static bool s_IgnoreNextChange = false;
static HANDLE s_WatcherThread = NULL;
static HANDLE s_WatcherEvent = NULL;
static std::filesystem::path s_PersistentStoragePath;

void FileSystem::AddFileSystemChangedCallback(
    const FileSystemChangedCallbackFunc& callback) {
  Callbacks.push_back(callback);
}

void FileSystem::ClearFileSystemChangedCallbacks() { Callbacks.clear(); }

void FileSystem::StartWatching() {
  DWORD thread_id;
  s_Watching = true;
  s_WatcherThread = CreateThread(
      nullptr, 0, [](_In_ LPVOID) -> DWORD { return Watch(); }, 0, 0,
      &thread_id);
  BE_CORE_ASSERT(s_WatcherThread);
  SetThreadDescription(s_WatcherThread, L"FileSystemWatcher");
}

void FileSystem::StopWatching() {
  if (!s_Watching) return;

  s_Watching = false;
  s_IgnoreNextChange = true;

  if (s_WatcherEvent) {
    SetEvent(s_WatcherEvent);
    WaitForSingleObject(s_WatcherThread, 5000);
  }
  CloseHandle(s_WatcherThread);
}

unsigned long FileSystem::Watch() {
	const LPCTSTR dir = _T(".\\");
  char buf[2048]{};
  DWORD bytesReturned;
  std::filesystem::path filepath;
  BOOL result = TRUE;

  const HANDLE directory_handle = CreateFile(
      dir, GENERIC_READ | FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

  if (directory_handle == INVALID_HANDLE_VALUE) {
    BE_CORE_VERIFY(false, "Failed to open directory!");
    return 0;
  }

  OVERLAPPED polling_overlap;
  polling_overlap.OffsetHigh = 0;
  polling_overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  s_WatcherEvent = polling_overlap.hEvent;

  std::vector<FileSystemChangedEvent> event_batch;
  event_batch.reserve(16);

  while (s_Watching && result) {
    result = ReadDirectoryChangesW(directory_handle, &buf, sizeof(buf), TRUE,
                                   FILE_NOTIFY_CHANGE_FILE_NAME |
                                       FILE_NOTIFY_CHANGE_DIR_NAME |
                                       FILE_NOTIFY_CHANGE_SIZE,
                                   &bytesReturned, &polling_overlap, NULL);

    WaitForSingleObject(polling_overlap.hEvent, INFINITE);

    if (s_IgnoreNextChange) {
      s_IgnoreNextChange = false;
      event_batch.clear();
      continue;
    }

    FILE_NOTIFY_INFORMATION* pNotify;
    int offset = 0;
    std::wstring old_name;

    do {
      pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
          static_cast<char*>(buf) + offset);
      const size_t filename_length = pNotify->FileNameLength / sizeof(wchar_t);

      FileSystemChangedEvent e;
      e.filepath = std::filesystem::path(
          std::wstring(pNotify->FileName, filename_length));
      e.is_directory = is_directory(e.filepath);

      switch (pNotify->Action) {
        case FILE_ACTION_ADDED: {
          e.action = FileSystemAction::kAdded;
          break;
        }
        case FILE_ACTION_REMOVED: {
          e.action = FileSystemAction::kDelete;
          break;
        }
        case FILE_ACTION_MODIFIED: {
          e.action = FileSystemAction::kModified;
          break;
        }
        case FILE_ACTION_RENAMED_OLD_NAME: {
          old_name = e.filepath.filename();
          break;
        }
        case FILE_ACTION_RENAMED_NEW_NAME: {
          e.old_name = old_name;
          e.action = FileSystemAction::kRename;
          break;
        }
      }

      bool has_added_event = false;
      if (e.action == FileSystemAction::kModified) {
        for (const auto& [action, filepath, _2, _3] : event_batch) {
          if (filepath == e.filepath && action == FileSystemAction::kAdded)
            has_added_event = true;
        }
      }

      if (pNotify->Action != FILE_ACTION_RENAMED_OLD_NAME && !has_added_event)
        event_batch.push_back(e);

      offset += pNotify->NextEntryOffset;
    } while (pNotify->NextEntryOffset);

    if (!event_batch.empty()) {
      for (auto& cb : Callbacks) cb(event_batch);
      event_batch.clear();
    }
  }

  CloseHandle(directory_handle);

  return 0;
}
}  // namespace base_engine
