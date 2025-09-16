#pragma once

#include <filesystem>
#include <string>
#include <vector>

// Represents an opened file with some cached data for fast per-frame rendering

struct rumDebugFile
{
  struct MultilineComment
  {
    int32_t m_iStartLine{ 0 };
    int32_t m_iEndLine{ 0 };
  };

  static constexpr size_t s_uiMinimumColumns{ 120U };

  std::string m_strFilename;
  std::filesystem::path m_fsFilePath;
  std::string m_strData;
  std::vector<size_t> m_vStringOffsets;
  std::vector<MultilineComment> m_vMultilineComments;
  size_t m_uiLongestLine{ s_uiMinimumColumns };
};
