#include "FileDialog.h"

#include <shobjidl_core.h>
#include <span>
#include <wrl.h>
namespace base_engine {

namespace detail
{

}

void FileDialog::SaveFile(const std::vector<std::string>& filters,
                          const std::filesystem::path& default_directory,
                          std::string_view title) {
  /*
  using namespace Microsoft::WRL;

  ComPtr<IFileSaveDialog> file_save_dialog;
  CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL,
                   IID_PPV_ARGS(&file_save_dialog));
  if (!filters.empty()) {
    std::wstring filter;
    for (auto& f : filters) {
      filter += std::wstring(f.begin(), f.end());
      filter += L'\0';
    }
    filter += L'\0';
    file_save_dialog->SetFileTypes(static_cast<UINT>(filters.size()),
                                   filter.c_str());
  }

  if (!default_directory.empty()) {
    file_save_dialog->SetDefaultFolder(default_directory.c_str());
  }

  if (!title.empty()) {
    file_save_dialog->SetTitle(
        std::wstring(title.begin(), title.end()).c_str());
  }
  file_save_dialog->Show(nullptr);
  ComPtr<IShellItem> result;
  file_save_dialog->GetResult(&result);

  PWSTR path;
  result->GetDisplayName(SIGDN_FILESYSPATH, &path);
  std::filesystem::path p(path);
  CoTaskMemFree(path);
  */
}
}  // namespace base_engine
