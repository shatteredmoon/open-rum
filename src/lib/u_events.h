#pragma once

#include <network/u_network.h>

#include <observe/event.h>


struct rumEvent
{
  static observe::Event<rumNetwork::rumDownloadInfo> s_cFileDownloadedEvent;
};
