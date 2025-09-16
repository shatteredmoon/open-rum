#ifndef _C_PATCHER_H_
#define _C_PATCHER_H_

#include <network/u_network.h>
#include <network/u_patcher.h>

#define DEFAULT_PATCH_SERVER "127.0.0.1"
#define DEFAULT_PATCH_PORT   80
#define DEFAULT_PATCH_PATH   ""


class rumClientPatcher : public rumPatcher
{
public:

  enum class Status : uint8_t
  {
    Idle,
    Starting,
    Patching,
    Finished,
    Failed,
    NotApplicable
  };

  bool IsPatchingRequired() const
  {
    return m_bPatchRequired;
  }

  void Reset();

  // For setting the patch server's address, path, and port
  void SetConnectionInfo( bool bPatchRequired,
                          const std::string& i_strPatchChecksum,
                          const std::string& i_strAddress,
                          const std::string& i_strPath,
                          int32_t i_iPort );

  void StartPatch();

  void Update();

  // For accessing the singleton object
  static rumClientPatcher* GetInstance();

  // Creates the singleton
  static void Init( const std::string& i_strFilePath );

  static void OnFileDownloaded( const rumNetwork::rumDownloadInfo& i_rcDownloadInfo );

  static void NotifyScripts( const rumNetwork::rumDownloadInfo& i_rcDownloadInfo );

  // Destroys the singleton object
  static void Shutdown();

protected:

  void CheckPatchDatabase();
  bool DownloadPatchDatabase();
  rumNetwork::rumDownloadInfo GenerateDownloadInfo( const std::string& i_strFile, uint64_t i_uiFileSize );
  bool ReceivePatchFile( rumNetwork::rumDownloadInfo* i_pcDownloadInfo );

private:

  // The UUID used for identifying which product's files to download
  std::string m_strUUID;

  std::string m_strPatchChecksum;
  std::string m_strServerAddress{ DEFAULT_PATCH_SERVER };
  std::string m_strServerPath{ DEFAULT_PATCH_PATH };

  int32_t m_iServerPort{ DEFAULT_PATCH_PORT };

  uint32_t m_uiNumFilesToPatch{ 0 };
  Status m_eStatus{ Status::Idle };

  std::queue<rumNetwork::rumDownloadInfo> m_uiFileUpdatesQueue;

  bool m_bPatchRequired{ false };

  static std::vector<size_t> s_uiEventHandles;
  static std::mutex s_cMutex;

  using super = rumPatcher;
};

#endif // _C_PATCHER_H_
